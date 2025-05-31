#include <wireless_targets.h>

comm comm_data;
bool conns[DEVICE_COUNT] = {false};

bool game_over = false;
bool print_game_over = true;

bool waiting = false;
uint8_t round_count = 0;
float score = 0;

void recv_data(uint8_t *mac, uint8_t *data, uint8_t len) {
  memcpy(&comm_data, data, sizeof(comm_data));
  switch (comm_data.type) {
    case INIT:
      Serial.printf("INIT received from %d\n", comm_data.index);

      conns[comm_data.index] = true;

      break;
    case ENEMY:
      Serial.printf("ENEMY shot in %d ms\n", comm_data.time);

      score += 1000.0 / comm_data.time;
      ++round_count;

      break;
    case ALLY:
      Serial.println("ALLY shot: ending game");

      game_over = true;
      score = 0;

      break;
    case TIMEOUT:
      Serial.println("Target not shot");

      break;
  }

  waiting = false;
}

void send_init_conn(const uint8_t index) {
  comm_data.type = INIT;
  comm_data.index = index;
  int err = esp_now_send(device_addrs[index], (uint8_t *) &comm_data, sizeof(comm_data));
  if (!err) {
    Serial.printf("Successfully sent INIT data to: %d\n", index);
  } else {
    Serial.printf("Failed to send INIT data to: %d\n", index);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.printf("\n----------START----------\n");

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  for (uint8_t i = 1; i < DEVICE_COUNT; i++) {
    add_peer(i);
  }

  esp_now_register_recv_cb(recv_data);

  for (uint8_t i = 1; i < DEVICE_COUNT; i++) {
    while (!conns[i]) {
        send_init_conn(i);

        digitalWrite(LED, HIGH);
        delay(500);
        digitalWrite(LED, LOW);
        delay(500);
    }
  }

  Serial.println(">>> Device setup successful");
}

void loop() {
  if (round_count == 5 || game_over) {
    if (print_game_over) {
      Serial.println("GAME OVER");
      Serial.printf("SCORE: %f\n", score * 1000);
      print_game_over = false;
    }
    return;
  }

  if (waiting) {
    return;
  }

  delay(ESP8266TrueRandom.random(500, 2000));

  if (ESP8266TrueRandom.random(5) == 0) {
    comm_data.type = ALLY;
  } else {
    comm_data.type = ENEMY;
  }

  // uint8_t target_index = ESP8266TrueRandom.random(1, DEVICE_COUNT);
  uint8_t target_index = 1;

  int err = esp_now_send(device_addrs[target_index], (uint8_t *) &comm_data, sizeof(comm_data));
  if (!err) {
    Serial.printf("Successfully sent %c data to %d\n", comm_data.type, target_index);
  } else {
    Serial.printf("Failed to send %c data to %d\n", comm_data.type, target_index);
  }
  waiting = true;
}
