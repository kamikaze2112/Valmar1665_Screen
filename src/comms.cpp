#include <WiFi.h>
#include <esp_now.h>
#include "globals.h"
#include "comms.h"
#include "ui.h"
#include "prefs.h"

uint8_t controllerAddress[6];
uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

bool controllerPaired = false;
bool pairingMode = false;
int pairRequests = 0;

static esp_now_peer_info_t peerInfo;

// === Data holders ===
IncomingData incomingData = {};
OutgoingData outgoingData = {};


void printMac(const uint8_t *mac) {
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 0x10) Serial.print("0"); // Leading zero if needed
        Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
}


void addPeer(const uint8_t mac[6]) {

  memcpy(peerInfo.peer_addr, mac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.print("Failed to add peer: ");
    } else {
        Serial.print("Peer added: ");

        if (mac == broadcastAddress) {
            controllerPaired = false;
        } else {
              controllerPaired = true;
        }
    }
    printMac(mac);
}

// === Receive callback ===
  void onDataRecv(const uint8_t *mac, const uint8_t *incoming, int len) {
  
  PacketType type = static_cast<PacketType>(incoming[0]);

  if (type == PACKET_TYPE_PAIR_ACK) {
      memcpy(controllerAddress, mac, 6);

      Serial.print("Pairing ACK received from: ");
      printMac(controllerAddress);
      
      // Remove broadcast peer if needed
      esp_now_del_peer(broadcastAddress);

      // Add paired controller as a peer
      memcpy(peerInfo.peer_addr, controllerAddress, 6);
      peerInfo.channel = 0;
      peerInfo.encrypt = false;

      if (esp_now_add_peer(&peerInfo) != ESP_OK) {
          Serial.println("Failed to add controller as peer");
      } else {
          Serial.println("Controller added as peer");
          pairingMode = false;
          controllerPaired = true;
          saveComms();
          lv_obj_clear_state(ui_btnPairing, LV_STATE_DISABLED);
      }

  } else if (type == PACKET_TYPE_DATA) {
      if (len == sizeof(IncomingData)) {
          memcpy(&incomingData, incoming, min(len, (int)sizeof(IncomingData)));
          newData = true;  // Flag for main loop

          // Use live values from globals
          outgoingData.calibrationMode = calibrationMode;
          outgoingData.seedingRate = seedingRate;
          outgoingData.calibrationWeight = calibrationWeight;
          outgoingData.motorTestSwitch = motorTestSwitch;
          outgoingData.motorTestPWM = motorTestPWM;
          outgoingData.speedTestSwitch = speedTestSwitch;
          outgoingData.speedTestSpeed = speedTestSpeed;
          
          esp_now_send(mac, (uint8_t*)&outgoingData, sizeof(outgoingData));
      }
  }
}

// === Send callback ===
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
/*   DBG_PRINT("Reply send status: ");
  DBG_PRINTLN(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail"); */
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

  if (commsValid) {
      addPeer(controllerAddress);

    } else {
      addPeer(broadcastAddress);

    }
}

void sendPairingRequest() {
    if (pairRequests < 50) {
        Serial.print(pairRequests);
        Serial.print(" : ");  
        Serial.print("Sending Pairing request to: ");
        printMac(broadcastAddress);
        struct {
            PacketType type = PACKET_TYPE_PAIR_SEND;
        } pairingRequest;

        esp_now_send(broadcastAddress, (uint8_t*)&pairingRequest, sizeof(pairingRequest));
        pairRequests++;
    } else {
        Serial.println("Pairing failed after 50 attempts.  Is controller in pairing mode?");
        pairRequests = 0;
        pairingMode = false;
        lv_obj_clear_state(ui_btnPairing, LV_STATE_DISABLED);
    }
}