#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <ESP32Time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <elapsedMillis.h>
#include <CoordTopocentric.h>
#include <Observer.h>
#include <SGP4.h>
#include <Satellites.h>

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
const char SYMBOL_DEGREES = 0xF8;
const char SYMBOL_ARROW_UP = 0x1E;
const char SYMBOL_ARROW_DOWN = 0x1F;
const char SYMBOL_EQUALS = 0x3D;
const double SPEED_OF_LIGHT = 299792.458; // km/s

HardwareSerial serialGnss(0);
Adafruit_ST7735 display(PIN_TFT_CS, PIN_TFT_RS, PIN_TFT_SDIN, PIN_TFT_SCLK, PIN_TFT_RES);
TinyGPSPlus gnss;
ESP32Time rtc;
elapsedMillis millisSinceStart;

libsgp4::Observer observer(0, 0, 0);
Satellite satellite = iss;
Payload *payload = satellite.getPayload();

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
    display.println("Location found and time configured!");
    display.println();
    display.printf("Lat: %.6f %c, Lng: %.6f %c, Alt: %i m\n", gnss.location.lat(), SYMBOL_DEGREES, gnss.location.lng(), SYMBOL_DEGREES, gnss.altitude.meters());
    display.println();
    display.println(rtc.getDateTime());
    delay(2000);
  }
}

unsigned long getDopplerShift(unsigned long sourceFrequency, double relativeSpeed)
{
  return sourceFrequency * relativeSpeed / SPEED_OF_LIGHT;
}

String formatFrequency(unsigned long frequency)
{
  unsigned int MHz = frequency / 100000;
  unsigned int KHz = (frequency % 100000) / 100;
  unsigned int daHz = frequency % 100;
  char buffer[11];
  snprintf(buffer, sizeof(buffer), "%03u.%03u.%02u", MHz, KHz, daHz);
  return String(buffer);
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
    libsgp4::Eci satellitePosition = satellite.FindPosition(libsgp4::DateTime(libsgp4::UnixEpoch + rtc.getEpoch() * libsgp4::TicksPerSecond));
    libsgp4::CoordTopocentric lookAngle = observer.GetLookAngle(satellitePosition);
    libsgp4::DateTime dateTime = satellitePosition.GetDateTime();
    char elevationChangeSymbol = lookAngle.range_rate == 0 ? SYMBOL_EQUALS : lookAngle.range_rate < 0 ? SYMBOL_ARROW_UP
                                                                                                      : SYMBOL_ARROW_DOWN;
    display.setCursor(0, 0);
    display.printf("%s  %02i:%02i:%02i\n", satellite.getName().c_str(), dateTime.Hour(), dateTime.Minute(), dateTime.Second());
    display.println();
    display.printf("Az: %05.1f  El: %04.1f %c\n", libsgp4::Util::RadiansToDegrees(lookAngle.azimuth), libsgp4::Util::RadiansToDegrees(lookAngle.elevation), elevationChangeSymbol);
    display.println();
    display.printf("%s\n", formatFrequency(payload->getUplinkFrequency()).c_str());
    display.printf("%s\n", formatFrequency(payload->getDownlinkFrequency()).c_str());
    display.println();
    display.printf("%s\n", formatFrequency(payload->getUplinkFrequency() + getDopplerShift(payload->getUplinkFrequency(), lookAngle.range_rate)).c_str());
    display.printf("%s\n", formatFrequency(payload->getDownlinkFrequency() - getDopplerShift(payload->getDownlinkFrequency(), lookAngle.range_rate)).c_str());

    millisSinceStart = 0;
  }
}