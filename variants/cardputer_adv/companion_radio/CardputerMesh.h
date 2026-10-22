#pragma once

#include "CardputerDataStore.h"
#include "CardputerUITask.h"
#include "types.h"

#include <MyMesh.h>

#define CTL_TYPE_NODE_DISCOVER_REQ  0x80
#define CTL_TYPE_NODE_DISCOVER_RESP 0x90

#define HEARD_LOG_SIZE 8

/** How a received message was heard: for the "heard you" reply (Ctrl+H in a chat) */
struct HeardInfo {
  uint8_t key[8];      // channel secret or contact public key, first 8 bytes
  bool is_channel;
  bool valid;
  uint8_t hops;
  float snr;
  float rssi;
  char sender[32];
};

class CardputerMesh : public MyMesh {
  CardputerUITask *_ui;
  CardputerDataStore *_store;
  uint64_t rx_packet_count = 0;
  uint32_t last_ping_tag = 0;
  uint32_t last_discover_tag = 0;
  uint32_t last_signal_tag = 0;
  float last_rx_rssi = 0;
  HeardInfo heard_log[HEARD_LOG_SIZE];
  uint8_t heard_log_next = 0;
  void logHeard(const uint8_t *key, bool is_channel, mesh::Packet *pkt, const char *sender);
  uint8_t last_message_hash[MAX_HASH_SIZE] = { 0 };
  uint16_t last_message_heard_repeats = 0;
  CustomNodePrefs _custom_prefs{ .power_save = 0, .battery_correction = 1.0f, .font_size = 0, .font_face = 0 };
  LastSentMessage last_msg;
  unsigned long resend_timeout = 0;

public:
  CardputerMesh(mesh::Radio &radio, mesh::RNG &rng, mesh::RTCClock &rtc, SimpleMeshTables &tables,
                CardputerDataStore &store, CardputerUITask *ui)
      : MyMesh(radio, rng, rtc, tables, store, ui), _ui(ui), _store(&store) {}

  void onTraceRecv(mesh::Packet *packet, uint32_t tag, uint32_t auth_code, uint8_t flags,
                   const uint8_t *path_snrs, const uint8_t *path_hashes, uint8_t path_len) override;
  void onControlDataRecv(mesh::Packet *packet) override;
  mesh::DispatcherAction onRecvPacket(mesh::Packet *pkt) override;
  void onChannelMessageRecv(const mesh::GroupChannel &channel, mesh::Packet *pkt, uint32_t timestamp,
                            const char *text) override;
  void onMessageRecv(const ContactInfo &from, mesh::Packet *pkt, uint32_t sender_timestamp,
                     const char *text) override;
  void sendFloodScoped(const mesh::GroupChannel &channel, mesh::Packet *pkt, uint32_t delay_millis) override;

  // not virtual; using function hiding
  bool sendGroupMessage(uint32_t timestamp, mesh::GroupChannel &channel, const char *sender_name,
                        const char *text, int text_len);

  int sendDirectMessage(const ContactInfo &recipient, uint32_t timestamp, const char *text);

  void logRxRaw(float snr, float rssi, const uint8_t raw[], int len) override;

  void checkResend();

  ContactInfo *processAck(const uint8_t *data) override;

  inline uint64_t receivedPacketsCount() const { return rx_packet_count; }

  void begin(bool has_display);
  void savePrefs();
  bool sendPing(ContactInfo &contact);
  bool sendAdvert(bool flood);
  bool sendRepeatersDiscover();
  /** Signal tester: trace to target, optionally through via. Returns the tag used. */
  bool sendSignalTrace(const ContactInfo *via, const ContactInfo &target, uint32_t &tag);
  /** Most recent message heard in a chat (channel secret or contact pubkey), false if none */
  bool getLastHeard(const uint8_t *key, bool is_channel, HeardInfo &out);
  uint32_t getLastPingTag() const { return last_ping_tag; }
  inline CustomNodePrefs *getCustomNodePrefs() { return &_custom_prefs; }
  void loadMessageHistory(const uint8_t pkey[PUB_KEY_SIZE], bool is_channel, ChatHistory &history);
  void loop();
  void cancelResending();
  bool isMessageSending() { return last_msg.state == MESSAGE_SENDING; }
};

extern CardputerMesh the_mesh_cp;
