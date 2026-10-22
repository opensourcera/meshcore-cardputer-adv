#pragma once

#include "../../CardputerMesh.h"
#include "../CardputerUITask.h"

#define SIGNAL_TEST_TRIES        5
#define SIGNAL_TEST_INTERVAL_MS  10000

class ToolsScreen : public CardputerScreen {
  enum ToolsPage { MenuPage, DiscoverPage, SignalTargetPage, SignalViaPage, SignalRunPage };
  enum ToolsMenuItem {
    MeshcoreSeparator,
    AdvertZeroHop,
    AdvertFlood,
    DiscoverRepeaters,
    SignalTester,
    DeviceSeparator,
    PowerOff,
    Restart,
    Count
  };

  struct DiscoveredRepeater {
    char name[25];
    float snr;
  };

  struct SignalTry {
    uint32_t tag;
    bool sent;
    bool returned;
    float there;   // SNR heard at the target
    float back;    // SNR heard on the way back (at the via node, or here for a direct test)
  };

  const char *menu_item_labels[ToolsMenuItem::Count] = {
    "---- RADIO ----",
    "Advert (Zero Hop)",
    "Advert (Flood)",
    "Discover repeaters",
    "Signal tester",
    "---- DEVICE ----",
    "Power off",
    "Restart",
  };

  CardputerUITask *_task;
  mesh::RTCClock *_rtc;

  ToolsPage page = ToolsPage::MenuPage;

  int menu_index = 0;

  DiscoveredRepeater discovered_repeaters[MAX_DISCOVERED_REPEATERS];
  uint16_t discovered_repeaters_count = 0;

  // Signal tester: pick a target repeater, optionally a repeater to go through, then send a few traces
  int signal_list_idx = 0;
  int signal_target_idx = -1;      // contact slot of the target
  int signal_via_idx = -1;         // contact slot of the via repeater, -1 = direct
  char signal_target_name[32];
  char signal_via_name[32];
  SignalTry signal_tries[SIGNAL_TEST_TRIES];
  int signal_sent = 0;
  int signal_returned = 0;
  bool signal_running = false;
  unsigned long signal_next_send = 0;

  void renderRepeaterList(CardputerDisplay &lcd, const char *title, bool with_direct, int exclude_idx);
  void renderSignalRun(CardputerDisplay &lcd);
  void startSignalTest();
  void sendSignalTry();
  void stopSignalTest();

public:
  ToolsScreen(CardputerUITask *task, mesh::RTCClock *rtc) : _task(task), _rtc(rtc) {}
  int render(CardputerDisplay &lcd) override;
  void poll() override;
  int repeaterCount(int exclude_idx);
  bool repeaterByListIdx(int list_idx, int exclude_idx, ContactInfo &contact, int &real_idx);
  void menuItemEnter(ToolsMenuItem item);
  bool handleInput(Keyboard::Event &e) override;
  void discoverRecv(const mesh::Identity &id, float snr);
  void signalTraceRecv(uint32_t tag, const uint8_t *snrs, uint8_t count, float final_snr);

};
