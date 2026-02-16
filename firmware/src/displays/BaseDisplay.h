#ifndef BASE_DISPLAY_H
#define BASE_DISPLAY_H

#include <Arduino.h>

// Interface de base pour tous les displays
class BaseDisplay {
public:
  virtual ~BaseDisplay() {}
  virtual void init() = 0;
  virtual void update(bool fullRefresh = false) = 0;
  virtual void clear() = 0;
};

#endif
