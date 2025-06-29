#pragma once
#include <Arduino.h>

// === Structures ===
struct IncomingData {
  bool calibrationMode;
  float seedingRate;
  float calibrationWeight;
} __attribute__((packed));

struct OutgoingData {
  int fixStatus;
  int numSats;
  float gpsSpeed;
  int gpsHour;
  int gpsMinute;
  int gpsSecond;
  double calibrationRevs;
  bool workSwitch;
  bool motorActive;
  double shaftRPM;
  int errorCode;
  float actualRate;
} __attribute__((packed));

// === Extern declarations ===
extern OutgoingData receivedData;
extern IncomingData replyData;
extern volatile bool newData;
extern bool calibrationMode;

// === Functions ===
void setupComms();

void printReplyData();