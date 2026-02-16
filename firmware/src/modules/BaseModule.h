#ifndef BASE_MODULE_H
#define BASE_MODULE_H

#include "displays/BaseDisplay.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp-iot-utils.h>

// Available screen types
enum ScreenType { SCREEN_TYPE_BW, SCREEN_TYPE_COLOR };

// Base interface for all modules
class BaseModule {
public:
  virtual ~BaseModule() {}

  // Metadata
  virtual String getName() = 0;

  // Requirements
  virtual int getRequiredScreenCount() { return 0; }
  virtual ScreenType getRequiredScreenType(int index) { return SCREEN_TYPE_BW; }

  // Resource Injection
  virtual void assignScreen(int index, BaseDisplay *display) {}

  // ConfigHelper Injection (Inversion of Control)
  void setConfig(ConfigHelper *config) { _config = config; }

  // Lifecycle
  virtual void begin() {}
  virtual void update() {}
  virtual void forceUpdate() { _lastUpdate = 0; }

protected:
  ConfigHelper *_config = nullptr;
  unsigned long _lastUpdate = 0;
  unsigned long _updateInterval = 60000; // Default 1 min
};

#endif
