#include <ESP8266WiFi.h>
#include <espnow.h>

#include <ESP8266TrueRandom.h>

#define LED 16

#define D5  14
#define D6  12
#define D7  13
#define D8  15

#define INIT    'i'
#define ENEMY   'e'
#define ALLY    'a'
#define TIMEOUT 't'

#define DEVICE_COUNT 3

typedef struct comm {
  char type;
  uint8_t index;
  unsigned long time;
} comm;

uint8_t device_addrs[][6] = {
  {0xCC, 0x50, 0xE3, 0x16, 0x63, 0xF5},
  {0xA8, 0x48, 0xFA, 0xD6, 0x32, 0xD5},
  {0x84, 0xF3, 0xEB, 0x16, 0x68, 0xC8}
};

void add_peer(uint8_t index) {
    int err = esp_now_add_peer(device_addrs[index], ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    if (!err) {
      Serial.printf("Successfully added peer %d\n", index);
    } else {
      Serial.printf("Failed to add peer %d: %d\n", index, err);
    }
}
