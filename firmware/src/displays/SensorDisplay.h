#ifndef SENSOR_DISPLAY_H
#define SENSOR_DISPLAY_H

#include "BaseDisplay.h"
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>

// Screen 1: Sensors + EDF Consumption
class SensorDisplay : public BaseDisplay {
public:
  SensorDisplay(
      GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> &display,
      U8G2_FOR_ADAFRUIT_GFX &u8g2);

  void init() override;
  void update(bool fullRefresh = false) override;
  void clear() override;

  // Structure for sensor data
  struct SensorData {
    String label;
    String value;
    String unit;
  };

  void setData(SensorData row1_col1, SensorData row1_col2, SensorData row2_col1,
               SensorData row2_col2, SensorData row3_col1, SensorData row3_col2,
               SensorData row4_col1, SensorData row4_col2);

  GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> &
  getDisplay() {
    return _display;
  }

  void setStyle(int style) { _style = style; }
  void setLastUpdate(const String &time) { _lastUpdateTime = time; }

private:
  GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> &_display;
  U8G2_FOR_ADAFRUIT_GFX &_u8g2;

  SensorData _data[8];
  int _style = 0;
  String _lastUpdateTime = "";

  void drawCell(int col, int row, String label, String val, String unit,
                bool inverted);
  void drawCircles();
};

#endif
