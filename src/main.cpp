/* Import the libraries */
#include <Arduino.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <TFT_eSPI.h>

/* Define the Board model and Board version */

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

/* Define GPS Module Pin & Baudrate */
#define RXPin (35)
#define TXPin (33)

static const uint32_t GPSBaud = 9600;

/* Define libraries */
HardwareSerial hs(2);
TinyGPSPlus gps;
TFT_eSPI tft = TFT_eSPI();

/* Initialize the function */
static void smartDelay(unsigned long ms);
static void printFloat(float val, bool valid, int len, int prec);
static void printInt(unsigned long val, bool valid, int len);
static void printDate(TinyGPSDate &d);
static void printTime(TinyGPSTime &t);

void setup()
{
  /* Setup the baudrate */
  Serial.begin(115200);

  /* Setup GPS Module */
  hs.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin, false);

  /* Initialize the TFT Display */
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);

  if (TFT_BL > 0)
  {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
  }
}

void loop()
{
  /* Clear TFT Screen */
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);

  /* Display GPS */
  tft.print("Satellites: ");
  printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
  tft.print("\nHDOP: ");
  printFloat(gps.hdop.hdop(), gps.hdop.isValid(), 6, 1);
  tft.print("\nLatitude: ");
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  tft.print("\nLongitude: ");
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  tft.print("\nFix (Age): ");
  printInt(gps.location.age(), gps.location.isValid(), 5);
  tft.print("\nDate: ");
  printDate(gps.date);
  tft.print("\nTime: ");
  printTime(gps.time);
  tft.print("\nDate (Age): ");
  printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
  tft.print("\nChars: ");
  printInt(gps.charsProcessed(), true, 6);
  tft.print("\nSentences: ");
  printInt(gps.sentencesWithFix(), true, 10);
  tft.print("\nChecksum: ");
  printInt(gps.failedChecksum(), true, 9);

  smartDelay(1000);

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    tft.print(F("No GPS data received: check wiring"));
  }
}

/* Set the function */
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (hs.available())
      gps.encode(hs.read());
  } while (millis() - start < ms);
}

static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      tft.print('*');
    tft.print(' ');
  }
  else
  {
    tft.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i = flen; i < len; ++i)
      tft.print(' ');
  }
  smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  tft.print(sz);
  smartDelay(0);
}

static void printDate(TinyGPSDate &d)
{
  if (!d.isValid())
  {
    tft.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    tft.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

static void printTime(TinyGPSTime &t)
{
  if (!t.isValid())
  {
    tft.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    tft.print(sz);
  }

  printInt(t.age(), t.isValid(), 5);
  smartDelay(0);
}
