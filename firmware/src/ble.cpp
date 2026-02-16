#include "ble.h"
#include "modules/SensorModule.h"
#include <esp-iot-utils.h>

Ble::Ble(ConfigHelper &config)
    : _config(config),
      _pipe("E-PAPER-C6", SERVICE_UUID, CHARACTERISTIC_JSON_GATE_UUID) {}

void Ble::begin() {
  _pipe.setOnJson([this](const JsonDocument &doc) { handleCommand(doc); });
  _pipe.begin();
}

void Ble::handleCommand(const JsonDocument &doc) {
  String cmd = doc["cmd"] | "";

  if (cmd == "get_config") {
    JsonDocument res;
    res["cmd"] = "config_data";

    // System Settings
    res["ssid"] = _config.get("ssid", String(""));
    res["ntpServer"] = _config.get("ntp_srv", String("pool.ntp.org"));
    res["gmt"] = _config.get("ntp_gmt", 3600L);
    res["dst"] = _config.get("ntp_dst", 3600);
    res["dnsMode"] = _config.get("dns_mode", String("auto"));
    res["dnsPrimary"] = _config.get("dns_pri", String("8.8.8.8"));
    res["dnsSecondary"] = _config.get("dns_sec", String("1.1.1.1"));
    res["tempusUrl"] = _config.get("tempus_url", String(""));
    res["bleTimeout"] = _config.get("ble_timeout", 15);
    res["sensorInterval"] = _config.get("sensorInterval", 60);
    res["lang"] = _config.get("language", String("en"));
    res["style"] = _config.get("sens_style", 0);
    res["module_map"] = _config.get("module_map", String(""));

    // Sensors
    JsonArray sensors = res["sensors"].to<JsonArray>();
    for (int i = 0; i < 16; i++) {
      String key = "sensor_" + String(i);
      JsonDocument sDoc;
      if (!_config.get(key.c_str(), sDoc)) {
        sDoc["enabled"] = false;
        sDoc["label"] = "";
      }
      sensors.add(sDoc);
    }

    _pipe.sendJson(res);

  } else if (cmd == "save_config") {
    JsonObjectConst cfg = doc["config"];

    // Update individual values if present in the config object
    if (cfg["ssid"])
      _config.set("ssid", cfg["ssid"].as<String>());
    if (cfg["password"])
      _config.set("password", cfg["password"].as<String>());
    if (cfg["ntpServer"])
      _config.set("ntp_srv", cfg["ntpServer"].as<String>());
    if (cfg["gmt"])
      _config.set("ntp_gmt", cfg["gmt"].as<long>());
    if (cfg["dst"])
      _config.set("ntp_dst", cfg["dst"].as<int>());
    if (cfg["dnsMode"])
      _config.set("dns_mode", cfg["dnsMode"].as<String>());
    if (cfg["dnsPrimary"])
      _config.set("dns_pri", cfg["dnsPrimary"].as<String>());
    if (cfg["dnsSecondary"])
      _config.set("dns_sec", cfg["dnsSecondary"].as<String>());
    if (cfg["tempusUrl"])
      _config.set("tempus_url", cfg["tempusUrl"].as<String>());
    if (cfg["bleTimeout"])
      _config.set("ble_timeout", cfg["bleTimeout"].as<int>());
    if (cfg["sensorInterval"])
      _config.set("sensorInterval", cfg["sensorInterval"].as<int>());
    if (cfg["lang"]) {
      String lang = cfg["lang"].as<String>();
      _config.set("language", lang);
      TimeHelper::setLanguage(lang);
    }
    if (cfg["style"])
      _config.set("sens_style", cfg["style"].as<int>());
    if (cfg["module_map"])
      _config.set("module_map", cfg["module_map"].as<String>());

    // Update sensors if present
    if (cfg["sensors"]) {
      JsonArrayConst sensors = cfg["sensors"];
      int i = 0;
      for (JsonVariantConst s : sensors) {
        if (i >= 16)
          break;
        String key = "sensor_" + String(i++);
        _config.set(key.c_str(), s);
      }
    }

    shouldUpdateSensors = true;
    Serial.println("[BLE] Full Configuration Saved");

    // Ack
    JsonDocument ack;
    ack["cmd"] = "save_ok";
    _pipe.sendJson(ack);
  }
}

void Ble::stop() { _pipe.stop(); }

bool Ble::isConnected() { return _pipe.isConnected(); }
