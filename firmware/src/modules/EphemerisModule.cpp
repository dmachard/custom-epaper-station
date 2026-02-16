#include "EphemerisModule.h"
#include <ArduinoJson.h>
#include <esp-iot-utils.h>

EphemerisModule::EphemerisModule() {
  _updateInterval = 3600000; // 1 hour
}

void EphemerisModule::assignScreen(int index, BaseDisplay *display) {
  if (index == 0) {
    _display = static_cast<EphemerisDisplay *>(display);
  }
}

void EphemerisModule::begin() {
  if (_display)
    _display->init();
  update();
}

void EphemerisModule::update() {
  if (!_display)
    return;

  struct tm timeinfo;
  if (!TimeHelper::getLocalTime(&timeinfo)) {
    Serial.println("[EphemerisModule] Failed to get local time");
    return;
  }

  // Logic:
  // 1. Always update on first boot (_lastFullRefreshDay == -1)
  // 2. Otherwise, only update once a day at 00:01
  bool isFirstBoot = (_lastFullRefreshDay == -1);
  bool isScheduledTime = (timeinfo.tm_hour == 0 && timeinfo.tm_min == 1);
  bool isNewDay = (_lastFullRefreshDay != timeinfo.tm_mday);

  if (isFirstBoot || (isScheduledTime && isNewDay)) {
    Serial.println("[EphemerisModule] Performing daily update...");

    String url = _config->get("tempus_url", String(""));
    String finalUrl = UrlHelper::replaceDatePlaceholders(url);
    JsonDocument doc;

    if (url.isEmpty() || !HttpClient::fetchJson(finalUrl, doc)) {
      Serial.println("[EphemerisModule] Failed to fetch Sun/Season data");
      _display->drawError("Fetch Failed");
      return;
    }

    // Local buffers for C-string storage
    static char jourNom[12];
    static char jourChiffre[3];
    static char moisNom[12];
    static char annee[5];

    strcpy(jourNom, TimeHelper::getDayName(timeinfo.tm_wday));
    snprintf(jourChiffre, sizeof(jourChiffre), "%d", timeinfo.tm_mday);
    strcpy(moisNom, TimeHelper::getMonthName(timeinfo.tm_mon));
    snprintf(annee, sizeof(annee), "%d", 1900 + timeinfo.tm_year);

    int jourAnnee = timeinfo.tm_yday + 1;
    int jourTotal = ((timeinfo.tm_year + 1900) % 4 == 0) ? 366 : 365;
    int semaine = (jourAnnee / 7) + 1;

    // Use static pointers to these buffers for the display struct
    EphemerisDisplay::DateData dateData = {
        jourNom, jourChiffre, moisNom, annee, jourAnnee, jourTotal, semaine};

    EphemerisDisplay::SunData sunData;
    sunData.sunrise = doc["sun"]["sunrise"].as<String>();
    sunData.sunset = doc["sun"]["sunset"].as<String>();
    sunData.dailyChange = doc["sun"]["daily_change"].as<String>();

    EphemerisDisplay::SeasonData seasonData;
    seasonData.currentSeason = doc["season"]["name"].as<String>();
    seasonData.seasonProgress = doc["season"]["progress"].as<float>();
    seasonData.daysUntilSpring = doc["season"]["days_until_spring"].as<int>();
    seasonData.daysUntilSummer = doc["season"]["days_until_summer"].as<int>();
    seasonData.daysUntilFall = doc["season"]["days_until_fall"].as<int>();
    seasonData.daysUntilWinter = doc["season"]["days_until_winter"].as<int>();

    _display->setData(dateData, sunData, seasonData);
    _display->update(true); // Always full refresh for Ephemeris (Color)

    _lastFullRefreshDay = timeinfo.tm_mday;
    _lastUpdate = millis();
    Serial.println("[EphemerisModule] Daily update completed");
  }
}

void EphemerisModule::forceUpdate() {
  Serial.println("[EphemerisModule] Force update called!");
  Serial.print("[EphemerisModule] Current Language: ");
  Serial.println(TimeHelper::getLanguage());
  _lastUpdate = 0;
}
