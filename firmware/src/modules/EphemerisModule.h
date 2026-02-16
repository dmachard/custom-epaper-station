#ifndef EPHEMERIS_MODULE_H
#define EPHEMERIS_MODULE_H

#include "../displays/EphemerisDisplay.h"
#include "BaseModule.h"

// Responsibilities: Date, Sun, Seasons
// Screen: 2 (Color)
class EphemerisModule : public BaseModule {
public:
  EphemerisModule();
  String getName() override { return "Ephemeris"; }
  int getRequiredScreenCount() override { return 1; }
  ScreenType getRequiredScreenType(int index) override {
    return SCREEN_TYPE_COLOR;
  }
  void assignScreen(int index, BaseDisplay *display) override;
  void begin();
  void update();
  void forceUpdate() override;

private:
  EphemerisDisplay *_display;
  int _lastFullRefreshDay = -1;
};

#endif
