#include "SensorModule.h"
#include <esp-iot-utils.h>

SensorModule::SensorModule(String name, int startSlot)
    : _moduleName(name), _startSlot(startSlot) {}

void SensorModule::assignScreen(int index, BaseDisplay *display) {
  if (index == 0) {
    _display = static_cast<SensorDisplay *>(display);
  }
}

void SensorModule::begin() {
  if (_display) {
    _display->init();
  }
}

void SensorModule::update() {
  if (_lastUpdate != 0 && millis() - _lastUpdate < _updateInterval)
    return;

  Serial.println("[SensorModule] Updating sensors...");
  _lastUpdate = millis();

  for (int slot = 0; slot < 8; slot++) {
    String key = "sensor_" + String(_startSlot + slot);
    SensorConfig config;
    JsonDocument sDoc;
    if (_config->get(key.c_str(), sDoc)) {
      SensorConfigHelper::fromJson(sDoc.as<JsonVariantConst>(), config);
    }

    if (config.enabled && !config.url.isEmpty()) {
      Serial.println("[SensorModule] Updating slot " +
                     String(_startSlot + slot) + ": " + config.label);

      float value = 0;
      bool success = false;
      String finalUrl = UrlHelper::replaceDatePlaceholders(config.url);

      if (config.type == "prometheus") {
        PrometheusResult res;
        if (PrometheusFetcher::fetch(finalUrl, config.divisor, config.decimals,
                                     res)) {
          value = res.value.toFloat();
          success = res.valid;
        }
      } else {
        JsonFetcherResult res;
        if (JsonFetcher::fetch(finalUrl, config.jsonPath, config.divisor,
                               config.decimals, res)) {
          value = res.value.toFloat();
          success = res.success;
        }
      }

      if (success) {
        _values[slot] = value;
        _labels[slot] = config.label;
        _units[slot] = config.unit;
        _decimals[slot] = config.decimals;
        _hasData[slot] = true;

        struct tm timeinfo;
        if (TimeHelper::getLocalTime(&timeinfo)) {
          char timeStr[6];
          snprintf(timeStr, sizeof(timeStr), "%02d:%02d", timeinfo.tm_hour,
                   timeinfo.tm_min);
          _lastUpdateTimes[slot] = String(timeStr);
          if (_display) {
            _display->setLastUpdate(String(timeStr));

            // Refresh daily at 3 AM
            if (timeinfo.tm_hour == 3 &&
                _lastFullRefreshDay != timeinfo.tm_mday) {
              _display->update(true); // Full refresh
              _lastFullRefreshDay = timeinfo.tm_mday;
              Serial.println("[SensorModule] Daily full refresh triggered");
            } else {
              _display->update(false); // Partial refresh
            }
          }
        }
      }
    } else {
      // Clear data for disabled/empty slots
      _values[slot] = 0;
      _labels[slot] = "";
      _units[slot] = "";
      _decimals[slot] = 0;
      _hasData[slot] = false;
    }
  }

  // Final display update with ALL data (8 sensors)
  if (_display) {
    auto getValueStr = [&](int slot) {
      return _hasData[slot] ? String(_values[slot], _decimals[slot]) : "--";
    };

    _display->setData({_labels[0], getValueStr(0), _units[0]},
                      {_labels[1], getValueStr(1), _units[1]},
                      {_labels[2], getValueStr(2), _units[2]},
                      {_labels[3], getValueStr(3), _units[3]},
                      {_labels[4], getValueStr(4), _units[4]},
                      {_labels[5], getValueStr(5), _units[5]},
                      {_labels[6], getValueStr(6), _units[6]},
                      {_labels[7], getValueStr(7), _units[7]});

    _display->setStyle(_config->get("sens_style", 0));
    _display->update(false);
  }
}
