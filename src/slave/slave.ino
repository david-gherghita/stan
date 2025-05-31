#include <wireless_targets.h>

int err;
comm comm_data;
bool active;
unsigned long active_time;

void led_off() {
  digitalWrite(D6, LOW);
  digitalWrite(D7, LOW);
  digitalWrite(D8, LOW);
}

void led_red() {
  digitalWrite(D6, HIGH);
  digitalWrite(D7, LOW);
  digitalWrite(D8, LOW);
}

void led_green() {
  digitalWrite(D6, LOW);
  digitalWrite(D7, LOW);
  digitalWrite(D8, HIGH);
}

void led_blue() {
  digitalWrite(D6, LOW);
  digitalWrite(D7, HIGH);
  digitalWrite(D8, LOW);
}


void recv_data(uint8_t *mac, uint8_t *data, uint8_t len) {
  memcpy(&comm_data, data, sizeof(comm_data));
  switch (comm_data.type) {
    case INIT:
      Serial.println("received data: INIT");

      err = esp_now_send(device_addrs[0], (uint8_t *) &comm_data, sizeof(comm_data));
      if (!err) {
        Serial.printf("Successfully sent INIT confirmation\n");
      } else {
        Serial.printf("Error sending INIT confirmation: %d\n", err);
      }

      led_off();

      break;
    case ENEMY:
      Serial.println("received data: ENEMY - lighting up red");

      led_red();

      active = true;
      active_time = millis();

      break;
    case ALLY:
      Serial.println("received data: ALLY - lighting up green");

      led_green();

      active = true;
      active_time = millis();

      break;
  }

  digitalWrite(D5, HIGH);
  delay(200);
  digitalWrite(D5, LOW);
}

void setup() {
  pinMode(A0, INPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);

  led_blue();

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.printf("\n----------START----------\n");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  add_peer(0);

  esp_now_register_recv_cb(recv_data);

  active = false;

  Serial.println("Device setup successful");
}

void loop() {
  if (!active) {
    return;
  }

  Serial.println(analogRead(A0));
  if (analogRead(A0) > 150) {
    Serial.println("HIT");

    comm_data.time = millis() - active_time;

    int err = esp_now_send(device_addrs[0], (uint8_t *) &comm_data, sizeof(comm_data));
    if (!err) {
      printf("Successfully sent %c to master\n", comm_data.type);
    } else {
      printf("Failed to send %c to master: %d\n", comm_data.type, err);
    }

    led_off();
    active = false;
  }

  if (comm_data.type == ALLY && millis() - active_time > 1000) {
    Serial.println("TIMEOUT");

    comm_data.type = TIMEOUT;

    int err = esp_now_send(device_addrs[0], (uint8_t *) &comm_data, sizeof(comm_data));
    if (!err) {
      printf("Successfully sent %c to master\n", comm_data.type);
    } else {
      printf("Failed to send %c to master: %d\n", comm_data.type, err);
    }

    led_off();
    active = false;
  }

  delay(10);
}
