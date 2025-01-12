#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <ESP32Time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <elapsedMillis.h>

const int PIN_VEXT_CTRL = 3;
const int PIN_GNSS_TX = 33;
const int PIN_GNSS_RX = 34;
const int PIN_TFT_LED_K = 21;
const int PIN_TFT_CS = 38;
const int PIN_TFT_RES = 39;
const int PIN_TFT_RS = 40;
const int PIN_TFT_SCLK = 41;
const int PIN_TFT_SDIN = 42;
const int SERIAL_BAUD_GNSS = 115200;
const int FIND_LOCATION_TIMEOUT_S = 60;

HardwareSerial serialGnss(0);
Adafruit_ST7735 display(PIN_TFT_CS, PIN_TFT_RS, PIN_TFT_SDIN, PIN_TFT_SCLK, PIN_TFT_RES);
TinyGPSPlus gnss;
ESP32Time rtc;

void setupExternalPower()
{
  // Enable power to peripherals
  pinMode(PIN_VEXT_CTRL, OUTPUT);
  digitalWrite(PIN_VEXT_CTRL, HIGH);
}

void setupDisplay()
{
  display.initR(INITR_MINI160x80_PLUGIN);
  display.setRotation(1); // Landscape mode
  display.cp437(true);
  display.fillScreen(ST7735_BLACK);
  display.setTextColor(ST7735_WHITE, ST7735_BLACK);

  // Enable backlight
  pinMode(PIN_TFT_LED_K, OUTPUT);
  digitalWrite(PIN_TFT_LED_K, HIGH);
}

bool locationFound()
{
  return gnss.location.isValid() && gnss.hdop.isValid() && gnss.hdop.hdop() <= 5;
}

void setupLocation()
{
  serialGnss.begin(SERIAL_BAUD_GNSS, SERIAL_8N1, PIN_GNSS_TX, PIN_GNSS_RX);

  display.fillScreen(ST7735_BLACK);
  display.setCursor(0, 0);
  display.print("Searching for location...");

  elapsedSeconds secondsSinceStart;
  do
  {
    while (serialGnss.available())
    {
      gnss.encode(serialGnss.read());
    }
  } while (!locationFound() && secondsSinceStart < FIND_LOCATION_TIMEOUT_S);

  serialGnss.end();
  int elapsedSeconds = secondsSinceStart;
  display.setCursor(0, ST7735_TFTWIDTH_80 / 2);
  if (elapsedSeconds >= FIND_LOCATION_TIMEOUT_S)
  {
    display.print("Timeout! Stopping...");
    while (true)
    {
      // Infinite loop
    }
  }
  else
  {
    display.print("Location found!");
    delay(2000);
  }
}

void setupTime()
{
  rtc.setTime(gnss.time.second(), gnss.time.minute(), gnss.time.hour(), gnss.date.day(), gnss.date.month(), gnss.date.year());
  display.fillScreen(ST7735_BLACK);
  display.setCursor(0, 0);
  display.print("Time set!");
  delay(2000);
}

void setup()
{
  setupExternalPower();
  setupDisplay();
  setupLocation();
  setupTime();
  display.fillScreen(ST7735_BLACK);
}

void loop()
{
}