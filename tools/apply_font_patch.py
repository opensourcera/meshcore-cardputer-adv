"""One-off patch that added the Settings > Font size feature (kept for reference)."""
import io, os

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'variants', 'cardputer_adv')


def patch(path, pairs):
    path = os.path.join(ROOT, path)
    s = io.open(path, encoding='utf-8').read()
    for old, new, cnt in pairs:
        n = s.count(old)
        assert n == cnt, '%s: expected %d of %r, found %d' % (path, cnt, old[:60], n)
        s = s.replace(old, new)
    io.open(path, 'w', encoding='utf-8', newline='\n').write(s)
    print('patched', path)


patch('globals.h', [(
    "#define UI_TEXT_LINE_HEIGHT                      12 // adjust to the used font todo: replace to display.getFontLineHeight\n"
    "#define UI_RECENT_LIST_SIZE                      4\n"
    "#define UI_CONTACT_LIST_SIZE                     7\n"
    "#define UI_CHANNEL_LIST_SIZE                     8\n"
    "#define UI_CHANNEL_SEARCH_MAX_CHARS              32\n"
    "#define UI_SETTINGS_LIST_SIZE                    9\n"
    "#define UI_CHAT_HISTORY_LIST_SIZE                5\n"
    "#define UI_TOOLS_LIST_SIZE                       9\n",
    "// Layout follows the selected font (Settings > Font size). Values below equal the original fixed layout at 12 px.\n"
    "#define UI_TEXT_LINE_HEIGHT                      (display.getUiLineHeight())\n"
    "#define UI_PAGE_TOP                              (UI_TEXT_LINE_HEIGHT + 8)                   // first row below the status bar\n"
    "#define UI_LIST_TOP                              (UI_PAGE_TOP + UI_TEXT_LINE_HEIGHT - 2)      // first list row below a page title\n"
    "#define UI_MENU_TOP                              (UI_TEXT_LINE_HEIGHT + 8)                   // first row below a screen title at y=5\n"
    "#define UI_LIST_ROWS(top, bottom_reserved)       ((display.height() - (bottom_reserved) - (top)) / UI_TEXT_LINE_HEIGHT)\n"
    "#define UI_RECENT_LIST_SIZE                      4\n"
    "#define UI_CONTACT_LIST_SIZE                     UI_LIST_ROWS(UI_LIST_TOP, UI_TEXT_LINE_HEIGHT + 4)\n"
    "#define UI_CHANNEL_LIST_SIZE                     UI_LIST_ROWS(UI_LIST_TOP, 0)\n"
    "#define UI_CHANNEL_SEARCH_MAX_CHARS              32\n"
    "#define UI_SETTINGS_LIST_SIZE                    UI_LIST_ROWS(UI_MENU_TOP, 0)\n"
    "#define UI_CHAT_HISTORY_TOP                      (UI_LIST_TOP + 5)\n"
    "#define UI_CHAT_HISTORY_LIST_SIZE                ((display.height() - UI_TEXT_LINE_HEIGHT - 4 - UI_CHAT_HISTORY_TOP) / display.getFontLineHeight())\n"
    "#define UI_TOOLS_LIST_SIZE                       UI_LIST_ROWS(UI_MENU_TOP, 0)\n", 1)])

patch('hw/CardputerDisplay.h', [(
    "  int16_t _fontYAdvance = 1;\n", "  int16_t _fontYAdvance = 1;\n  uint8_t _font_idx = 0;\n", 1), (
    "  bool begin();\n  void tryLoadUserFont();\n",
    "  bool begin();\n  void tryLoadUserFont();\n\n"
    "  /** Built-in font sizes selectable in Settings */\n"
    "  static const uint8_t FONT_COUNT = 4;\n"
    "  static uint8_t fontPixels(uint8_t idx);\n"
    "  bool setFontSize(uint8_t idx);\n"
    "  uint8_t getFontSize() const { return _font_idx; }\n"
    "  /** Row pitch used by lists and menus for the current font */\n"
    "  int16_t getUiLineHeight() const;\n", 1)])

patch('hw/CardputerDisplay.cpp', [(
    '#include "font.h"\n',
    '#include "font.h"\n#include "fonts/DejaVuSans_14.h"\n#include "fonts/DejaVuSans_16.h"\n#include "fonts/DejaVuSans_18.h"\n', 1), (
    "ColorVal UIColor::window_bkg = CardputerDisplay::PaletteColor::P_BLACK;\n",
    "ColorVal UIColor::window_bkg = CardputerDisplay::PaletteColor::P_BLACK;\n\n"
    "static const uint8_t *const FONTS[CardputerDisplay::FONT_COUNT] = { DejaVuSans_11, DejaVuSans_14, DejaVuSans_16, DejaVuSans_18 };\n"
    "static const uint8_t FONT_PX[CardputerDisplay::FONT_COUNT] = { 12, 14, 16, 18 };\n", 1), (
    "  LCD.loadFont(DejaVuSans_11);\n", "  LCD.loadFont(FONTS[_font_idx]);\n", 1), (
    "void CardputerDisplay::tryLoadUserFont() {\n",
    "uint8_t CardputerDisplay::fontPixels(uint8_t idx) {\n  return FONT_PX[idx < FONT_COUNT ? idx : 0];\n}\n\n"
    "bool CardputerDisplay::setFontSize(uint8_t idx) {\n"
    "  if (idx >= FONT_COUNT) {\n    idx = 0;\n  }\n"
    "  _font_idx = idx;\n  LCD.loadFont(FONTS[idx]);\n  updateFontYAdvance();\n  return true;\n}\n\n"
    "int16_t CardputerDisplay::getUiLineHeight() const {\n"
    "  // the original 12 px font is laid out with a 12 px pitch; larger fonts get one extra pixel of air\n"
    "  return _font_idx == 0 ? 12 : FONT_PX[_font_idx] + 1;\n}\n\n"
    "void CardputerDisplay::tryLoadUserFont() {\n", 1), (
    "void CardputerDisplay::setTextSize(int sz) {\n  LCD.setTextSize(sz);\n}\n",
    "void CardputerDisplay::setTextSize(int sz) {\n"
    "  // double-size text is kept at about 24 px whatever font is selected, so it still fits the screen\n"
    "  LCD.setTextSize(sz == 2 ? 24.0f / FONT_PX[_font_idx] : (float)sz);\n}\n", 1)])

patch('CardputerDataStore.h', [(
    "  uint8_t power_save;\n  float battery_correction;\n};",
    "  uint8_t power_save;\n  float battery_correction;\n  uint8_t font_size; // index into the CardputerDisplay font table\n};", 1)])

patch('CardputerDataStore.cpp', [(
    "  // Read file only if it exactly matches struct size, otherwise keep default values\n"
    "  if (file.size() == sizeof(prefs)) {\n    file.read((uint8_t *)&prefs, sizeof(prefs));\n  }\n",
    "  // Fields are only ever appended, so an older (shorter) file is read as far as it goes and the rest keeps defaults\n"
    "  size_t n = file.size();\n  if (n >= 5 && n <= sizeof(prefs)) {\n    file.read((uint8_t *)&prefs, n);\n  }\n", 1)])

patch('companion_radio/CardputerMesh.h', [(
    "{ .power_save = 0, .battery_correction = 1.0f }", "{ .power_save = 0, .battery_correction = 1.0f, .font_size = 0 }", 1)])

patch('companion_radio/ui-cardputer/CardputerUITask.h', [(
    "  void togglePowerSave();\n",
    "  void togglePowerSave();\n"
    "  /** Switch the display font without saving (live preview) */\n  void applyFontSize(uint8_t idx);\n"
    "  /** Switch the display font and save it */\n  void setFontSize(uint8_t idx);\n", 1)])

patch('companion_radio/ui-cardputer/CardputerUITask.cpp', [(
    "  _node_prefs = node_prefs;\n  _custom_prefs = custom_prefs;\n\n  if (_display != NULL) {\n    _display->turnOn();\n  }\n",
    "  _node_prefs = node_prefs;\n  _custom_prefs = custom_prefs;\n\n  if (_display != NULL) {\n    _display->setFontSize(_custom_prefs->font_size);\n    _display->turnOn();\n  }\n", 1), (
    "void CardputerUITask::togglePowerSave() {\n  _custom_prefs->power_save = !_custom_prefs->power_save;\n  the_mesh_cp.savePrefs();\n}\n",
    "void CardputerUITask::togglePowerSave() {\n  _custom_prefs->power_save = !_custom_prefs->power_save;\n  the_mesh_cp.savePrefs();\n}\n\n"
    "void CardputerUITask::applyFontSize(uint8_t idx) {\n  if (_display != NULL) {\n    _display->setFontSize(idx);\n  }\n  scheduleRefresh();\n}\n\n"
    "void CardputerUITask::setFontSize(uint8_t idx) {\n  _custom_prefs->font_size = idx;\n  applyFontSize(idx);\n  the_mesh_cp.savePrefs();\n}\n", 1)])

patch('companion_radio/ui-cardputer/screen/SettingsScreen.h', [(
    "    DevicePowersave,\n", "    DevicePowersave,\n    DeviceFontSize,\n", 1)])

patch('companion_radio/ui-cardputer/screen/SettingsScreen.cpp', [(
    "    case SettingsItem::DeviceBatteryCorrection:\n      snprintf(tmp, sizeof(tmp), \"Battery correction: %.3f\", _custom_prefs->battery_correction);\n      break;\n",
    "    case SettingsItem::DeviceBatteryCorrection:\n      snprintf(tmp, sizeof(tmp), \"Battery correction: %.3f\", _custom_prefs->battery_correction);\n      break;\n\n"
    "    case SettingsItem::DeviceFontSize:\n      snprintf(tmp, sizeof(tmp), \"Font size: %d\", CardputerDisplay::fontPixels(_custom_prefs->font_size));\n      break;\n", 1), (
    "    case SettingsItem::MeshPathSize:\n      edit_u8 = _node_prefs->path_hash_mode + 1;\n      is_editing = true;\n      return true;\n",
    "    case SettingsItem::MeshPathSize:\n      edit_u8 = _node_prefs->path_hash_mode + 1;\n      is_editing = true;\n      return true;\n\n"
    "    case SettingsItem::DeviceFontSize:\n      edit_u8 = _custom_prefs->font_size;\n      is_editing = true;\n      return true;\n", 1), (
    "void SettingsScreen::cancelItemEdit(SettingsItem item) {\n  is_editing = false;\n}\n",
    "void SettingsScreen::cancelItemEdit(SettingsItem item) {\n  if (item == SettingsItem::DeviceFontSize) {\n    _task->applyFontSize(_custom_prefs->font_size); // undo the live preview\n  }\n  is_editing = false;\n}\n", 1), (
    "    case SettingsItem::MeshPathSize:\n      _node_prefs->path_hash_mode = edit_u8 - 1;\n      the_mesh_cp.savePrefs();\n      return true;\n",
    "    case SettingsItem::MeshPathSize:\n      _node_prefs->path_hash_mode = edit_u8 - 1;\n      the_mesh_cp.savePrefs();\n      return true;\n\n"
    "    case SettingsItem::DeviceFontSize:\n      _task->setFontSize(edit_u8);\n      return true;\n", 1), (
    "    case SettingsItem::MeshPathSize:\n      if (e.key == Keyboard::ARROW_LEFT && edit_u8 > 1) {\n        edit_u8--;\n      } else if (e.key == Keyboard::ARROW_RIGHT && edit_u8 < 3) {\n        edit_u8++;\n      }\n      break;\n",
    "    case SettingsItem::MeshPathSize:\n      if (e.key == Keyboard::ARROW_LEFT && edit_u8 > 1) {\n        edit_u8--;\n      } else if (e.key == Keyboard::ARROW_RIGHT && edit_u8 < 3) {\n        edit_u8++;\n      }\n      break;\n\n"
    "    case SettingsItem::DeviceFontSize:\n      if (e.key == Keyboard::ARROW_LEFT && edit_u8 > 0) {\n        edit_u8--;\n      } else if (e.key == Keyboard::ARROW_RIGHT && edit_u8 < CardputerDisplay::FONT_COUNT - 1) {\n        edit_u8++;\n      }\n      _task->applyFontSize(edit_u8); // live preview\n      break;\n", 1), (
    "    switch (menu_index) {\n      case SettingsItem::RadioSf:\n      case SettingsItem::RadioCr:\n      case SettingsItem::RadioPwr:\n      case SettingsItem::MeshPathSize:\n        sprintf(buf, \"- %d +\", edit_u8);\n",
    "    switch (menu_index) {\n      case SettingsItem::DeviceFontSize:\n        sprintf(buf, \"- %d +\", CardputerDisplay::fontPixels(edit_u8));\n        display.drawTextCentered(display.width() / 2, display.height() / 2 - 8, buf);\n        break;\n"
    "      case SettingsItem::RadioSf:\n      case SettingsItem::RadioCr:\n      case SettingsItem::RadioPwr:\n      case SettingsItem::MeshPathSize:\n        sprintf(buf, \"- %d +\", edit_u8);\n", 1), (
    "20 + i * UI_TEXT_LINE_HEIGHT", "UI_MENU_TOP + i * UI_TEXT_LINE_HEIGHT", 2)])

patch('companion_radio/ui-cardputer/screen/ToolsScreen.cpp', [
    ("20 + i * UI_TEXT_LINE_HEIGHT", "UI_MENU_TOP + i * UI_TEXT_LINE_HEIGHT", 2),
    ("25 + i * UI_TEXT_LINE_HEIGHT", "(UI_MENU_TOP + 5) + i * UI_TEXT_LINE_HEIGHT", 2)])

patch('companion_radio/ui-cardputer/screen/MainScreen.cpp', [
    ("  int y = 16;\n", "  int y = UI_TEXT_LINE_HEIGHT + 4;\n", 1),
    ("  sprintf(tmp, \"%d\", batteryPercentage);\n  lcd.drawTextCentered(iconX + 2 + iconWidth / 2, 0, tmp);\n  lcd.setColor(CardputerDisplay::P_BLACK);\n  lcd.drawTextCentered(iconX + 2 + iconWidth / 2 - 1, 0, tmp);\n",
     "  if (UI_TEXT_LINE_HEIGHT <= 14) { // the percentage only fits inside the icon with the smaller fonts\n    sprintf(tmp, \"%d\", batteryPercentage);\n    lcd.drawTextCentered(iconX + 2 + iconWidth / 2, 0, tmp);\n    lcd.setColor(CardputerDisplay::P_BLACK);\n    lcd.drawTextCentered(iconX + 2 + iconWidth / 2 - 1, 0, tmp);\n  }\n", 1)])

patch('companion_radio/ui-cardputer/screen/main/ContactsPage.cpp', [
    ("lcd.drawTextCentered(lcd.width() / 2, 20, \"Contacts\");", "lcd.drawTextCentered(lcd.width() / 2, UI_PAGE_TOP, \"Contacts\");", 1),
    ("30 + i * UI_TEXT_LINE_HEIGHT", "UI_LIST_TOP + i * UI_TEXT_LINE_HEIGHT", 4)])

patch('companion_radio/ui-cardputer/screen/main/ChannelsPage.cpp', [
    ("lcd.drawTextCentered(lcd.width() / 2, 20, \"Channels\");", "lcd.drawTextCentered(lcd.width() / 2, UI_PAGE_TOP, \"Channels\");", 1),
    ("30 + i * UI_TEXT_LINE_HEIGHT", "UI_LIST_TOP + i * UI_TEXT_LINE_HEIGHT", 3)])

patch('companion_radio/ui-cardputer/screen/main/ChatPage.cpp', [
    ("display.drawTextCentered(display.width() / 2, 20, ", "display.drawTextCentered(display.width() / 2, UI_PAGE_TOP, ", 3),
    ("  int current_y = 35 + (UI_CHAT_HISTORY_LIST_SIZE - 1) * display.getFontLineHeight();\n",
     "  const int history_rows = UI_CHAT_HISTORY_LIST_SIZE;\n  int current_y = UI_CHAT_HISTORY_TOP + (history_rows - 1) * display.getFontLineHeight();\n", 1),
    ("    if (total_lines_drawn >= UI_CHAT_HISTORY_LIST_SIZE) {", "    if (total_lines_drawn >= history_rows) {", 1),
    ("    if (total_lines_drawn + message_lines > UI_CHAT_HISTORY_LIST_SIZE) {\n      message_lines = UI_CHAT_HISTORY_LIST_SIZE - total_lines_drawn;",
     "    if (total_lines_drawn + message_lines > history_rows) {\n      message_lines = history_rows - total_lines_drawn;", 1)])

patch('companion_radio/ui-cardputer/screen/main/StatsPage.cpp', [
    ("  int y = 20;\n  display.drawTextCentered(display.width() / 2, y, \"Stats\");\n",
     "  int y = UI_PAGE_TOP;\n  if (UI_TEXT_LINE_HEIGHT <= 14) {\n    display.drawTextCentered(display.width() / 2, y, \"Stats\");\n  } else {\n    y -= UI_TEXT_LINE_HEIGHT; // no room for a title with a large font; the page dots show where we are\n  }\n", 1)])

patch('companion_radio/ui-cardputer/screen/main/GpsPage.cpp', [("  int y = 18;\n", "  int y = UI_PAGE_TOP - 2;\n", 1)])

patch('companion_radio/ui-cardputer/screen/main/RecentAdvertsPage.cpp', [
    ("  int y = 20;\n", "  int y = UI_PAGE_TOP;\n", 1),
    ("  for (int i = 0; i < UI_RECENT_LIST_SIZE; i++, y += UI_TEXT_LINE_HEIGHT) {",
     "  for (int i = 0; i < UI_RECENT_LIST_SIZE && y + UI_TEXT_LINE_HEIGHT <= lcd.height(); i++, y += UI_TEXT_LINE_HEIGHT) {", 1)])
print('all patched')
