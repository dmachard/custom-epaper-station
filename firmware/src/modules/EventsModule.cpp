#include "EventsModule.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp-iot-utils.h>
#include <vector>

EventsModule::EventsModule() {
  _updateInterval = 3600000; // 1 hour
}

void EventsModule::assignScreen(int index, BaseDisplay *display) {
  if (index == 0) {
    _display = static_cast<EventsDisplay *>(display);
  }
}

void EventsModule::begin() {
  if (_display)
    _display->init();
  update();
}

void EventsModule::update() {
  if (!_display)
    return;

  struct tm timeinfo;
  if (!TimeHelper::getLocalTime(&timeinfo)) {
    Serial.println("[EventsModule] Failed to get local time");
    return;
  }

  bool isFirstBoot = (_lastFullRefreshDay == -1);
  bool isScheduledTime = (timeinfo.tm_hour == 0 && timeinfo.tm_min == 1);
  bool isNewDay = (_lastFullRefreshDay != timeinfo.tm_mday);

  if (isFirstBoot || (isScheduledTime && isNewDay)) {
    Serial.println("[EventsModule] Performing daily update...");

    String url = _config->get("tempus_url", String(""));
    String finalUrl = UrlHelper::replaceDatePlaceholders(url);
    JsonDocument doc;

    if (url.isEmpty() || !HttpClient::fetchJson(finalUrl, doc)) {
      Serial.println("[EventsModule] Failed to fetch Trash/Birthday data");
      _display->drawError("Fetch Failed");
      return;
    }

    EventsDisplay::TrashData trashData;
    trashData.blackToday = doc["trash"]["black"]["today"].as<bool>();
    trashData.blackDays = doc["trash"]["black"]["next_in_days"].as<int>();
    trashData.yellowToday = doc["trash"]["yellow"]["today"].as<bool>();
    trashData.yellowDays = doc["trash"]["yellow"]["next_in_days"].as<int>();

    std::vector<EventsDisplay::Birthday> birthdays;
    JsonArray bdayArray = doc["birthdays"]["this_month"].as<JsonArray>();
    for (JsonObject b : bdayArray) {
      EventsDisplay::Birthday bd;
      bd.name = b["name"].as<String>();
      bd.day = b["day"].as<int>();
      bd.days_until = b["days_until"].as<int>();
      bd.is_today = b["is_today"].as<bool>();
      birthdays.push_back(bd);
    }

    _display->setData(trashData, birthdays, timeinfo.tm_mday);
    _display->update(true); // Full refresh to avoid ghosting

    _lastFullRefreshDay = timeinfo.tm_mday;
    _lastUpdate = millis();
    Serial.println("[EventsModule] Daily update completed");
  }
}

void EventsModule::forceUpdate() {
  Serial.println("[EventsModule] Force update called!");
  _lastUpdate = 0;
}
