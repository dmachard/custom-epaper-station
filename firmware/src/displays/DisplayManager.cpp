#include "DisplayManager.h"

DisplayManager::DisplayManager(
    GxEPD2_3C<GxEPD2_420c_GDEY042Z98, GxEPD2_420c_GDEY042Z98::HEIGHT>
        &displayColor,
    GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> &displayBW1,
    GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> &displayBW2,
    GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> &displayBW3,
    U8G2_FOR_ADAFRUIT_GFX &u8g2)
    : _u8g2(u8g2), _ephemerisDisplay(displayColor, u8g2), // Index 0 (Color)
      _sensorDisplay(displayBW1, u8g2),                   // Index 1 (BW)
      _eventsDisplay(displayBW2, u8g2),                   // Index 2 (BW)
      _sensorDisplay2(displayBW3, u8g2) {}                // Index 3 (BW)

BaseDisplay *DisplayManager::getDisplay(int index) {
  switch (index) {
  case 0:
    return &_ephemerisDisplay; // Screen 0: Ephemeris (TL)
  case 1:
    return &_sensorDisplay; // Screen 1: Sensors 1 (TR)
  case 2:
    return &_eventsDisplay; // Screen 2: Events (BL)
  case 3:
    return &_sensorDisplay2; // Screen 3: Sensors 2 (BR)
  default:
    return nullptr;
  }
}

void DisplayManager::init() {
  Serial.println("[DisplayManager] Initializing all displays...");

  _u8g2.begin(_sensorDisplay.getDisplay()); // Init common font engine
  _ephemerisDisplay.init();
  Serial.println("  - EphemerisDisplay initialized");
  _sensorDisplay.init();
  Serial.println("  - SensorDisplay 1 initialized");
  _eventsDisplay.init();
  Serial.println("  - EventsDisplay initialized");
  _sensorDisplay2.init();
  Serial.println("  - SensorDisplay 2 initialized");

  Serial.println("[DisplayManager] All displays initialized");
}
