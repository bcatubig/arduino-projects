#include <FreeRTOS_SAMD21.h>
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <SPI.h>
#include <RH_RF95.h>

#define ERROR_LED_PIN 13
#define ERROR_LED_LIGHTUP_STATE LOW
#define STATUS_LED_PIN 14
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
#define RF95_FREQ 915.0

TaskHandle_t Handle_display;
TaskHandle_t Handle_temp;
TaskHandle_t Handle_LoRA;

Adafruit_SSD1306 display(128, 32, &Wire);
Adafruit_BMP280 bmp;
RH_RF95 rf95(RFM95_CS, RFM95_INT);

struct dataStruct {
  int sensorID;
  float temp_f;
  float pressure;
  float altitude;
} weatherData;

int sensor_id = 1;
float current_temp = 0.0;
float current_pressure = 0.0;
float current_altitude = 0.0;

static void blink(int delay, int count) {
  for (int i = 0; i < count; ++i) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    vTaskDelay(delay * portTICK_PERIOD_MS);
    digitalWrite(STATUS_LED_PIN, LOW);
    vTaskDelay(delay * portTICK_PERIOD_MS);
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);

  // Print Header
  display.println("Weather Node: 1");

  // Print Temperature
  display.print("Temp: ");
  display.print(current_temp);
  display.println(" F");

  // Print Pressure
  display.print("Pressure: ");
  display.print(current_pressure);
  display.println(" hPA");

  // Print Altitude
  display.print("Altitude: ");
  display.print(current_altitude);
  display.println(" m");

  // Show the display
  display.display();
}

static void threadTemp(void *pvParameters) {
  Serial.println("Thread Temp: Started");
  while (true) {
    Serial.println("Thread Temp: Reading BMP280 Sensor");
    current_temp = (bmp.readTemperature() * 1.8) + 32;
    current_pressure = bmp.readPressure() / 100;
    current_altitude = bmp.readAltitude(1019.0);
    vTaskDelay(5000 * portTICK_PERIOD_MS);
  }
}

static void threadDisplay(void *pvParameters) {
  Serial.println("Thread Display: Started");
  while (true) {
    updateDisplay();
    vTaskDelay(2000 * portTICK_PERIOD_MS);
  }
}

static void threadLora(void *pvParameters) {
  Serial.println("Thread LoRa: Started");
  byte tx_buf[sizeof(weatherData)] = {0};

  for (;;) {
    weatherData.sensorID = 1;
    weatherData.temp_f = current_temp;
    weatherData.pressure = current_pressure;
    weatherData.altitude - current_altitude;
    memcpy(tx_buf, &weatherData, sizeof(weatherData));

    byte tx_byte_size = sizeof(weatherData);
    rf95.send((uint8_t *) tx_buf, tx_byte_size);
    rf95.waitPacketSent();
    blink(200, 2);
    vTaskDelay(6000 / portTICK_PERIOD_MS);
  }
}

/*
 * Initialize Components
*/
static void init_display() {
  Serial.println("INFO: Initialing OLED Display");
  display.begin();
  display.display();
  vNopDelayMS(1000);
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
}

void init_bmp280() {
  if (!bmp.begin(0x76)) {
    Serial.println("ERROR: Failed to initialize BMP-280 Sensor");
    rtosFatalError();
  }
  Serial.println("INFO: Initialized BMP-280 Sensor");
}

bool init_rfm95() {
  digitalWrite(RFM95_RST, HIGH);

  // Manual reset
  digitalWrite(RFM95_RST, LOW);
  vNopDelayMS(100);
  digitalWrite(RFM95_RST, HIGH);
  vNopDelayMS(100);

  while (!rf95.init()) {
    Serial.println("ERROR: LoRA radio init failed");
    rtosFatalError();
  }

  Serial.println("INFO: LoRa radio init OK!");

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("ERROR: setFrequency failed");
    rtosFatalError();
  }

  Serial.print("INFO: Set freq to");
  Serial.println(RF95_FREQ);

  rf95.setTxPower(23, false);
}
void setup() {
  Serial.begin(115200);
  vNopDelayMS(1000); // Delay so that USB driver doesn't crash

  Serial.println("");
  Serial.println("********************");
  Serial.println("   Program Start    ");
  Serial.println("********************");
  init_display();
  init_bmp280();
  init_rfm95();

  vSetErrorLed(ERROR_LED_PIN, ERROR_LED_LIGHTUP_STATE);
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);

  //xTaskCreate(threadBlink, "LED BLINK", 256, NULL, tskIDLE_PRIORITY + 1, &Handle_ledBlink);
  xTaskCreate(threadDisplay, "DISPLAY", 256, NULL, tskIDLE_PRIORITY + 1, &Handle_display);
  xTaskCreate(threadTemp, "TEMPERATURE", 256, NULL, tskIDLE_PRIORITY + 1, &Handle_temp);
  xTaskCreate(threadLora, "LORA", 256, NULL, tskIDLE_PRIORITY + 1, &Handle_LoRA);
  vTaskStartScheduler();
}

void loop() {
};