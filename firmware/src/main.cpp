#define ENABLE_GxEPD2_GFX 0

#include <Arduino.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_BW.h>
#include <SPI.h>
#include <U8g2_for_Adafruit_GFX.h>

#include "ble.h"
#include "displays/DisplayManager.h"
#include "pin.h"
#include <esp-iot-utils.h>

// Modules
#include "modules/EphemerisModule.h"
#include "modules/EventsModule.h"
#include "modules/ModuleManager.h"
#include "modules/SensorModule.h"

// --- Global Objects ---
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
ConfigHelper config;
Ble ble(config);

// --- Display Hardware Instances ---
// Screen 1 (Top Right) - BW
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT>
    display1(GxEPD2_420_GDEY042T81(CS_PIN_1, DC_PIN_1, RES_PIN_1, BUSY_PIN_1));

// Screen 0 (Top Left) - Color
GxEPD2_3C<GxEPD2_420c_GDEY042Z98, GxEPD2_420c_GDEY042Z98::HEIGHT>
    display2(GxEPD2_420c_GDEY042Z98(CS_PIN_2, DC_PIN_2, RES_PIN_2, BUSY_PIN_2));

GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT>
    display3(GxEPD2_420_GDEY042T81(CS_PIN_3, DC_PIN_3, RES_PIN_3, BUSY_PIN_3));
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT>
    display4(GxEPD2_420_GDEY042T81(CS_PIN_4, DC_PIN_4, RES_PIN_4, BUSY_PIN_4));

// Display Manager
DisplayManager displayManager(display2, display1, display3, display4,
                              u8g2Fonts);

// Module Manager
ModuleManager moduleManager(displayManager, config);

// Modules
EphemerisModule ephemerisModule;
EventsModule eventsModule;
SensorModule sensorModule1("Sensors", 0);
SensorModule sensorModule2("Sensors", 8);

// Global flag shared with BleConfig
volatile bool shouldUpdateSensors = false;

// BLE Timeout tracking
unsigned long lastBleActivity = 0;
bool bleActive = true;

void setup() {
  Serial.begin(115200);
  Serial.println("[Main] Booting System...");

  // 1. Hardware Init
  SPI.begin(SPI_SCK, -1, SPI_MOSI);

  // Set display pins as outputs/inputs
  pinMode(CS_PIN_1, OUTPUT);
  pinMode(DC_PIN_1, OUTPUT);
  pinMode(RES_PIN_1, OUTPUT);
  pinMode(BUSY_PIN_1, INPUT);
  pinMode(CS_PIN_2, OUTPUT);
  pinMode(DC_PIN_2, OUTPUT);
  pinMode(RES_PIN_2, OUTPUT);
  pinMode(BUSY_PIN_2, INPUT);
  pinMode(CS_PIN_3, OUTPUT);
  pinMode(DC_PIN_3, OUTPUT);
  pinMode(RES_PIN_3, OUTPUT);
  pinMode(BUSY_PIN_3, INPUT);
  pinMode(CS_PIN_4, OUTPUT);
  pinMode(DC_PIN_4, OUTPUT);
  pinMode(RES_PIN_4, OUTPUT);
  pinMode(BUSY_PIN_4, INPUT);

  config.begin();
  Serial.println("NVS Initialized");

  // Load language preference
  String lang = config.get("language", String("en"));
  TimeHelper::setLanguage(lang);
  Serial.println("Language loaded: " + lang);

  // 2. Display Init
  displayManager.init();

  // 3. BLE Init
  ble.begin();
  Serial.println("[Main] BLE Started");

  // 4. Network Init
  bool wifiConnected = WiFiHelper::connect(
      config.get("ssid", String("")), config.get("password", String("")),
      config.get("dns_mode", String("auto")),
      config.get("dns_pri", String("8.8.8.8")),
      config.get("dns_sec", String("1.1.1.1")));

  if (wifiConnected) {
    Serial.println("[Main] WiFi Connected: " + WiFiHelper::getIP());

    // 4.1 Time Init (Perform AFTER WiFi is connected)
    String ntpServer = config.get("ntp_srv", String("pool.ntp.org"));
    long gmtOffset = config.get("ntp_gmt", 3600L);
    int daylightOffset = config.get("ntp_dst", 3600);

    Serial.println("[Main] Initializing NTP: " + ntpServer);
    TimeHelper::init(ntpServer.c_str(), gmtOffset, daylightOffset);

    // Sync NTP
    struct tm timeinfo;
    Serial.println("[Main] Syncing NTP...");
    int retry = 0;
    while (!TimeHelper::getLocalTime(&timeinfo) && retry < 30) {
      if (retry % 5 == 0) {
        Serial.printf("[Main] NTP sync in progress... (Time: %lld)\n",
                      (long long)time(nullptr));
      }
      retry++;
      delay(1000);
    }

    if (retry < 30) {
      Serial.println(&timeinfo, "[Main] Time: %A, %B %d %Y %H:%M:%S");
      Serial.println("[Main] NTP Server: " + ntpServer);
      Serial.println("[Main] GMT Offset: " + String(gmtOffset));
      Serial.println("[Main] DST Offset: " + String(daylightOffset));
    } else {
      Serial.println("[Main] NTP Sync failed after retries");
    }
  } else {
    Serial.println("[Main] Offline Mode");
  }

  // 5. Config Tempus
  String tempusUrl = config.get("tempus_url", String(""));
  if (tempusUrl.isEmpty()) {
    Serial.println("[Main] Warning: Tempus URL not set!");
  } else {
    Serial.println("[Main] Tempus URL OK");
  }

  // 6. Register Modules
  moduleManager.registerModule(&ephemerisModule);
  moduleManager.registerModule(&sensorModule1);
  moduleManager.registerModule(&eventsModule);
  moduleManager.registerModule(&sensorModule2);

  // 7. Apply System Configuration (before begin)
  int sensorInterval = config.get("sensorInterval", 60);
  if (sensorInterval < 10)
    sensorInterval = 10;
  sensorModule1.setRefreshInterval(sensorInterval * 1000);
  sensorModule2.setRefreshInterval(sensorInterval * 1000);

  // 8. Start Modules (ModuleManager handles screen assignment and begin)
  Serial.println("[Main] Starting Modules...");
  moduleManager.begin();

  // 9. Initialize BLE timeout timer AFTER boot completes
  lastBleActivity = millis();
  int bleTimeoutMin = config.get("ble_timeout", 15);
  Serial.println("[Main] BLE timeout: " + String(bleTimeoutMin) + "min");
  Serial.println("[Main] === BOOT COMPLETE ===");
}

void loop() {
  // Check for BLE configuration changes
  if (shouldUpdateSensors) {
    Serial.println("[Main] Configuration changed - forcing updates");
    moduleManager.forceUpdate();
    shouldUpdateSensors = false;
    lastBleActivity = millis(); // Reset timeout on activity
  }

  // BLE Timeout Logic
  if (bleActive) {
    if (ble.isConnected()) {
      lastBleActivity = millis(); // Keep alive while connected
    } else {
      int timeoutMinutes = config.get("ble_timeout", 15);
      if (timeoutMinutes > 0 && (millis() - lastBleActivity >
                                 (unsigned long)timeoutMinutes * 60 * 1000)) {
        Serial.println("[Main] BLE Timeout - Stopping BLE");
        ble.stop();
        bleActive = false;
      }
    }
  }

  // Update all modules
  moduleManager.update();

  delay(100);
}