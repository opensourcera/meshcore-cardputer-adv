#include "ChatPage.h"

#include "helpers.h"

#include <SD.h>

int ChatPage::render(CardputerDisplay &lcd) {
  char buf[64];

  display.setColor(CardputerDisplay::P_GREEN);

  if (current_contact_idx == -1 && current_channel_idx == -1) {
    display.setColor(CardputerDisplay::P_ORANGE);
    display.drawTextCentered(display.width() / 2, UI_PAGE_TOP, "Contact/Channel not selected");
  } else if (current_contact_idx != -1) {
    // out_path_len packs the path hash count in the lower 6 bits, and (hash size - 1) in the upper 2 bits
    uint8_t path_hops = current_contact.out_path_len & 63;
    if (current_contact.out_path_len == OUT_PATH_UNKNOWN) {
      snprintf(buf, sizeof(buf), "%s F", current_contact.name);
    } else if (path_hops == 0) {
      snprintf(buf, sizeof(buf), "%s D", current_contact.name);
    } else {
      snprintf(buf, sizeof(buf), "%s %uH", current_contact.name, path_hops);
    }
    display.drawTextCentered(display.width() / 2, UI_PAGE_TOP, buf);
  } else if (current_channel_idx != -1) {
    display.drawTextCentered(display.width() / 2, UI_PAGE_TOP, current_channel.name);
  }

  display.setColor(CardputerDisplay::P_GREEN);

  const int history_rows = UI_CHAT_HISTORY_LIST_SIZE;
  int current_y = UI_CHAT_HISTORY_TOP + (history_rows - 1) * display.getFontLineHeight();
  int total_lines_drawn = 0;

  int start_index = (int)chat_history.count() - 1 - chat_history_offset;

  for (int i = start_index; i >= 0; i--) {
    if (total_lines_drawn >= history_rows) {
      break;
    }

    const HistoryMessage *msg;

    if (!chat_history.get(i, msg)) {
      break;
    }

    int text_w = display.getTextWidth(msg->text);
    int message_lines = (text_w / display.width()) + 1;

    if (total_lines_drawn + message_lines > history_rows) {
      message_lines = history_rows - total_lines_drawn;
    }

    int message_top_y = current_y - ((message_lines - 1) * display.getFontLineHeight());

    if (msg->out) {
      if (text_w <= display.width()) {
        display.setCursor(display.width() - text_w - 5, message_top_y);
      } else {
        display.setCursor(5, message_top_y);
      }
    } else {
      display.setCursor(5, message_top_y);
    }

    if ((text_w / display.width()) + 1 == message_lines) {
      display.print(msg->text); // wraps text automatically
    } else {
      display.drawTextEllipsized(5, message_top_y, display.width(), msg->text);
    }

    current_y -= (message_lines * display.getFontLineHeight());
    total_lines_drawn += message_lines;
  }

  int start_x = 5;
  display.drawRect(1, display.height() - UI_TEXT_LINE_HEIGHT - 4, display.width() - 1, 1);

  display.setColor(CardputerDisplay::P_WHITE);

  int available_width = display.width() - start_x;

  if (chat_text_box.isEmpty()) {
    lcd.setColor(CardputerDisplay::P_GRAY);
    lcd.drawTextLeftAlign(5, lcd.height() - UI_TEXT_LINE_HEIGHT - 4, "Write a message...");
  } else {
    display.drawTextLeftAlignWithScroll(start_x, display.height() - UI_TEXT_LINE_HEIGHT - 4, available_width,
                                        chat_text_box.c_str());
  }

  return 15000;
}

void ChatPage::sendChatMessage() {
  if (chat_text_box.isEmpty()) {
    return;
  }

  uint32_t ts = the_mesh_cp.getRTCClock()->getCurrentTime();

  if (current_contact_idx != -1 && current_contact.type == ADV_TYPE_CHAT) { // direct msg
    int result = the_mesh_cp.sendDirectMessage(current_contact, ts, chat_text_box.c_str());

    if (result != MSG_SEND_FAILED) {
      HistoryMessage *msg = chat_history.push_ref();
      msg->out = true;
      snprintf(msg->text, MAX_MESSAGE_LENGTH, "%s", chat_text_box.c_str());

      last_sent_message = chat_text_box;
      chat_text_box.clear();
      chat_history_offset = 0;
    }
    return;
  }

  if (current_channel_idx != -1) { // channel msg

    bool ok = the_mesh_cp.sendGroupMessage(ts, current_channel.channel, _p->getUiTask()->getNodeName(),
                                           chat_text_box.c_str(), chat_text_box.length());
    if (ok) {
      HistoryMessage *msg = chat_history.push_ref();
      msg->out = true;

      snprintf(msg->text, MAX_MESSAGE_LENGTH, "%s", chat_text_box.c_str());

      last_sent_message = chat_text_box;
      chat_text_box.clear();
      chat_history_offset = 0;
      _p->getUiTask()->showAlert("Waiting for repeats...", 2000);
    }
    return;
  }
}

// ---- Quick replies (Ctrl+Space cycles through them) ----
// Placeholders: {name} sender of the last message heard here, {hops} / {snr} / {rssi} how it was heard, {me} this node.
// A file /meshcore_custom/quick_replies.txt on the SD card (one reply per line, up to 10) replaces the list below.

#define QUICK_REPLY_FILE "/meshcore_custom/quick_replies.txt"
#define QUICK_REPLY_MAX  10

static const char *const DEFAULT_QUICK_REPLIES[] = {
  "@[{name}] heard you: {hops}, SNR {snr} dB, RSSI {rssi} dBm",
  "@[{name}] received, thanks!",
  "@[{name}] loud and clear",
  "@[{name}] copy, weak here but readable",
  "Radio check, anyone hear me?",
  "Anyone around?",
  "Testing from {me}",
  "Thanks, 73",
  "Good morning mesh",
  "Good evening mesh",
};

static String quick_replies[QUICK_REPLY_MAX];
static int quick_reply_count = -1; // -1 = not loaded yet

static void loadQuickReplies() {
  if (quick_reply_count >= 0) {
    return;
  }
  quick_reply_count = 0;
  File f = SD.open(QUICK_REPLY_FILE);
  if (f) {
    while (f.available() && quick_reply_count < QUICK_REPLY_MAX) {
      String line = f.readStringUntil('\n');
      line.trim();
      if (line.length() > 0 && line.length() < MAX_MESSAGE_LENGTH) {
        quick_replies[quick_reply_count++] = line;
      }
    }
    f.close();
  }
  if (quick_reply_count == 0) {
    for (size_t i = 0; i < sizeof(DEFAULT_QUICK_REPLIES) / sizeof(DEFAULT_QUICK_REPLIES[0]) && i < QUICK_REPLY_MAX; i++) {
      quick_replies[quick_reply_count++] = DEFAULT_QUICK_REPLIES[i];
    }
  }
}

bool ChatPage::buildQuickReply(int idx, String &out) {
  String t = quick_replies[idx];
  bool needs_heard = t.indexOf("{name}") >= 0 || t.indexOf("{hops}") >= 0 || t.indexOf("{snr}") >= 0 ||
                     t.indexOf("{rssi}") >= 0;
  HeardInfo h;
  if (needs_heard) {
    bool found = false;
    if (current_contact_idx != -1 && current_contact.type == ADV_TYPE_CHAT) {
      found = the_mesh_cp.getLastHeard(current_contact.id.pub_key, false, h);
      if (found) {
        t.replace("@[{name}] ", ""); // no name prefix in a direct chat
      }
    } else if (current_channel_idx != -1) {
      found = the_mesh_cp.getLastHeard(current_channel.channel.secret, true, h);
    }
    if (!found) {
      return false; // nothing heard here yet: skip replies that quote it
    }
    char num[24];
    if (h.hops == 0) {
      snprintf(num, sizeof(num), "direct");
    } else {
      snprintf(num, sizeof(num), "%d hop%s", h.hops, h.hops == 1 ? "" : "s");
    }
    t.replace("{hops}", num);
    snprintf(num, sizeof(num), "%.1f", h.snr);
    t.replace("{snr}", num);
    snprintf(num, sizeof(num), "%.0f", h.rssi);
    t.replace("{rssi}", num);
    t.replace("{name}", h.sender);
  }
  t.replace("{me}", _p->getUiTask()->getNodeName());
  out = t;
  return true;
}

// Ctrl+Space: put the next quick reply into the message box. Typing something else restarts from the first one.
void ChatPage::fillQuickReply() {
  loadQuickReplies();
  if (chat_text_box.isEmpty() || chat_text_box != quick_reply_last) {
    quick_reply_idx = -1;
  }
  for (int n = 0; n < quick_reply_count; n++) {
    quick_reply_idx = (quick_reply_idx + 1) % quick_reply_count;
    String s;
    if (buildQuickReply(quick_reply_idx, s)) {
      chat_text_box = s;
      quick_reply_last = s;
      return;
    }
  }
  _p->getUiTask()->showAlert("No quick reply fits here", 2000);
}

bool ChatPage::handleInput(Keyboard::Event &e) {
  CardputerLayout *lay = _p->getUiTask()->getBoard()->getLayout();

  if (e.modifiers.ctrl && e.key == Keyboard::KEY_T) {
    the_mesh_cp.cancelResending();
    return true;
  }

  if (the_mesh_cp.isMessageSending()) {
    return true;
  }

  if (e.modifiers.ctrl && e.key == Keyboard::KEY_SPACE) {
    fillQuickReply();
    return true;
  }

  if (e.modifiers.ctrl && e.key == Keyboard::KEY_BACKSPACE) {
    chat_text_box.clear();
    return true;
  }

  if (e.key == Keyboard::KEY_BACKSPACE) {
    Helpers::removeLastStringChar(chat_text_box);
    return true;
  }

  if (e.key == Keyboard::KEY_ESC) {
    _p->returnToLastPage();
    return true;
  }

  if (e.modifiers.ctrl && e.key == Keyboard::ARROW_UP && chat_text_box.isEmpty()) {
    chat_text_box = last_sent_message;
    return true;
  }

  if (e.key == Keyboard::KEY_RETURN) {
    sendChatMessage();
    return true;
  }

  // Determine if arrow keys should act as scrolling/navigation or text input
  bool fnPressed = (chat_text_box.length() == 0) ? !e.modifiers.fn : e.modifiers.fn;

  if (fnPressed) {
    if (e.key == Keyboard::ARROW_DOWN) {
      if (chat_history_offset > 0) {
        chat_history_offset--;
      }
      return true;
    }

    if (e.key == Keyboard::ARROW_UP) {
      if (chat_history_offset < chat_history.count() - 1) {
        chat_history_offset++;
      }
      return true;
    }
  }

  bool skip_input = (fnPressed && (e.key == Keyboard::ARROW_RIGHT || e.key == Keyboard::ARROW_LEFT)) ||
                    e.key == Keyboard::KEY_ESC;

  if (!skip_input) {
    const char *repl = lay->lookup(e);

    if (repl[0]) {
      int maxlen = MAX_MESSAGE_LENGTH - strlen(_p->getUiTask()->getNodeName());

      if (chat_text_box.length() + strlen(repl) <= maxlen) {
        chat_text_box += repl;
      }
      return true;
    }
  }
  return false;
}

void ChatPage::selectContact(const ContactInfo &contact, int idx) {
  current_contact = contact;
  current_contact_idx = idx;
  current_channel_idx = -1;
  the_mesh_cp.loadMessageHistory(contact.id.pub_key, false, chat_history);
  chat_history_offset = 0;

  const HistoryMessage *msg;
  for (size_t i = chat_history.count(); i > 0; i--) {
    if (chat_history.get(i - 1, msg) && msg->out) {
      last_sent_message = msg->text;
      break;
    }
  }
}

void ChatPage::selectChannel(const ChannelDetails &channel, int idx) {
  current_channel = channel;
  current_channel_idx = idx;
  current_contact_idx = -1;
  the_mesh_cp.loadMessageHistory(channel.channel.secret, true, chat_history);
  chat_history_offset = 0;

  const HistoryMessage *msg;
  for (size_t i = chat_history.count(); i > 0; i--) {
    if (chat_history.get(i - 1, msg) && msg->out) {
      last_sent_message = msg->text;
      break;
    }
  }
}

bool ChatPage::onChannelMessageRecv(const mesh::GroupChannel &channel, const char *text) {

  if (current_channel_idx != -1 &&
      memcmp(channel.secret, current_channel.channel.secret, CONTACT_LOOKUP_BYTES) == 0) {
    HistoryMessage *msg = chat_history.push_ref();
    msg->out = false;
    snprintf(msg->text, MAX_MESSAGE_LENGTH, "%s", text);
    chat_history_offset = 0;
    return true;
  }
  return false;
}

bool ChatPage::onContactMessageRecv(const ContactInfo &contact, const char *text) {
  if (current_contact_idx != -1 &&
      memcmp(contact.id.pub_key, current_contact.id.pub_key, CONTACT_LOOKUP_BYTES) == 0) {
    HistoryMessage *msg = chat_history.push_ref();
    msg->out = false;
    snprintf(msg->text, MAX_MESSAGE_LENGTH, "%s", text);
    chat_history_offset = 0;
    return true;
  }
  return false;
}

void ChatPage::refreshSelectedContact() {
  if (current_contact_idx != -1) {
    ContactInfo c;
    if (the_mesh_cp.getContactByIdx(current_contact_idx, c)) {
      current_contact = c;
    }
  }
}
