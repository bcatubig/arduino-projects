#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
Adafruit_BMP280 bmp;

void setup() {

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

void loop() {
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

  display.setCursor(0,0);
  display.display();

  delay(1000);
  yield();
  display.clearDisplay();

}