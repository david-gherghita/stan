#include <esp_now.h>
#include <WiFi.h>

#define LED 2

#define INIT 'i'
#define ENEMY 'e'
#define ALLY 'a'
#define TIMEOUT 't'

#define DEVICE_COUNT 3

typedef struct comm {
  char type;
  uint8_t index;
  float time;
} comm;

uint8_t device_addrs[][ESP_NOW_ETH_ALEN] = {
  {0x30, 0xC6, 0xF7, 0x2F, 0x2A, 0x60}, // ESP32 DEVKIT V1
  {0x78, 0xE3, 0x6D, 0xDE, 0xF0, 0x98}, // ESP32-DevKitC V4
  {0x58, 0xBF, 0x25, 0x9A, 0x4D, 0x24}  // NodeMCU ESP-32S
};

void add_peer(uint8_t index) {
    esp_now_peer_info_t peer_info = {};
    memcpy(peer_info.peer_addr, device_addrs[index], ESP_NOW_ETH_ALEN);
    peer_info.channel = 0;
    peer_info.encrypt = false;
    if (esp_now_add_peer(&peer_info) != ESP_OK){
      Serial.print("Failed to add peer: ");
      Serial.println(index);
    }
}
