#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <TinyGPSPlus.h>
#include <ESP32Time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <elapsedMillis.h>
#include <OneButton.h>
#include <CoordTopocentric.h>
#include <Observer.h>
#include <SGP4.h>
#include <Satellite.h>
#include <Repeater.h>
#include <Transponder.h>
#include <FT818.h>
#include <map>

const int PIN_VEXT_CTRL = 3;
const int PIN_GNSS_TX = 33;
const int PIN_GNSS_RX = 34;
const int PIN_RX_D_1 = 7;
const int PIN_TX_D_1 = 6;
const int PIN_RX_D_2 = 5;
const int PIN_TX_D_2 = 4;
const int PIN_TFT_LED_K = 21;
const int PIN_TFT_CS = 38;
const int PIN_TFT_RES = 39;
const int PIN_TFT_RS = 40;
const int PIN_TFT_SCLK = 41;
const int PIN_TFT_SDIN = 42;
const int PIN_BUTTON = 0;
const int PIN_LED = 18;
const int SERIAL_BAUD_RATE_GNSS = 115200;
const int FIND_LOCATION_TIMEOUT_S = 60;
const int LOOP_TIMEOUT_MS = 1500;
const char SYMBOL_DEGREES = 0xF8;
const char SYMBOL_ARROW_UP = 0x1E;
const char SYMBOL_ARROW_DOWN = 0x1F;
const char SYMBOL_EQUALS = 0x3D;
const double SPEED_OF_LIGHT = 299792.458; // km/s
const String WIFI_FILE_PATH = "/wifi.txt";
const String TLE_FILE_PATH = "/tle.txt";
const String TLE_BACKUP_FILE_PATH = "/tle_backup.txt";
const String SATELLITE_FILE_PATH = "/satellites.txt";
const String TLE_URL = "http://www.amsat.org/tle/current/dailytle.txt";

HardwareSerial gnssSerial(0);
HardwareSerial uplinkRadioSerial(1);
HardwareSerial downlinkRadioSerial(2);

FT818 uplinkRadio(uplinkRadioSerial);
FT818 downlinkRadio(downlinkRadioSerial);

WiFiMulti wifiMulti;
Adafruit_ST7735 display(PIN_TFT_CS, PIN_TFT_RS, PIN_TFT_SDIN, PIN_TFT_SCLK, PIN_TFT_RES);
TinyGPSPlus gnss;
ESP32Time rtc;
OneButton button;
elapsedMillis millisSinceStart;
unsigned long lastTunedDownlinkFrequency;
long clarifierOffset;

libsgp4::Observer observer(0, 0, 0);
Satellite *satellite = nullptr;
Payload *payload = nullptr;
std::vector<Satellite *> satellites;

enum class State
{
  TRACKING,
  SET_CLARIFIER
};
State state = State::TRACKING;

void setupExternalPower()
{
  // Enable power to peripherals
  pinMode(PIN_VEXT_CTRL, OUTPUT);
  digitalWrite(PIN_VEXT_CTRL, HIGH);
}

void setupButton()
{
  pinMode(PIN_LED, OUTPUT);
  button.setup(PIN_BUTTON);
}

void setupDisplay()
{
  display.initR(INITR_MINI160x80_PLUGIN);
  display.setRotation(1); // Landscape mode
  display.cp437(true);
  display.setTextWrap(false);
  display.fillScreen(ST7735_BLACK);
  display.setTextColor(ST7735_WHITE, ST7735_BLACK);

  // Enable backlight
  pinMode(PIN_TFT_LED_K, OUTPUT);
  digitalWrite(PIN_TFT_LED_K, HIGH);
}

void setupFileSystem()
{
  if (!LittleFS.begin())
  {
    display.println("File system error");
    display.println();
    display.println("Error! Stopping...");
    while (true)
    {
      // Infinite loop
    }
  }

  if (!LittleFS.exists(WIFI_FILE_PATH))
  {
    display.println("WiFi file not found");
    LittleFS.end();
    display.println();
    display.println("Error! Stopping...");
    while (true)
    {
      // Infinite loop
    }
  }

  if (!LittleFS.exists(TLE_FILE_PATH))
  {
    display.println("TLE file not found");
    LittleFS.end();
    display.println();
    display.println("Error! Stopping...");
    while (true)
    {
      // Infinite loop
    }
  }
}

std::map<String, String> parseWifiCredentials(String &wifiCredentials)
{
  std::map<String, String> wifiNetworks;
  String ssid;
  String password;
  while (!wifiCredentials.isEmpty())
  {
    String line = wifiCredentials.substring(0, wifiCredentials.indexOf('\n'));
    String ssid = line.substring(0, line.indexOf(' '));
    String password = line.substring(line.indexOf(' ') + 1);
    wifiNetworks[ssid] = password;
    wifiCredentials.remove(0, line.length() + 1);
  }
  return wifiNetworks;
}

void setupWifi()
{
  if (File file = LittleFS.open(WIFI_FILE_PATH))
  {
    String wifiCredentials = file.readString();
    file.close();
    for (const auto &credentials : parseWifiCredentials(wifiCredentials))
    {
      wifiMulti.addAP(credentials.first.c_str(), credentials.second.c_str());
    }
  }
  else
  {
    display.println("WiFi setup error");
    LittleFS.end();
    display.println();
    display.println("Error! Stopping...");
    while (true)
    {
      // Infinite loop
    }
  }
}

void setupTle()
{
  if (wifiMulti.run() != WL_CONNECTED)
  {
    WiFi.disconnect(true, true);
    return;
  }

  display.fillScreen(ST7735_BLACK);
  display.setCursor(0, 0);
  display.println("Updating TLE...");

  HTTPClient http;
  http.begin(TLE_URL);

  if (http.GET() != HTTP_CODE_OK)
  {
    http.end();
    WiFi.disconnect(true, true);
    return;
  }

  if (!LittleFS.rename(TLE_FILE_PATH, TLE_BACKUP_FILE_PATH))
  {
    http.end();
    WiFi.disconnect(true, true);
    return;
  }

  bool tleUpdated = false;
  if (File file = LittleFS.open(TLE_FILE_PATH, FILE_WRITE))
  {
    tleUpdated = file.print(http.getString());
    file.close();
  }

  if (!tleUpdated && !LittleFS.rename(TLE_BACKUP_FILE_PATH, TLE_FILE_PATH))
  {
    http.end();
    WiFi.disconnect(true, true);
    display.println("TLE update and restore error!");
    display.println();
    display.println("Error! Stopping...");
    while (true)
    {
      // Infinite loop
    }
  }

  if (tleUpdated)
  {
    LittleFS.remove(TLE_BACKUP_FILE_PATH);
    display.println("TLE updated!");
    delay(2000);
  }

  http.end();
  WiFi.disconnect(true, true);
}

void setupSatellites()
{
  std::map<String, std::pair<String, Payload *>> nameAndPayloadBySatCat;
  if (File file = LittleFS.open(SATELLITE_FILE_PATH))
  {
    while (file.available() > 0)
    {
      String line = file.readStringUntil('\n');
      int start = 0;
      int end = line.indexOf(' ');
      std::vector<String> fields;
      while (end != -1)
      {
        fields.push_back(line.substring(start, end));
        start = end + 1;
        end = line.indexOf(' ', start);
      }
      fields.push_back(line.substring(start)); // Last field

      if (fields[0] == "R")
      {
        unsigned long uplinkFrequency = fields[1].toInt();
        unsigned long downlinkFrequency = fields[2].toInt();
        OperatingMode mode = StringToOperatingMode(fields[3].c_str());
        String catalogNumber = fields[4];
        String name = fields[5];
        nameAndPayloadBySatCat[catalogNumber] = std::make_pair(name, new Repeater(uplinkFrequency, downlinkFrequency, mode));
      }
      else if (fields[0] == "T")
      {
        unsigned long lowerUplinkFrequency = fields[1].toInt();
        unsigned long upperUplinkFrequency = fields[2].toInt();
        OperatingMode uplinkMode = StringToOperatingMode(fields[3].c_str());
        unsigned long lowerDownlinkFrequency = fields[4].toInt();
        unsigned long upperDownlinkFrequency = fields[5].toInt();
        OperatingMode downlinkMode = StringToOperatingMode(fields[6].c_str());
        bool inverting = fields[7] == "1" ? true : false;
        String catalogNumber = fields[8];
        String name = fields[9];
        nameAndPayloadBySatCat[catalogNumber] = std::make_pair(name, new Transponder({lowerUplinkFrequency, upperUplinkFrequency}, uplinkMode, {lowerDownlinkFrequency, upperDownlinkFrequency}, downlinkMode, inverting));
      }
    }
    file.close();
  }

  std::map<String, std::pair<String, String>> tleBySatCat;
  if (File file = LittleFS.open(TLE_FILE_PATH))
  {
    while (file.available() > 0)
    {
      String line = file.readStringUntil('\n');
      if (!line.startsWith("1 "))
      {
        continue;
      }
      String line2 = file.readStringUntil('\n');
      String catalogNumber = line.substring(2, 7);
      tleBySatCat[catalogNumber] = std::make_pair(line, line2);
    }
    file.close();
  }

  for (const auto &item : nameAndPayloadBySatCat)
  {
    const String &catalogNumber = item.first;
    const String &name = item.second.first;
    Payload *payload = item.second.second;
    if (tleBySatCat.find(catalogNumber) != tleBySatCat.end())
    {
      const String &tleLine1 = tleBySatCat[catalogNumber].first;
      const String &tleLine2 = tleBySatCat[catalogNumber].second;
      satellites.push_back(new Satellite(std::string(name.c_str()), std::string(tleLine1.c_str()), std::string(tleLine2.c_str()), payload));
    }
  }
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

struct SatelliteSelection
{
  int index;
  bool selected;
};

void nextSatellite(void *satelliteSelection)
{
  SatelliteSelection *selection = (SatelliteSelection *)satelliteSelection;
  selection->index = selection->index < satellites.size() - 1 ? selection->index + 1 : 0;
  Satellite *satellite = satellites.at(selection->index);
  Payload *payload = satellite->getPayload();
  display.fillScreen(ST7735_BLACK);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println(satellite->getName().c_str());
  display.println();
  display.printf("%s %s\n", formatFrequency(payload->getUplinkFrequency()), OperatingModeToString(payload->getUplinkMode()).c_str());
  display.printf("%s %s\n", formatFrequency(payload->getDownlinkFrequency()), OperatingModeToString(payload->getDownlinkMode()).c_str());
}

void selectSatellite(void *satelliteSelection)
{
  SatelliteSelection *selection = (SatelliteSelection *)satelliteSelection;
  selection->selected = true;
}

void setupSatelliteSelection()
{
  SatelliteSelection satelliteSelection = {0, false};
  button.attachClick(nextSatellite, &satelliteSelection);
  button.attachLongPressStart(selectSatellite, &satelliteSelection);
  nextSatellite(&satelliteSelection); // Show first satellite
  while (!satelliteSelection.selected)
  {
    button.tick();
  }
  satellite = satellites.at(satelliteSelection.index);
  payload = satellite->getPayload();
  button.reset();
  button.attachLongPressStart(NULL, NULL);
  button.attachClick(NULL, NULL);
  display.setTextSize(1);
}

void setupRadios()
{
  uplinkRadio.begin(PIN_TX_D_2, PIN_RX_D_2);
  downlinkRadio.begin(PIN_TX_D_1, PIN_RX_D_1);

  display.fillScreen(ST7735_BLACK);
  display.setCursor(0, 0);

  bool error = false;
  unsigned long frequency = 0;
  if (uplinkRadio.getFrequency(frequency))
  {
    display.println("Uplink: OK!");
    display.println(frequency);
  }
  else
  {
    display.println("Uplink: ERROR!");
    error = true;
  }

  display.println();
  if (downlinkRadio.getFrequency(frequency))
  {
    display.println("Downlink: OK!");
    display.println(frequency);
  }
  else
  {
    display.println("Downlink: ERROR!");
    error = true;
  }

  if (!error)
  {
    uplinkRadio.setOperatingMode(payload->getUplinkMode());
    uplinkRadio.setFrequency(payload->getUplinkFrequency());

    downlinkRadio.setOperatingMode(payload->getDownlinkMode());
    unsigned long initialDownlinkFrequency = payload->getDownlinkFrequency();
    downlinkRadio.setFrequency(initialDownlinkFrequency);
    unsigned long tunedDownlinkFrequency;
    downlinkRadio.getFrequency(tunedDownlinkFrequency);
    clarifierOffset = tunedDownlinkFrequency - initialDownlinkFrequency;
    lastTunedDownlinkFrequency = tunedDownlinkFrequency;

    delay(2000);
  }
  else
  {
    uplinkRadio.end();
    downlinkRadio.end();
    display.println();
    display.println("Error! Stopping...");
    while (true)
    {
      // Infinite loop
    }
  }
}

bool locationFound()
{
  return gnss.location.isValid() && gnss.hdop.isValid() && gnss.hdop.hdop() <= 5;
}

void setupLocationAndTime()
{
  gnssSerial.begin(SERIAL_BAUD_RATE_GNSS, SERIAL_8N1, PIN_GNSS_TX, PIN_GNSS_RX);

  display.fillScreen(ST7735_BLACK);
  display.setCursor(0, 0);
  display.print("Searching for location...");

  elapsedSeconds secondsSinceStart;
  do
  {
    while (gnssSerial.available())
    {
      gnss.encode(gnssSerial.read());
    }
  } while (!locationFound() && secondsSinceStart < FIND_LOCATION_TIMEOUT_S);

  int elapsedSeconds = secondsSinceStart;
  if (elapsedSeconds < FIND_LOCATION_TIMEOUT_S)
  {
    rtc.setTime(gnss.time.second(), gnss.time.minute(), gnss.time.hour(), gnss.date.day(), gnss.date.month(), gnss.date.year());
    observer = libsgp4::Observer(gnss.location.lat(), gnss.location.lng(), gnss.altitude.kilometers());
    gnssSerial.end();
    display.setCursor(0, ST7735_TFTWIDTH_80 / 2);
    display.println("Location found and time configured!");
    display.println();
    display.printf("Lat: %.6f %c, Lng: %.6f %c, Alt: %i m\n", gnss.location.lat(), SYMBOL_DEGREES, gnss.location.lng(), SYMBOL_DEGREES, gnss.altitude.meters());
    display.println();
    display.println(rtc.getDateTime());
    delay(2000);
  }
  else
  {
    gnssSerial.end();
    display.setCursor(0, ST7735_TFTWIDTH_80 / 2);
    display.print("Timeout! Stopping...");
    while (true)
    {
      // Infinite loop
    }
  }
}

void setupButtonEvent()
{
  button.reset();
  button.attachClick([]()
                     {
    switch (state)
    {
    case State::TRACKING:
      display.fillScreen(ST7735_BLACK);
      digitalWrite(PIN_LED, HIGH);
      state = State::SET_CLARIFIER;
      break;
    case State::SET_CLARIFIER:
      display.fillScreen(ST7735_BLACK);
      digitalWrite(PIN_LED, LOW);
      state = State::TRACKING;
      break;
    } });
  state = State::TRACKING;
}

long getDopplerShift(unsigned long sourceFrequency, double relativeSpeed)
{
  return sourceFrequency * relativeSpeed / SPEED_OF_LIGHT;
}

void setup()
{
  setupExternalPower();
  setupDisplay();
  setupFileSystem();
  setupWifi();
  setupTle();
  setupSatellites();
  setupButton();
  setupSatelliteSelection();
  setupRadios();
  setupLocationAndTime();
  setupButtonEvent();
  display.fillScreen(ST7735_BLACK);
}

void loop()
{
  button.tick();
  if (millisSinceStart >= LOOP_TIMEOUT_MS)
  {
    libsgp4::Eci satellitePosition = satellite->FindPosition(libsgp4::DateTime(libsgp4::UnixEpoch + rtc.getEpoch() * libsgp4::TicksPerSecond));
    libsgp4::CoordTopocentric lookAngle = observer.GetLookAngle(satellitePosition);

    if (lookAngle.elevation < 0)
    {
      display.setCursor(0, 0);
      display.println("Satellite below horizon!");
      millisSinceStart = 0;
      return;
    }

    unsigned long tunedDownlinkFrequency;
    if (!downlinkRadio.getFrequency(tunedDownlinkFrequency))
    {
      display.setCursor(0, 0);
      display.println("Downlink get freq error!");
      millisSinceStart = 0;
      return;
    }

    unsigned long tunedUplinkFrequency;
    if (!uplinkRadio.getFrequency(tunedUplinkFrequency))
    {
      display.setCursor(0, 0);
      display.println("Uplink get freq error!");
      millisSinceStart = 0;
      return;
    };

    unsigned long sourceDownlinkFrequency;
    unsigned long observedDownlinkFrequency;
    if (tunedDownlinkFrequency == lastTunedDownlinkFrequency)
    {
      sourceDownlinkFrequency = payload->getDownlinkFrequency();
      observedDownlinkFrequency = sourceDownlinkFrequency - getDopplerShift(sourceDownlinkFrequency, lookAngle.range_rate);
      if (observedDownlinkFrequency != tunedDownlinkFrequency - clarifierOffset)
      {
        if (!downlinkRadio.setFrequency(observedDownlinkFrequency))
        {
          display.setCursor(0, 0);
          display.println("Downlink set freq error!");
          millisSinceStart = 0;
          return;
        }
        lastTunedDownlinkFrequency = observedDownlinkFrequency + clarifierOffset;
      }
    }
    else
    {
      if (state == State::TRACKING)
      {
        observedDownlinkFrequency = tunedDownlinkFrequency - clarifierOffset;
        sourceDownlinkFrequency = observedDownlinkFrequency + getDopplerShift(observedDownlinkFrequency, lookAngle.range_rate);
        payload->setDownlinkFrequency(sourceDownlinkFrequency);
      }
      else if (state == State::SET_CLARIFIER)
      {
        clarifierOffset += tunedDownlinkFrequency - lastTunedDownlinkFrequency;
      }
      lastTunedDownlinkFrequency = tunedDownlinkFrequency;
    }

    unsigned long sourceUplinkFrequency = payload->getUplinkFrequency();
    unsigned long observedUplinkFrequency = sourceUplinkFrequency + getDopplerShift(sourceUplinkFrequency, lookAngle.range_rate);
    if (observedUplinkFrequency != tunedUplinkFrequency)
    {
      uplinkRadio.setFrequency(observedUplinkFrequency); // Ignore error (FT-818 does not support setting frequency while transmitting)
    }

    libsgp4::DateTime dateTime = satellitePosition.GetDateTime();
    char elevationChangeSymbol = lookAngle.range_rate == 0 ? SYMBOL_EQUALS : lookAngle.range_rate < 0 ? SYMBOL_ARROW_UP
                                                                                                      : SYMBOL_ARROW_DOWN;
    display.setCursor(0, 0);
    if (state == State::TRACKING)
    {
      display.printf("%s  %02i:%02i:%02i\n", satellite->getName().c_str(), dateTime.Hour(), dateTime.Minute(), dateTime.Second());
      display.println();
      display.printf("Az: %05.1f  El: %04.1f %c\n", libsgp4::Util::RadiansToDegrees(lookAngle.azimuth), libsgp4::Util::RadiansToDegrees(lookAngle.elevation), elevationChangeSymbol);
      display.println();
      display.printf("%s\n", formatFrequency(sourceUplinkFrequency));
      display.printf("%s\n", formatFrequency(sourceDownlinkFrequency));
      display.println();
      display.printf("%s\n", formatFrequency(observedUplinkFrequency));
      display.printf("%s\n", formatFrequency(observedDownlinkFrequency));
    }
    else if (state == State::SET_CLARIFIER)
    {
      display.printf("Clarifier: %03li\n", clarifierOffset);
    }

    millisSinceStart = 0;
  }
}