#ifndef BLE_H
#define BLE_H

#include <NimBLE-DataPipe.h>
#include <esp-iot-utils.h>

// UUIDs for Service and Characteristics
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_JSON_GATE_UUID "a1b2c3d4-1234-5678-9abc-def012345685"

class Ble {
public:
  Ble(ConfigHelper &config);
  void begin();
  void stop();
  bool isConnected();

private:
  ConfigHelper &_config;
  NimBLE_DataPipe _pipe;

  void handleCommand(const JsonDocument &doc);
};

// Flag to trigger immediate sensor update from main loop
extern volatile bool shouldUpdateSensors;

#endif
