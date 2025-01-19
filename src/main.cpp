#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <ESP32Time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <elapsedMillis.h>
#include <CoordTopocentric.h>
#include <Observer.h>
#include <SGP4.h>

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
elapsedMillis millisSinceStart;

libsgp4::Observer observer(0, 0, 0);
libsgp4::SGP4 satellite(libsgp4::Tle("AO-07",
                                     "1 07530U 74089B   25018.84567510 -.00000039  00000-0  46297-4 0  9990",
                                     "2 07530 101.9918  21.6106 0012315  33.6228  35.2087 12.53685076296306"));

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

void setupLocationAndTime()
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

  int elapsedSeconds = secondsSinceStart;
  if (elapsedSeconds >= FIND_LOCATION_TIMEOUT_S)
  {
    serialGnss.end();
    display.setCursor(0, ST7735_TFTWIDTH_80 / 2);
    display.print("Timeout! Stopping...");
    while (true)
    {
      // Infinite loop
    }
  }
  else
  {
    rtc.setTime(gnss.time.second(), gnss.time.minute(), gnss.time.hour(), gnss.date.day(), gnss.date.month(), gnss.date.year());
    observer = libsgp4::Observer(gnss.location.lat(), gnss.location.lng(), gnss.altitude.kilometers());
    serialGnss.end();
    display.setCursor(0, ST7735_TFTWIDTH_80 / 2);
    display.print("Location found and time configured!");
    delay(2000);
  }
}

void setup()
{
  setupExternalPower();
  setupDisplay();
  setupLocationAndTime();
  display.fillScreen(ST7735_BLACK);
}

void loop()
{
  if (millisSinceStart >= 1000)
  {
    tm time = rtc.getTimeStruct();
    libsgp4::Eci eci = satellite.FindPosition(libsgp4::DateTime(libsgp4::UnixEpoch + mktime(&time) * libsgp4::TicksPerSecond));
    libsgp4::CoordTopocentric topo = observer.GetLookAngle(eci);

    display.setCursor(0, 0);
    display.println(eci.GetDateTime().ToString().c_str());
    display.println();
    display.println(observer.GetLocation().ToString().c_str());
    display.println();
    display.println(topo.ToString().c_str());

    millisSinceStart = 0;
  }
}