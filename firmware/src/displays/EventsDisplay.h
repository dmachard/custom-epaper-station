#ifndef EVENTS_DISPLAY_H
#define EVENTS_DISPLAY_H

#include "BaseDisplay.h"
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <vector>

// Screen 3: Events (Trash + Birthdays)
class EventsDisplay : public BaseDisplay {
public:
  EventsDisplay(
      GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> &display,
      U8G2_FOR_ADAFRUIT_GFX &u8g2);

  void init() override;
  void update(bool fullRefresh = false) override;
  void clear() override;
  void drawError(const char *message);

  struct TrashData {
    bool blackToday;
    int blackDays;
    bool yellowToday;
    int yellowDays;
  };

  struct Birthday {
    String name;
    int day;
    int days_until;
    bool is_today;
  };

  void setData(const TrashData &trash, const std::vector<Birthday> &birthdays,
               int currentDay);

private:
  GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> &_display;
  U8G2_FOR_ADAFRUIT_GFX &_u8g2;

  TrashData _trash;
  std::vector<Birthday> _birthdays;
  int _currentDay;

  void drawBin(int x, int y, bool isBlack, bool isToday, int days);
};

#endif
