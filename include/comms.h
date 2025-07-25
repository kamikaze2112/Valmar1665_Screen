#pragma once
#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h> 

// === Structures ===

// Define packet types
enum PacketType : uint8_t {
    PACKET_TYPE_DATA = 0,
    PACKET_TYPE_PAIR_SEND = 1,
    PACKET_TYPE_PAIR_ACK = 2
};

struct OutgoingData {
  PacketType type = PACKET_TYPE_DATA;
  bool calibrationMode;
  float seedingRate;
  float calibrationWeight;
  bool calcSeedPerRev;
  bool motorTestSwitch;
  int motorTestPWM;
  bool speedTestSwitch;
  float speedTestSpeed;
  float workingWidth;
  int numberOfRuns;
  float newSeedPerRev;
  bool manualSeedUpdate;
} __attribute__((packed));

struct IncomingData {
  PacketType type = PACKET_TYPE_DATA;
  int fixStatus;
  int numSats;
  float gpsSpeed;
  int gpsHour;
  int gpsMinute;
  int gpsSecond;
  double calibrationRevs;
  bool workSwitch;
  bool motorActive;
  float seedPerRev;
  double shaftRPM;
  int errorCode;
  float actualRate;
  char controllerVersion[12];
} __attribute__((packed));

// === Extern declarations ===
extern IncomingData incomingData;
extern OutgoingData outgoingData;
extern volatile bool newData;
extern bool calibrationMode;

// === Functions ===
void setupComms();

void printMac(const uint8_t *mac);

void addPeer(const uint8_t mac[6]);

void sendPairingRequest();