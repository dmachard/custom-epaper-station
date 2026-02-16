#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <GxEPD2_3C.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>

#include "../pin.h"
#include "EphemerisDisplay.h"
#include "EventsDisplay.h"
#include "SensorDisplay.h"

// Screen 0: Color (Ephemeris)
// Screen 1: BW (Sensors)
// Screen 2: BW
// Screen 3: BW
class DisplayManager {
public:
  DisplayManager(GxEPD2_3C<GxEPD2_420c_GDEY042Z98,
                           GxEPD2_420c_GDEY042Z98::HEIGHT> &displayColor,
                 GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT>
                     &displayBW1,
                 GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT>
                     &displayBW2,
                 GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT>
                     &displayBW3,
                 U8G2_FOR_ADAFRUIT_GFX &u8g2);

  void init();
  BaseDisplay *getDisplay(int index);

  // Specific Accessors (Optional)
  EphemerisDisplay &getEphemerisDisplay() { return _ephemerisDisplay; }
  SensorDisplay &getSensorDisplay() { return _sensorDisplay; }   // Index 1
  SensorDisplay &getSensorDisplay2() { return _sensorDisplay2; } // Index 3

private:
  U8G2_FOR_ADAFRUIT_GFX &_u8g2;

  EphemerisDisplay _ephemerisDisplay;
  SensorDisplay _sensorDisplay;  // Index 1 (TR)
  EventsDisplay _eventsDisplay;  // Index 2 (BL)
  SensorDisplay _sensorDisplay2; // Index 3 (BR)
};

#endif
