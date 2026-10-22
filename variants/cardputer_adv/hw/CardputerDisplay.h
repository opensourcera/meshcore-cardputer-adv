#pragma once

#if USE_SD_CARD
  #include <SD.h>
  #include "CardputerDataStore.h"
#endif
#include <LovyanGFX.hpp>
#include <helpers/ui/DisplayDriver.h>


class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 panel_instance_;
  lgfx::Bus_SPI bus_instance_;
  lgfx::Light_PWM light_instance_;

public:
  LGFX();
};


class CardputerDisplay : public DisplayDriver {
private:
  bool _isOn = false;
  uint16_t _lastColor = 0;
  int16_t _fontYAdvance = 1;
  uint8_t _font_idx = 0;
  uint8_t _font_face = 0;
  LGFX LCD;

  void updateFontYAdvance();

public:
  enum PaletteColor : uint16_t {
    P_BLACK = TFT_BLACK,
    P_WHITE = TFT_WHITE,
    P_GREEN = TFT_GREEN,
    P_BLUE = TFT_BLUE,
    P_RED = TFT_RED,
    P_ORANGE = TFT_ORANGE,
    P_YELLOW = TFT_YELLOW,
    P_GRAY = TFT_GRAY,
  };

  CardputerDisplay() : DisplayDriver(240, 135) {}
  bool begin();
  void tryLoadUserFont();

  /** Built-in font sizes selectable in Settings */
  static const uint8_t FONT_COUNT = 4;
  /** Built-in font faces: 0 = Sans, 1 = Mono */
  static const uint8_t FACE_COUNT = 2;
  static const char *fontFaceName(uint8_t face);
  bool setFontFace(uint8_t face);
  uint8_t getFontFace() const { return _font_face; }
  static uint8_t fontPixels(uint8_t idx);
  bool setFontSize(uint8_t idx);
  uint8_t getFontSize() const { return _font_idx; }
  /** Row pitch used by lists and menus for the current font */
  int16_t getUiLineHeight() const;

  bool isOn() override { return _isOn; };
  void turnOn() override;
  void turnOff() override;
  void clear() override;
  void startFrame(ColorVal bkg = UIColor::window_bkg) override;
  void setTextSize(int sz) override;
  void setColor(ColorVal c) override;
  void setCursor(int x, int y) override;
  void print(const char *str) override;
  void fillRect(int x, int y, int w, int h) override;
  void drawRect(int x, int y, int w, int h) override;
  void drawXbm(int x, int y, const uint8_t *bits, int w, int h) override;
  void endFrame() override;
  uint16_t getTextWidth(const char *str) override;
  int32_t getFontHeight() const;
  int16_t getFontLineHeight() const;
  void drawTextLeftAlignWithScroll(int x, int y, int available_width, const char *text);
};
