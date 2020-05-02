// The first time you need to define the board model and version

// #define T4_V12
#define T4_V13
// #define T10_V14
// #define T10_V18
// #define T10_V20

#if defined(T10_V18)
#include "T10_V18.h"
#elif defined(T10_V14)
#include "T10_V14.h"
#elif defined(T4_V12)
#include "T4_V12.h"
#elif defined(T4_V13)
#include "T4_V13.h"
#elif defined(T10_V20)
#include "T10_V20.h"
#else
#error "Please select board version."
#endif

#include <TFT_eSPI.h>
#include <WiFi.h>
#include <Wire.h>

TFT_eSPI tft = TFT_eSPI();

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.printf("Current select %s version\n", BOARD_VRESION);
  Serial.printf("TFT_MISO:%d\n", TFT_MISO);
  Serial.printf("TFT_MOSI:%d\n", TFT_MOSI);
  Serial.printf("TFT_SCLK:%d\n", TFT_SCLK);
  Serial.printf("TFT_CS:%d\n", TFT_CS);
  Serial.printf("TFT_DC:%d\n", TFT_DC);
  Serial.printf("TFT_RST:%d\n", TFT_RST);
  Serial.printf("TFT_BL:%d\n", TFT_BL);

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_WHITE);
  tft.setTextSize(1);
  tft.setTextColor(TFT_BLACK);

  if (TFT_BL > 0)
  {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
  }
}

void loop()
{
  tft.setTextColor(TFT_BLACK, TFT_DARKGREY);
  tft.drawString("Hello, World", tft.width() / 2, tft.height() / 2);
  tft.fillScreen(TFT_BLACK);
}