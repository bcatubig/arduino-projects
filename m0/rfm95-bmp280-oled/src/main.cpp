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

  display.print(F("Temp: "));
  display.print(bmp.readTemperature());
  display.print(" ");
  display.print((char)247);
  display.println("C");

  display.print(F("Pressure: "));
  display.print(bmp.readPressure());
  display.println(" Pa");

  display.print(F("Alt: "));
  display.print(bmp.readAltitude(1013.25)); /* Adjusted to local forecast! */
  display.println(" m");

  char radiopacket[20] = "Hello World #      ";
  itoa(packetnum++, radiopacket + 13, 10);
  radiopacket[19] = 0;

  delay(10);
  rf95.send((uint8_t *)radiopacket, 20);

  delay(10);
  rf95.waitPacketSent();

  display.print("LoRa: Sent ");
  display.print(packetnum);
  display.println(" packets");

  display.setCursor(0, 0);
  display.display();

  delay(1000);
  yield();
  display.clearDisplay();
}