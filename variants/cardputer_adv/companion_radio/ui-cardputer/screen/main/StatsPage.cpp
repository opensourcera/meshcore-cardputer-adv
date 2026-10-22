#include "StatsPage.h"

int StatsPage::render(CardputerDisplay &lcd) {
  char tmp[80];

  int y = UI_PAGE_TOP;
  if (UI_TEXT_LINE_HEIGHT <= 14) {
    display.drawTextCentered(display.width() / 2, y, "Stats");
  } else {
    y -= UI_TEXT_LINE_HEIGHT; // no room for a title with a large font; the page dots show where we are
  }

  display.setColor(CardputerDisplay::P_YELLOW);

  y += UI_TEXT_LINE_HEIGHT;
  sprintf(tmp, "Radio noise floor: %d", radio_driver.getNoiseFloor());
  display.drawTextLeftAlign(5, y, tmp);

  y += UI_TEXT_LINE_HEIGHT;
  sprintf(tmp, "Heap usage: %u/%u (%u%%)", (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getHeapSize(),
          (unsigned)((ESP.getHeapSize() - ESP.getFreeHeap()) * 100 / ESP.getHeapSize()));
  display.drawTextLeftAlign(5, y, tmp);

  y += UI_TEXT_LINE_HEIGHT;
  sprintf(tmp, "Packets received: %llu", (unsigned long long)the_mesh_cp.receivedPacketsCount());
  display.drawTextLeftAlign(5, y, tmp);

  y += UI_TEXT_LINE_HEIGHT;
  sprintf(tmp, "Battery: %umV", _p->getUiTask()->getBattMilliVoltsCorrected());
  display.drawTextLeftAlign(5, y, tmp);

  y += UI_TEXT_LINE_HEIGHT;
  sprintf(tmp, "Unsynced messages: %d", _p->getUiTask()->getMsgCount());
  display.drawTextLeftAlign(5, y, tmp);

  return 5000;
}
