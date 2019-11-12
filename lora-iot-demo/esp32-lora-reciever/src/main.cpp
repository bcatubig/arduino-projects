#include "Arduino.h"
#include <esp_wifi.h>
#include <SPI.h>
#include <RH_RF95.h>

#define LED_BUILTIN 17
#define LED_ERROR 16

#define RFM95_CS 5
#define RFM95_RST 4
#define RFM95_INT 27
#define RF95_FREQ 915.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

TaskHandle_t Handle_Blink;
TaskHandle_t Handle_LoraListen;

void TaskBlink(void *pvParameters);
void TaskListen(void *pvParameters);
void errorBlink(String msg);

struct dataStruct {
  int sensorID;
  float temp_f;
  float pressure;
  float altitude;
} weatherData;

void init_lora() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  // Manual Reset
  digitalWrite(RFM95_RST, LOW);
  vTaskDelay(10 / portTICK_PERIOD_MS);
  digitalWrite(RFM95_RST, HIGH);
  vTaskDelay(10 / portTICK_PERIOD_MS);

  while (!rf95.init()) {

    for (;;) {
      errorBlink("LoRa radio init failed");
    }
  }

  Serial.println("INFO: LoRA Radio Initialized");

  if (!rf95.setFrequency(RF95_FREQ)) {
    errorBlink("Failed to set LoRA Frequency");
  }

  Serial.print("Frequency set to ");
  Serial.println(RF95_FREQ);
  rf95.setTxPower(23, false);

}

void setup() {
  Serial.begin(115200);

  init_lora();

  xTaskCreate(
      TaskBlink,
      "TaskBlink",
      1024,
      nullptr,
      1,
      &Handle_Blink
  );

  xTaskCreate(
      TaskListen,
      "TaskListen",
      1024,
      nullptr,
      1,
      &Handle_LoraListen
  );

}

void loop() {}

void TaskBlink(void *pvParameters) {
  pinMode(LED_BUILTIN, OUTPUT);
  for (;;) {
//    Serial.println("Task Blink: Blinking");
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(100);
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(100);
  }
}

void TaskListen(void *pvParameters) {
  Serial.println("Task Listen: Started");
  for (;;) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t buflen = sizeof(buf);
    int i;

    if (rf95.recv(buf, &buflen)) {
      ++i;
      Serial.println("Task Listen: Message Recieved");

      // Message is good, dump it
      rf95.printBuffer("Got: ", buf, buflen);
      memcpy(&weatherData, buf, sizeof(dataStruct));
      Serial.println();

      Serial.print("Sensor ID: ");
      Serial.println(weatherData.sensorID);

      Serial.print("Temp: ");
      Serial.println(weatherData.temp_f);

      Serial.print("Pressure: ");
      Serial.println(weatherData.pressure);

      Serial.print("Altitude: ");
      Serial.println(weatherData.altitude);

      Serial.print("Counter: ");
      Serial.println(i);
    }
    vTaskDelay(10);
  }
}

void errorBlink(String msg) {
  pinMode(LED_ERROR, OUTPUT);
  Serial.println(msg);
  digitalWrite(LED_ERROR, HIGH);
  vTaskDelay(100);
  digitalWrite(LED_ERROR, LOW);
  vTaskDelay(100);
}