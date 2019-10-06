#include <Arduino.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RH_RF95.h>

// for feather m0
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

// Setup 1306 Display
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
Adafruit_BMP280 bmp;

// Setyp LoRa radio

// Set to 915 - US Region
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup()
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  while (!Serial)
  {
    delay(1);
  }

  delay(100);

  Serial.println("Feather LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init())
  {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1)
      ;
  }

  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ))
  {
    Serial.println("setFrequency failed");
    while (1)
      ;
  }

  Serial.print("Set Freq to: ");
  Serial.println(RF95_FREQ);

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);

  // BMP280 default address is 0x76
  bmp.begin(0x76);

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  //

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.display(); // actually display all of the above
}

int16_t packetnum = 0;

void loop()
{
  display.clearDisplay();

  float current_temp, temp_f, current_pressure, current_altitude;
  current_temp = bmp.readTemperature();         // Returns temp in Celsius
  temp_f = current_temp * 1.8 + 32;             // Murica'
  current_pressure = bmp.readPressure();        // I bought the wrong sensor so ya'll get pressure
  current_altitude = bmp.readAltitude(1013.25); /* Adjusted to local forecast! */

  display.print(F("Temp: "));
  display.print(temp_f);
  display.print(" ");
  display.print((char)247);
  display.println("F");

  display.print(F("Pressure: "));
  display.print(current_pressure);
  display.println(" Pa");

  display.print(F("Alt: "));
  display.print(current_altitude);
  display.println(" m");

  /*
  LoRA THings
  */
  uint8_t data[20];
  data[0] = (uint8_t)temp_f;

  rf95.send(data, sizeof(data));

  rf95.waitPacketSent();
  packetnum++;

  display.print("LoRa: Sent ");
  display.print(packetnum);
  display.println(" packets");

  display.setCursor(0, 0);
  display.display();

  // Sleep 10 seconds
  delay(10000);
  yield();
}