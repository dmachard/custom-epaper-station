# Custom E-Paper Station

A connected information display based on an **ESP32-C6** and a **4.2" E-Paper Screen**.

<img src="station-epaper" alt="Custom epaper station" width="300">

## 🌟 Features

*   **Multi-Screen Display**:
    *   **Events**: Displays upcoming calendar events.
    *   **Sensors**: 16 configurable slots for displaying data from APIs (Prometheus, JSON).
*   **Connectivity**:
    *   **WiFi**: For data fetching methods.
    *   **Bluetooth LE (BLE)**: For easy configuration via a Web App.
*   **Web Configuration App**:

## 🛠️ Hardware

*   **Microcontroller**: ESP32-C6 (RISC-V)
*   **Display**: GoodDisplay GDEY042T81 (4.2" E-Ink) or compatible GxEPD2 supported screen.

## 📂 Project Structure

*   `firmware/`: PlatformIO project (C++ source code).
*   `web/`: The Configuration Web App (HTML/JS/CSS).
*   `docs/`: Additional documentation.
*   `enclosure/`: STL files

## 🚀 Getting Started

### 1. Firmware
1.  Open the `firmware` folder in VS Code with the **PlatformIO** extension.
2.  Connect your ESP32-C6.
3.  Click **Upload**.

### 2. Web Interface
1.  Open `web/index.html` in a Chrome-based browser or your mobile
2.  Click **Connect to Device**.
3.  Select your ESP32 from the list (named "ESP32_Config").
4.  Configure your WiFi and Sensors!

## 🔧 Configuration

The device exposes a generic sensor system. You can map any JSON API value to a display slot.
*   **Source Type**: `Prometheus` (raw text) or `JSON`.
*   **JSON Path**: Extract specific values using dot notation (e.g., `feeds.field1`).
*   **Adjustments**: Set divisors, decimals, and units.
