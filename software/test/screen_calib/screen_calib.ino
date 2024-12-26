#include <Adafruit_GFX.h>  // Core graphics library
#include <SPI.h>
#include <Wire.h>  // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>


#define STMPE_CS 13
#define TOUCH_RESPONSE_TIME 50

#define TFT_CS 15
#define TFT_DC 14
#define TFT_SIZE_X 320
#define TFT_SIZE_Y 240

XPT2046_Touchscreen ts(STMPE_CS, 18);

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

void setup() {

  SPI.setRX(4);
  SPI.setTX(3);
  SPI.setSCK(2);


  SPI1.setRX(12);
  SPI1.setTX(11);
  SPI1.setSCK(10);
  SPI1.setCS(13);

  tft.begin();
  tft.setRotation(3);

  ts.setRotation(3);
  
  if (!ts.begin(SPI1)) {
    Serial.println("Couldn't start touchscreen controller");
    while (1)
      ;
  }

  //Serial.println("Touchscreen started");
  tft.fillScreen(ILI9341_BLACK);

  Serial.begin(115200);
}

void loop() {
  if (ts.touched()) {
    uint16_t x, y;
    uint8_t z;
    ts.readData(&x, &y, &z);
    Serial.print(x);
    Serial.print(" ");
    Serial.print(y);
    Serial.print(" ");
    Serial.println(z);
  }
}
