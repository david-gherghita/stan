#include <wireless_targets.h>

comm comm_data;

void recv_data(const uint8_t *mac, const uint8_t *data, int len) {
  memcpy(&comm_data, data, sizeof(comm_data));
  switch (comm_data.type) {
    case INIT:
      Serial.println("INIT received");
      break;
    case ENEMY:
      Serial.println("ENEMY: lighting up red");
      break;
    case ALLY:
      Serial.println("ALLY: lighting up green");
      break;
  }


  if (comm_data.type != INIT) {
    delay(random(3000, 10000));

    if (random(2) == 0) {
      comm_data.time = random(1000, 2000);
    } else {
      comm_data.type = TIMEOUT;
    }
  }

  esp_err_t result = esp_now_send(device_addrs[0], (uint8_t *) &comm_data, sizeof(comm_data));
  if (result == ESP_OK) {
    Serial.println("Successfully sent data");
  } else {
    Serial.println("Error sending data");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  add_peer(0);

  esp_now_register_recv_cb(esp_now_recv_cb_t(recv_data));

  Serial.println(">>> Device setup successful");
  digitalWrite(LED, HIGH);
}

void loop() {
}
