# 2026.10.22.4 (opensourcera fork)

* Signal tester: traces always use 2-byte path hashes (repeaters in 2-byte regions ignore 1-byte traces), and when a
  repeater is chosen to go through, the answer comes back through it as well instead of over a weak direct path.

# 2026.10.22.3 (opensourcera fork)

* Chat: `Ctrl+Space` now cycles through ten quick replies, the "heard you" line (hops, SNR, RSSI) first. Your own
  list goes in `/meshcore_custom/quick_replies.txt` on the SD card.

# 2026.10.22.2 (opensourcera fork)

* Chat: `Ctrl+Space` fills the message box with a "heard you" reply to the last message received in that chat
  (hops, SNR, RSSI; `@[name]` prefix on channels). `Ctrl+Backspace` clears the message box.
* Keyboard layout switch is `Ctrl+L` in a chat (`Ctrl+Space` elsewhere, as before).

# 2026.10.22.1 (opensourcera fork)

* Settings > Font: Sans or Mono (DejaVu Sans Mono at 12, 14, 16 and 18 px). `Enter` toggles and saves.

# 2026.10.22 (opensourcera fork)

* Tools > Signal tester: pick a repeater (and optionally one to go through), send five traces ten seconds apart
  and see the SNR there and back for each, plus a reply count. `Esc` stops, `Enter` repeats.
* Tools menu and the repeater lists now ellipsize long names and page with the selected font.

# 2026.10.21 (opensourcera fork)

* Settings > Font size: 12 (original), 14, 16 or 18 px. Live preview while choosing, `Enter` saves, `Esc` reverts.
* Menus, lists, chat history and the message line follow the selected font; row counts adapt to the screen.
* Custom settings file: an older, shorter file is now read as far as it goes instead of being ignored, so settings
  survive the new field.
* Fonts generated from DejaVu Sans with `tools/make_vlw_font.py`.

# 2026.10.1

* Fix contact hop count display.
* Fix direct message ACK handling.
* Other small fixes.

# 2026.10.0

* Update MeshCore to 1.17.1
* Fix tools actions order
* `Ctrl+R` to reset path (instead of `R`)
* Block keys if message is sending

# 2026.7.3

* Add default scope (region) option to the settings
* Add search box to the contacts page
* Move layout icon to the status bar

# 2026.7.2

* Remove messages preview screen
* Add "New message" screen that appears when a new message is received when screen is off or current page is main
* Move unsynced messages count to the stats page
* After selecting chat and pressing `Esc`, current page is restored to the previous one

# 2026.7.1

* Multiple direct message sending attempts
* Press `Ctrl+T` to break resending
* Press `Ctrl+Up` to insert last sent message into the text field
* Add arrows to font
* Fix screen not updated on new message

# 2026.7.0

* Add versioning
* **Cardputer hardware abstraction rework**
  - Remove M5Unified, M5GFX, and M5Cardputer
  - Replace M5GFX with "LovyanGFX"
  - Replace M5Cardputer with "Adafruit TCA8418"
* **[BREAKING] Keyboard layout rework**
  - Support keyboard layers based on key modifiers (`Fn`, `OPT`, `Ctrl`, `Alt`, `Shift`)
  - Allow overriding the base layout
  - Layout file format changed (check the Wiki for details)
  - Improve chat page `Fn` + `Arrows` logic (an empty input line does not require pressing `Fn`; the logic is reversed when typing)
* **Power consumption reduced**
  - Sleep mode (power saving): from ~52mA to ~22mA
* Remove G0 button handling
* Add more Unicode blocks to font
* Move shutdown and reset to the tools menu
* Move node name to the settings menu and make it editable
* Move status icons to the left side
* Add unread icon
* Fix chat input line truncated at the wrong side
* Do not pop up the message preview in the chat page
