#include "ModuleManager.h"

ModuleManager::ModuleManager(DisplayManager &displayManager,
                             ConfigHelper &config)
    : _displayManager(displayManager), _config(config) {}

void ModuleManager::registerModule(BaseModule *module) {
  // Inject Config dependency
  module->setConfig(&_config);
  _modules.push_back(module);
}

void ModuleManager::begin() {
  Serial.println("[ModuleManager] Starting...");

  // Load Mapping from Config
  String mappingJson = _config.get("module_map", String(""));
  Serial.println("[ModuleManager] Loaded Mapping: " + mappingJson);

  std::vector<String> mapping;
  if (mappingJson.length() > 2) {
    JsonDocument doc;
    if (!deserializeJson(doc, mappingJson)) {
      JsonArray arr = doc.as<JsonArray>();
      for (JsonVariant v : arr) {
        mapping.push_back(v.as<String>());
      }
    }
  }

  // 1. Assign Screens
  // First, verify if we have a valid mapping for all 4 screens
  bool useMapping = (mapping.size() == 4);

  if (useMapping) {
    Serial.println("[ModuleManager] Using Custom Mapping");

    // Track which module instances are already assigned
    std::vector<BaseModule *> assignedModules;

    // Iterate over screens 0 to 3
    for (int screenIdx = 0; screenIdx < 4; screenIdx++) {
      String moduleName = mapping[screenIdx];
      BaseDisplay *display = _displayManager.getDisplay(screenIdx);

      if (moduleName == "None" || moduleName == "Empty") {
        continue;
      }

      // Find an available module with this name
      BaseModule *targetModule = nullptr;
      for (auto *m : _modules) {
        if (m->getName() == moduleName) {
          // Check if THIS instance is already assigned
          bool alreadyAssigned = false;
          for (auto *am : assignedModules) {
            if (am == m) {
              alreadyAssigned = true;
              break;
            }
          }
          if (!alreadyAssigned) {
            targetModule = m;
            break;
          }
        }
      }

      if (targetModule) {
        targetModule->assignScreen(0, display);
        assignedModules.push_back(targetModule);
        _screenAssigned[screenIdx] = true;
        Serial.printf("[ModuleManager] Assigned %s to Screen %d\n",
                      moduleName.c_str(), screenIdx);

      } else {
        Serial.printf("[ModuleManager] No available module %s for Screen %d\n",
                      moduleName.c_str(), screenIdx);
      }
    }

  } else {
    // FALBACK TO DEFAULT LOGIC (First Fit)
    Serial.println("[ModuleManager] Using Default Logic");

    for (auto *module : _modules) {
      Serial.printf("[ModuleManager] Configuring module: %s\n",
                    module->getName().c_str());

      int required = module->getRequiredScreenCount();
      for (int i = 0; i < required; i++) {
        ScreenType type = module->getRequiredScreenType(i);
        BaseDisplay *screen = findFreeScreen(type);

        if (screen) {
          Serial.printf("[ModuleManager] Assigned screen to %s\n",
                        module->getName().c_str());
          module->assignScreen(i, screen);

          // Find which index this screen corresponds to
          for (int j = 0; j < 4; j++) {
            if (_displayManager.getDisplay(j) == screen) {
              break;
            }
          }

        } else {
          Serial.printf("[ModuleManager] ERROR: No free screen of type %d for "
                        "module %s\n",
                        type, module->getName().c_str());
        }
      }
    }
  }

  // 2. Begin Module
  for (auto *module : _modules) {
    module->begin();
  }
}

void ModuleManager::update() {
  for (auto *module : _modules) {
    module->update();
  }
}

void ModuleManager::forceUpdate() {
  Serial.println("[ModuleManager] Force Update Triggered");
  for (auto *module : _modules) {
    Serial.printf("[ModuleManager] Forcing update for %s\n",
                  module->getName().c_str());
    module->forceUpdate();
  }
}

BaseDisplay *ModuleManager::findFreeScreen(ScreenType type) {
  // Current mapping (Hardcoded in DisplayManager / hardware setup)
  // Index 0: Color (CalendarDisplay)
  // Index 1: BW (SensorDisplay)
  // Index 2: BW (EventsDisplay)
  // Index 3: BW (QRCodeDisplay)

  // Strategy: First Fit
  for (int i = 0; i < 4; i++) {
    if (_screenAssigned[i])
      continue;

    // Check compatibility
    bool isCompatible = false;
    if (type == SCREEN_TYPE_COLOR &&
        i == 0) { // Screen 0 is Color (Previously 1)
      isCompatible = true;
    } else if (type == SCREEN_TYPE_BW && i != 0) { // Screens 1, 2, 3 are BW
      isCompatible = true;
    }

    if (isCompatible) {
      _screenAssigned[i] = true;
      return _displayManager.getDisplay(i);
    }
  }
  return nullptr;
}
