#include <wireless_targets.h>

comm comm_data;
bool conns[DEVICE_COUNT] = {false};

bool waiting = false;
bool game_over = false;
uint8_t round_count = 0;
float score = 0;

void recv_data(const uint8_t *mac, const uint8_t *data, int len) {
  memcpy(&comm_data, data, sizeof(comm_data));
  switch (comm_data.type) {
    case INIT:
      Serial.print("INIT received from: ");
      Serial.println(comm_data.index);

      conns[comm_data.index] = true;
      break;
    case ENEMY:
      Serial.print("ENEMY shot in: ");
      Serial.println(comm_data.time);

      score += 1000 / comm_data.time;
      ++round_count;
      break;
    case ALLY:
      Serial.println("ALLY shot: ending game");

      game_over = true;
      break;
    case TIMEOUT:
      Serial.println("Target not shot");

      ++round_count;
      break;
  }

  waiting = false;
}

void send_init_conn(const uint8_t index) {
  comm_data.type = INIT;
  comm_data.index = index;
  esp_err_t result = esp_now_send(device_addrs[index], (uint8_t *) &comm_data, sizeof(comm_data));
  if (result == ESP_OK) {
    Serial.print("Successfully sent INIT data to: ");
    Serial.println(index);
  } else {
    Serial.print("Error sending INIT data to: ");
    Serial.println(index);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  randomSeed(analogRead(0));
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  for (uint8_t i = 1; i < DEVICE_COUNT; i++) {
    add_peer(i);
  }

  esp_now_register_recv_cb(esp_now_recv_cb_t(recv_data));

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
  if (waiting) {
    return;
  }

  if (round_count == 5) {
    game_over = true;
  }

  if (game_over) {
    Serial.println("GAME OVER");
    Serial.print("SCORE: ");
    Serial.println(score);
    while (true);
  }

  delay(random(100, 1000));

  if (random(5) == 0) {
    comm_data.type = ALLY;
  } else {
    comm_data.type = ENEMY;
  }

  uint8_t target_index = random(1, DEVICE_COUNT);

  esp_err_t result = esp_now_send(device_addrs[target_index], (uint8_t *) &comm_data, sizeof(comm_data));
  if (result == ESP_OK) {
    Serial.print("Successfully sent '");
    Serial.print(comm_data.type);
    Serial.print("' target data to: ");
    Serial.println(target_index);
  } else {
    Serial.print("Error sending '");
    Serial.print(comm_data.type);
    Serial.print("' target data to: ");
    Serial.println(target_index);
  }

  waiting = true;
}
