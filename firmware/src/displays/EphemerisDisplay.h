#ifndef EPHEMERIS_DISPLAY_H
#define EPHEMERIS_DISPLAY_H

#include "BaseDisplay.h"
#include <Arduino.h>
#include <GxEPD2_3C.h>
#include <U8g2_for_Adafruit_GFX.h>

// Screen 2: Calendar + Ephemeris
// Affiche la date, les saisons et les horaires de lever/coucher du soleil
class EphemerisDisplay : public BaseDisplay {
public:
  EphemerisDisplay(GxEPD2_3C<GxEPD2_420c_GDEY042Z98,
                             GxEPD2_420c_GDEY042Z98::HEIGHT> &display,
                   U8G2_FOR_ADAFRUIT_GFX &u8g2);

  void init() override;
  void update(bool fullRefresh = false) override;
  void clear() override;
  void drawError(const char *message);

  // Data for display
  struct DateData {
    const char *dayName;
    const char *dayNumber;
    const char *monthName;
    const char *year;
    int dayOfYear;
    int totalDays;
    int weekNumber;
  };

  struct SunData {
    String sunrise;
    String sunset;
    String dailyChange;
  };

  struct SeasonData {
    String currentSeason; // "spring", "summer", "fall", "winter"
    float seasonProgress; // 0-100%
    int daysUntilSpring;
    int daysUntilSummer;
    int daysUntilFall;
    int daysUntilWinter;
  };

  void setData(const DateData &date, const SunData &sun,
               const SeasonData &season);

private:
  GxEPD2_3C<GxEPD2_420c_GDEY042Z98, GxEPD2_420c_GDEY042Z98::HEIGHT> &_display;
  U8G2_FOR_ADAFRUIT_GFX &_u8g2;

  DateData _date;
  SunData _sun;
  SeasonData _season;

  void drawLayout();
  void drawDate(const DateData &date);
  void drawSunInfo(const SunData &sun);
  void drawSeason(const SeasonData &season);
  void drawSeasonCircle(int x, int y, int r, const char *label, bool isCurrent,
                        int value, bool isPercent);
};

#endif
