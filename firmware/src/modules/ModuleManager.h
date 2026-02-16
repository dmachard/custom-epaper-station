#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

#include "../displays/DisplayManager.h"
#include "modules/BaseModule.h"
#include <esp-iot-utils.h>
#include <vector>

class ModuleManager {
public:
  ModuleManager(DisplayManager &displayManager, ConfigHelper &config);

  void registerModule(BaseModule *module);
  void begin();
  void update();
  void forceUpdate();

private:
  DisplayManager &_displayManager;
  ConfigHelper &_config;
  std::vector<BaseModule *> _modules;

  // Track assigned screens to avoid double assignment
  // 4 screens total as per config.h
  bool _screenAssigned[4] = {false, false, false, false};

  BaseDisplay *findFreeScreen(ScreenType type);
};

#endif
