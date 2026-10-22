#include "ToolsScreen.h"

static void drawMenuRows(CardputerDisplay &lcd, int top, int rows, int count, int selected,
                         const char *(*label)(int, void *), void *ctx) {
  int list_page = selected / rows;
  int list_idx = selected % rows;

  for (int i = 0; i < rows; i++) {
    int real_idx = list_page * rows + i;
    if (real_idx >= count) {
      break;
    }
    const char *text = label(real_idx, ctx);
    lcd.setColor(text[0] == '-' ? CardputerDisplay::P_GREEN : CardputerDisplay::P_YELLOW);
    lcd.drawTextEllipsized(15, top + i * UI_TEXT_LINE_HEIGHT, lcd.width() - 17, text);

    if (i == list_idx) {
      lcd.setColor(CardputerDisplay::P_GREEN);
      lcd.drawTextLeftAlign(5, top + i * UI_TEXT_LINE_HEIGHT, ">");
    }
  }
}

static const char *menuLabel(int i, void *ctx) {
  return static_cast<const char **>(ctx)[i];
}

int ToolsScreen::render(CardputerDisplay &lcd) {
  char buf[64];

  lcd.setTextSize(1);
  lcd.setColor(CardputerDisplay::P_GREEN);
  if (page == ToolsPage::MenuPage) {
    lcd.drawTextCentered(lcd.width() / 2, 5, "Tools");

    drawMenuRows(lcd, UI_MENU_TOP, UI_TOOLS_LIST_SIZE, ToolsMenuItem::Count, menu_index, menuLabel,
                 (void *)menu_item_labels);
  } else if (page == ToolsPage::DiscoverPage) {
    lcd.drawTextCentered(lcd.width() / 2, 5, "Discover");

    for (int i = 0; i < discovered_repeaters_count; i++) {
      DiscoveredRepeater *rep = &discovered_repeaters[i];

      lcd.setColor(CardputerDisplay::P_YELLOW);
      lcd.drawTextLeftAlign(0, (UI_MENU_TOP + 5) + i * UI_TEXT_LINE_HEIGHT, rep->name);

      if (rep->snr > 0) {
        lcd.setColor(CardputerDisplay::P_GREEN);
      } else {
        lcd.setColor(CardputerDisplay::P_RED);
      }

      sprintf(buf, "%.2fdb", rep->snr);
      lcd.drawTextRightAlign(lcd.width() - 2, (UI_MENU_TOP + 5) + i * UI_TEXT_LINE_HEIGHT, buf);
    }

    return 1000;
  } else if (page == ToolsPage::SignalTargetPage) {
    renderRepeaterList(lcd, "Signal: which repeater?", false, -1);
  } else if (page == ToolsPage::SignalViaPage) {
    renderRepeaterList(lcd, "Signal: go through?", true, signal_target_idx);
  } else if (page == ToolsPage::SignalRunPage) {
    renderSignalRun(lcd);
    return 1000;
  }

  return 5000;
}

// ---- Signal tester ----

int ToolsScreen::repeaterCount(int exclude_idx) {
  ContactInfo contact;
  int num = 0;
  for (int i = MAX_ANON_CONTACTS; i < the_mesh_cp.getTotalContactSlots(); i++) {
    if (!the_mesh_cp.getContactByIdx(i, contact)) {
      break;
    }
    if (contact.type == ADV_TYPE_REPEATER && i != exclude_idx) {
      num++;
    }
  }
  return num;
}

bool ToolsScreen::repeaterByListIdx(int list_idx, int exclude_idx, ContactInfo &contact, int &real_idx) {
  int match = 0;
  for (int i = MAX_ANON_CONTACTS; i < the_mesh_cp.getTotalContactSlots(); i++) {
    if (!the_mesh_cp.getContactByIdx(i, contact)) {
      break;
    }
    if (contact.type == ADV_TYPE_REPEATER && i != exclude_idx) {
      if (match == list_idx) {
        real_idx = i;
        return true;
      }
      match++;
    }
  }
  return false;
}

struct RepeaterListCtx {
  ToolsScreen *screen;
  bool with_direct;
  int exclude_idx;
  char buf[40];
};

static const char *repeaterListLabel(int i, void *p) {
  RepeaterListCtx *c = static_cast<RepeaterListCtx *>(p);
  if (c->with_direct) {
    if (i == 0) {
      return "(direct, no repeater)";
    }
    i--;
  }
  ContactInfo contact;
  int real_idx;
  if (c->screen->repeaterByListIdx(i, c->exclude_idx, contact, real_idx)) {
    snprintf(c->buf, sizeof(c->buf), "%s", contact.name);
  } else {
    snprintf(c->buf, sizeof(c->buf), "?");
  }
  return c->buf;
}

void ToolsScreen::renderRepeaterList(CardputerDisplay &lcd, const char *title, bool with_direct, int exclude_idx) {
  lcd.drawTextCentered(lcd.width() / 2, 5, title);

  int count = repeaterCount(exclude_idx) + (with_direct ? 1 : 0);
  if (count == 0) {
    lcd.setColor(CardputerDisplay::P_RED);
    lcd.drawTextLeftAlign(5, UI_MENU_TOP, "No repeaters in contacts");
    return;
  }
  if (signal_list_idx >= count) {
    signal_list_idx = count - 1;
  }

  RepeaterListCtx ctx = { this, with_direct, exclude_idx, { 0 } };
  drawMenuRows(lcd, UI_MENU_TOP, UI_TOOLS_LIST_SIZE, count, signal_list_idx, repeaterListLabel, &ctx);
}

void ToolsScreen::renderSignalRun(CardputerDisplay &lcd) {
  char buf[80];

  // Title: target name, plus the via name when there is one
  if (signal_via_idx >= 0) {
    snprintf(buf, sizeof(buf), "%s via %s", signal_target_name, signal_via_name);
  } else {
    snprintf(buf, sizeof(buf), "%s", signal_target_name);
  }
  lcd.setColor(CardputerDisplay::P_GREEN);
  int max_w = lcd.width() - 4;
  if (lcd.getTextWidth(buf) <= max_w) {
    lcd.drawTextCentered(lcd.width() / 2, 5, buf);
  } else {
    lcd.drawTextEllipsized(2, 5, max_w, buf);
  }

  int rows = UI_TOOLS_LIST_SIZE;
  int y = UI_MENU_TOP;
  int first = (signal_sent > rows - 1) ? signal_sent - (rows - 1) : 0;   // keep the latest tries visible
  for (int i = first; i < signal_sent; i++) {
    SignalTry *t = &signal_tries[i];
    lcd.setColor(CardputerDisplay::P_YELLOW);
    snprintf(buf, sizeof(buf), "#%d", i + 1);
    lcd.drawTextLeftAlign(5, y, buf);

    if (t->returned) {
      lcd.setColor(t->there > 0 && t->back > 0 ? CardputerDisplay::P_GREEN : CardputerDisplay::P_ORANGE);
      snprintf(buf, sizeof(buf), "%.1f / %.1f dB", t->there, t->back);
    } else if (signal_running && i == signal_sent - 1) {
      lcd.setColor(CardputerDisplay::P_GRAY);
      snprintf(buf, sizeof(buf), "waiting...");
    } else {
      lcd.setColor(CardputerDisplay::P_RED);
      snprintf(buf, sizeof(buf), "no reply");
    }
    lcd.drawTextEllipsized(40, y, lcd.width() - 42, buf);
    y += UI_TEXT_LINE_HEIGHT;
  }

  // Summary on the last row
  y = UI_MENU_TOP + (rows - 1) * UI_TEXT_LINE_HEIGHT;
  if (signal_running) {
    lcd.setColor(CardputerDisplay::P_GREEN);
    snprintf(buf, sizeof(buf), "Try %d/%d  Esc=stop", signal_sent, SIGNAL_TEST_TRIES);
  } else {
    lcd.setColor(signal_returned == 0 ? CardputerDisplay::P_RED : CardputerDisplay::P_GREEN);
    snprintf(buf, sizeof(buf), "%d/%d ok  Enter=again", signal_returned, SIGNAL_TEST_TRIES);
  }
  lcd.drawTextEllipsized(5, y, lcd.width() - 7, buf);
}

void ToolsScreen::startSignalTest() {
  memset(signal_tries, 0, sizeof(signal_tries));
  signal_sent = 0;
  signal_returned = 0;
  signal_running = true;
  signal_next_send = 0;   // send the first one on the next poll
  page = ToolsPage::SignalRunPage;
  _task->scheduleRefresh();
}

void ToolsScreen::sendSignalTry() {
  ContactInfo target, via;
  if (!the_mesh_cp.getContactByIdx(signal_target_idx, target)) {
    stopSignalTest();
    return;
  }
  const ContactInfo *via_ptr = NULL;
  if (signal_via_idx >= 0 && the_mesh_cp.getContactByIdx(signal_via_idx, via)) {
    via_ptr = &via;
  }

  SignalTry *t = &signal_tries[signal_sent];
  t->sent = the_mesh_cp.sendSignalTrace(via_ptr, target, t->tag);
  signal_sent++;
  signal_next_send = millis() + SIGNAL_TEST_INTERVAL_MS;
  _task->scheduleRefresh();
}

void ToolsScreen::stopSignalTest() {
  if (signal_running) {
    signal_running = false;
    _task->playSound(SoundType::DiscoveryResult);
    _task->scheduleRefresh();
  }
}

void ToolsScreen::poll() {
  if (!signal_running) {
    return;
  }
  if (signal_sent < SIGNAL_TEST_TRIES) {
    if (millis() >= signal_next_send) {
      sendSignalTry();
    }
  } else if (millis() >= signal_next_send) {
    stopSignalTest();
  }
}

void ToolsScreen::signalTraceRecv(uint32_t tag, const uint8_t *snrs, uint8_t count, float final_snr) {
  if (page != ToolsPage::SignalRunPage) {
    return;
  }
  for (int i = 0; i < signal_sent; i++) {
    SignalTry *t = &signal_tries[i];
    if (!t->sent || t->returned || t->tag != tag) {
      continue;
    }
    // snrs[] holds one entry per hop the trace went through: out to the target, then back.
    // Outbound hops: 1 (direct) or 2 (through a repeater).
    int out_hops = (signal_via_idx >= 0) ? 2 : 1;
    if (count >= out_hops) {
      t->there = (int8_t)snrs[out_hops - 1] / 4.0f;
    } else {
      t->there = final_snr;
    }
    t->back = (count > out_hops) ? (int8_t)snrs[out_hops] / 4.0f : final_snr;
    t->returned = true;
    signal_returned++;
    _task->playSound(SoundType::DiscoveryResult);
    _task->scheduleRefresh();
    return;
  }
}

// ---- Menu ----

void ToolsScreen::menuItemEnter(ToolsMenuItem item) {
  switch (item) {
    case ToolsMenuItem::AdvertFlood:
      if (the_mesh_cp.sendAdvert(true)) {
        _task->showAlert("Flood Advert sent", 3000);
      }

      break;
    case ToolsMenuItem::AdvertZeroHop:
      if (the_mesh_cp.sendAdvert(false)) {
        _task->showAlert("Advert sent", 3000);
      }
      break;

    case ToolsMenuItem::DiscoverRepeaters:
      discovered_repeaters_count = 0;

      if (the_mesh_cp.sendRepeatersDiscover()) {
        page = ToolsPage::DiscoverPage;
        _task->showAlert("Waiting for response...", 10000);
      }

      break;

    case ToolsMenuItem::SignalTester:
      signal_list_idx = 0;
      signal_target_idx = -1;
      signal_via_idx = -1;
      page = ToolsPage::SignalTargetPage;
      break;

    case ToolsMenuItem::PowerOff: {
      bool done = false;
      // Wait for key release to prevent firing interrupt
      do {
        Keyboard::Event e = _task->getBoard()->getKeyboard()->poll();
        done = (e.changed && e.key == Keyboard::KEY_RETURN && e.down == false);
        delay(10);
      } while (!done);
      _task->shutdown();
    } break;

    case ToolsMenuItem::Restart:
      ESP.restart();
      break;

    default:
      break;
  }
}

bool ToolsScreen::handleInput(Keyboard::Event &e) {
  if (_task->isAlertActive()) {
    return false;
  }

  if (page == ToolsPage::MenuPage) {
    if (e.key == Keyboard::KEY_ESC) {
      _task->gotoMainScreen();
      return true;
    }

    if (e.key == Keyboard::ARROW_UP) {
      if (menu_index == 0) {
        menu_index = ToolsMenuItem::Count - 1;
      } else {
        menu_index--;
      }
      return true;
    }

    if (e.key == Keyboard::ARROW_DOWN) {
      if (menu_index < ToolsMenuItem::Count - 1) {
        menu_index++;
      } else {
        menu_index = 0;
      }
      return true;
    }

    if (e.key == Keyboard::KEY_RETURN) {
      menuItemEnter(static_cast<ToolsMenuItem>(menu_index));
      return true;
    }
  } else if (page == ToolsPage::DiscoverPage) {
    if (e.key == Keyboard::KEY_ESC) {
      page = ToolsPage::MenuPage;
      return true;
    }
  } else if (page == ToolsPage::SignalTargetPage || page == ToolsPage::SignalViaPage) {
    bool via_page = (page == ToolsPage::SignalViaPage);
    int exclude = via_page ? signal_target_idx : -1;
    int count = repeaterCount(exclude) + (via_page ? 1 : 0);

    if (e.key == Keyboard::KEY_ESC) {
      if (via_page) {
        page = ToolsPage::SignalTargetPage;
        signal_list_idx = 0;
      } else {
        page = ToolsPage::MenuPage;
      }
      return true;
    }
    if (count == 0) {
      return false;
    }
    if (e.key == Keyboard::ARROW_UP) {
      signal_list_idx = (signal_list_idx == 0) ? count - 1 : signal_list_idx - 1;
      return true;
    }
    if (e.key == Keyboard::ARROW_DOWN) {
      signal_list_idx = (signal_list_idx < count - 1) ? signal_list_idx + 1 : 0;
      return true;
    }
    if (e.key == Keyboard::KEY_RETURN) {
      ContactInfo contact;
      int real_idx;
      if (!via_page) {
        if (repeaterByListIdx(signal_list_idx, -1, contact, real_idx)) {
          signal_target_idx = real_idx;
          snprintf(signal_target_name, sizeof(signal_target_name), "%s", contact.name);
          signal_list_idx = 0;
          page = ToolsPage::SignalViaPage;
        }
      } else {
        if (signal_list_idx == 0) {
          signal_via_idx = -1;
          signal_via_name[0] = 0;
          startSignalTest();
        } else if (repeaterByListIdx(signal_list_idx - 1, signal_target_idx, contact, real_idx)) {
          signal_via_idx = real_idx;
          snprintf(signal_via_name, sizeof(signal_via_name), "%s", contact.name);
          startSignalTest();
        }
      }
      return true;
    }
  } else if (page == ToolsPage::SignalRunPage) {
    if (e.key == Keyboard::KEY_ESC) {
      signal_running = false;
      page = ToolsPage::MenuPage;
      return true;
    }
    if (e.key == Keyboard::KEY_RETURN && !signal_running) {
      startSignalTest();
      return true;
    }
  }
  return false;
}

void ToolsScreen::discoverRecv(const mesh::Identity &id, float snr) {
  if (page == ToolsPage::DiscoverPage && discovered_repeaters_count < MAX_DISCOVERED_REPEATERS) {
    DiscoveredRepeater rep;
    rep.snr = snr;
    rep.name[0] = '\0';

    ContactInfo *contact = the_mesh_cp.lookupContactByPubKey(id.pub_key, CONTACT_LOOKUP_BYTES);

    if (contact) {
      snprintf(rep.name, sizeof(rep.name), "%s", contact->name);
    } else {
      snprintf(rep.name, sizeof(rep.name), "[%02X %02X %02X %02X]", id.pub_key[0], id.pub_key[1],
               id.pub_key[2], id.pub_key[3]);
    }

    discovered_repeaters[discovered_repeaters_count] = rep;
    discovered_repeaters_count++;

    _task->dismissAlert();
    _task->playSound(SoundType::DiscoveryResult);
  }
}
