#include <Arduino.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>

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

SPIClass sdSPI(VSPI);
#define IP5306_ADDR 0X75
#define IP5306_REG_SYS_CTL0 0x00

#define RXPin (35)
#define TXPin (33)

static const uint32_t GPSBaud = 9600;

HardwareSerial hs(2);
TinyGPSPlus gps;
TFT_eSPI tft = TFT_eSPI();

static void smartDelay(unsigned long ms);
static void printFloat(float val, bool valid, int len, int prec);
static void printInt(unsigned long val, bool valid, int len);
static void printDate(TinyGPSDate &d);
static void printTime(TinyGPSTime &t);
String setFilename(TinyGPSDate &d);
void writeRoot(fs::FS &fs);

unsigned long last1 = 0;
unsigned long last2 = 0;
char filename[12];

void setup()
{
  Serial.begin(115200);

  hs.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin, false);

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

  Serial.print("Initializing SD Card...   ");
  if (SD_CS > 0)
  {
    sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS, sdSPI))
    {
      Serial.println("Failed!");
      tft.setCursor(0, 223);
      tft.println("SD Card: Mount failed!");
      while (1)
        ;
    }
    else
    {
      Serial.println("Success!");
      tft.setCursor(0, 223);
      tft.println("SD Card: Mount Success!");
    }
  }
}

void loop()
{
  unsigned long now1 = millis() + 1000;
  unsigned long now2 = millis() + 1000;

  if (now1 - last1 >= 1000)
  {
    last1 = now1;

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(0, 223);

    if (millis() > 5000 && gps.charsProcessed() < 10)
    {
      tft.print(F("No GPS data received: check wiring"));
    }

    tft.setCursor(0, 0);
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
    tft.print("\nAlt (m): ");
    printFloat(gps.course.deg(), gps.course.isValid(), 7, 2);
    tft.print("\nCourse Speed Card: ");
    printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
    tft.print("\nChars: ");
    printInt(gps.charsProcessed(), true, 6);
    tft.print("\nSentences: ");
    printInt(gps.sentencesWithFix(), true, 10);
    tft.print("\nChecksum: ");
    printInt(gps.failedChecksum(), true, 9);

    smartDelay(1000);
  }

  if (now2 - last2 >= 10000)
  {
    last2 = now2;

    writeRoot(SD);
  }
}

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
    sprintf(sz, "%02d/%02d/%04d ", d.month(), d.day(), d.year());
    tft.print(sz);
  }
  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

static void printTime(TinyGPSTime &t)
{
  char sz[32];
  if (t.isValid())
  {
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
  }

  printInt(t.age(), t.isValid(), 5);
  smartDelay(0);
}

String setFilename(TinyGPSDate &d)
{
  if (!d.isValid())
  {
    sprintf(filename, "/NULLFiles.csv");
  }
  else
  {
    sprintf(filename, "/%02d%02d%04d.csv", d.day(), d.month(), d.year());
  }

  return (String)filename;
}

void writeRoot(fs::FS &fs)
{
  File root = fs.open(setFilename(gps.date), FILE_APPEND);
  Serial.print("SD Card Write...   ");
  if (root)
  {
    tft.setCursor(0, 223);
    tft.print("\nSD Card: Working!");
    Serial.println("Success");

    root.print(gps.satellites.isValid());
    root.print(",");
    root.print(gps.satellites.value());
    root.print(",");
    root.print(gps.hdop.hdop());
    root.print(",");
    root.print(gps.location.lat());
    root.print(",");
    root.print(gps.location.lng());
    root.print(",");
    root.print(gps.location.age());
    root.print(",");
    root.print(gps.date.month());
    root.print("/");
    root.print(gps.date.day());
    root.print("/");
    root.print(gps.date.year());
    root.print(",");
    root.print(gps.time.hour());
    root.print(":");
    root.print(gps.time.minute());
    root.print(":");
    root.print(gps.time.second());
    root.print(",");
    root.print(gps.altitude.meters());
    root.print(",");
    root.print(gps.course.deg());
    root.print(",");
    root.print(gps.speed.kmph());
    root.print(",");
    root.print(gps.charsProcessed());
    root.print(",");
    root.print(gps.sentencesWithFix());
    root.print(",");
    root.println(gps.failedChecksum());

    root.close();
  }
  else
  {
    Serial.println("Failed");
    tft.setCursor(0, 223);
    tft.print("\nSD Card: Failed to write");
  }
}
