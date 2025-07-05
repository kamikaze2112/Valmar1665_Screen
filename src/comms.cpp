#include <WiFi.h>
#include <esp_now.h>
#include "globals.h"
#include "comms.h"
#include "ui.h"

// Replace with your slave MAC address
uint8_t slaveMac[] = { 0xD8, 0x3B, 0xDA, 0xA3, 0x7A, 0xEC };

// === Data holders ===
OutgoingData receivedData = {};
IncomingData replyData = {};

// === Receive callback ===
void onDataRecv(const uint8_t *mac, const uint8_t *incoming, int len) {
  if (len == sizeof(OutgoingData)) {
    memcpy(&receivedData, incoming, sizeof(receivedData));
    newData = true;  // Flag for main loop

    // Use live values from globals
    replyData.calibrationMode = calibrationMode;
    replyData.seedingRate = seedingRate;
    replyData.calibrationWeight = calibrationWeight;
    replyData.motorTestSwitch = motorTestSwitch;
    replyData.motorTestPWM = motorTestPWM;
    replyData.speedTestSwitch = speedTestSwitch;
    replyData.speedTestSpeed = speedTestSpeed;
    
    esp_now_send(mac, (uint8_t*)&replyData, sizeof(replyData));
  }
}


// === Send callback ===
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  DBG_PRINT("Reply send status: ");
  DBG_PRINTLN(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

// === Setup comms ===
void setupComms() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    DBG_PRINTLN("ESP-NOW init failed");
    while (true) delay(1000);
  }

  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);

  // Add slave as peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, slaveMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(slaveMac)) {
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      DBG_PRINTLN("Failed to add slave peer");
    }
  }

  DBG_PRINTLN("ESP-NOW Controller Ready");
}

void printReplyData() {
  DBG_PRINTLN("=== Reply Data ===");
  DBG_PRINT("Calibration Mode: ");
  DBG_PRINTLN(replyData.calibrationMode);
  DBG_PRINT("Seeding Rate: ");
  DBG_PRINTLN(replyData.seedingRate);
  DBG_PRINT("Calibration Weight: ");
  DBG_PRINTLN(replyData.calibrationWeight);
}