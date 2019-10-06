#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <FreeRTOS_SAMD21.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <Wire.h>
#include <avr/dtostrf.h>

#define ERROR_LED_PIN 13
#define ERROR_LED_LIGHTUP_STATE HIGH

// ***************************************************************************
// FreeRTOS Tasks
// ***************************************************************************
TaskHandle_t Handle_helloWorldTask;
TaskHandle_t Handle_goodNightWorldTask;

// ***************************************************************************
// Setup 1306 Display
// ***************************************************************************
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
Adafruit_BMP280 bmp;

// ***************************************************************************
// LoRa Settings
// The following pins are for the m0 express
// ***************************************************************************
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
#define RF95_FREQ 915.0  // USA Frequency

// ***************************************************************************
// Singleton instance of the radio driver
// ***************************************************************************
RH_RF95 rf95(RFM95_CS, RFM95_INT);

bool init_display() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    return false;
  }  // Address 0x3C for 128x32

  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.display();  // actually display all of the above

  return true;
}

bool init_bmp() {
  // BMP280 default address is 0x76
  if (!bmp.begin(0x76)) {
    return false;
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  return true;
}

bool init_lora() {
  if (!rf95.init()) {
    Serial.println("Failed to initialize LoRa radio");
    return false;
  }
  return true;
}

void read_weather_sensor() {
  float current_temp, temp_f, current_pressure, current_altitude;
  current_temp = bmp.readTemperature();  // Returns temp in Celsius
  temp_f = current_temp * 1.8 + 32;      // Murica'
  current_pressure =
      bmp.readPressure();  // I bought the wrong sensor so ya'll get pressure
  current_altitude =
      bmp.readAltitude(1013.25); /* Adjusted to local forecast! */
}

//  Handle_helloWorldTask;
//  Handle_goodNightWorldTask;

static void helloWorld(void *pvParameters) {
  Serial.println("Hello World: Started");
  while (1) {
    Serial.println("Hello, World!");
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

static void goodNightWorld(void *pvParameters) {
  Serial.println("Hello World: Started");
  while (1) {
    Serial.println("Goodnight, World!");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  vNopDelayMS(1000);
  while (!Serial)
    ;

  Serial.println("");
  Serial.println("********************");
  Serial.println("feather m0 LoRa");
  Serial.println("********************");

  vSetErrorLed(ERROR_LED_PIN, ERROR_LED_LIGHTUP_STATE);

  if (!init_bmp()) {
    Serial.println("Failed to initalize BMP Sensor");
    while (1)
      ;
  }

  if (!init_display()) {
    Serial.println("Failed to initalize OLED");
    while (1)
      ;
  }

  xTaskCreate(helloWorld, "helloWorld", 256, NULL, tskIDLE_PRIORITY + 2,
              &Handle_helloWorldTask);
  xTaskCreate(goodNightWorld, "goodNightWorld", 256, NULL, tskIDLE_PRIORITY + 3,
              &Handle_goodNightWorldTask);

  // Start the scheduler
  vTaskStartScheduler();
}

void loop() {
  Serial.print(".");
  vNopDelayMS(100);
}
