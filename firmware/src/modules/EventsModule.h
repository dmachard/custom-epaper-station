#ifndef EVENTS_MODULE_H
#define EVENTS_MODULE_H

#include "../displays/EventsDisplay.h"
#include "BaseModule.h"

// Responsibilities: Trash, Birthdays
// Screen: 3 (BW)
class EventsModule : public BaseModule {
public:
  EventsModule();

  String getName() override { return "Events"; }

  int getRequiredScreenCount() override { return 1; }
  ScreenType getRequiredScreenType(int index) override {
    return SCREEN_TYPE_BW;
  }

  void assignScreen(int index, BaseDisplay *display) override;

  void begin() override;
  void update() override;
  void forceUpdate() override;

private:
  EventsDisplay *_display = nullptr;
  int _lastFullRefreshDay = -1;
};

#endif
