#ifndef SENSOR_MODULE_H
#define SENSOR_MODULE_H

#include "../displays/SensorDisplay.h"
#include "BaseModule.h"
#include <ArduinoJson.h>

struct SensorConfig {
  String label;  // Label displayed on screen (e.g., "Conso. EDF")
  String url;    // Full URL (Prometheus query or JSON API)
  String unit;   // Unit of measurement (e.g., "kWh", "°C", "W")
  float divisor; // Conversion divisor (e.g., 1000 for Wh → kWh)
  int decimals;  // Number of decimals to display (0-2)
  bool enabled;  // Whether this sensor is active
  String type;   // "prometheus" or "json"
  String
      jsonPath; // Path based on simple dot/check notation (e.g. data.values[0])

  SensorConfig()
      : label(""), url(""), unit(""), divisor(1.0), decimals(1), enabled(false),
        type("prometheus"), jsonPath("") {}
};

class SensorConfigHelper {
public:
  static void fromJson(JsonVariantConst src, SensorConfig &dest) {
    dest.label = src["label"] | "";
    dest.url = src["url"] | "";
    dest.unit = src["unit"] | "";
    dest.divisor = src["divisor"] | 1.0f;
    dest.decimals = src["decimals"] | 1;
    dest.enabled = src["enabled"] | false;
    dest.type = src["type"] | "prometheus";
    dest.jsonPath = src["jsonPath"] | "";
  }

  static void toJson(const SensorConfig &src, JsonVariant dest) {
    dest["label"] = src.label;
    dest["url"] = src.url;
    dest["unit"] = src.unit;
    dest["divisor"] = src.divisor;
    dest["decimals"] = src.decimals;
    dest["enabled"] = src.enabled;
    dest["type"] = src.type;
    dest["jsonPath"] = src.jsonPath;
  }
};

class SensorModule : public BaseModule {
public:
  SensorModule(String name, int startSlot);

  String getName() override { return _moduleName; }

  int getRequiredScreenCount() override { return 1; }
  ScreenType getRequiredScreenType(int index) override {
    return SCREEN_TYPE_BW;
  }

  void assignScreen(int index, BaseDisplay *display) override;

  void begin() override;
  void update() override;
  void forceUpdate() override {
    _lastUpdate = 0;
    _lastFullRefreshDay = -1;
  }

  void setRefreshInterval(unsigned long interval) {
    _updateInterval = interval;
  }

private:
  SensorDisplay *_display = nullptr;
  String _moduleName;
  int _startSlot;

  float _values[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  bool _hasData[8] = {false, false, false, false, false, false, false, false};
  String _labels[8] = {"", "", "", "", "", "", "", ""};
  String _units[8] = {"", "", "", "", "", "", "", ""};
  int _decimals[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  String _lastUpdateTimes[8] = {"", "", "", "", "", "", "", ""};

  int _lastFullRefreshDay = -1;
  unsigned long _lastUpdate = 0;
  unsigned long _updateInterval = 60000; // Default 1 minute
};

#endif
