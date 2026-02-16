# Development Guide

## Prerequisites

*   **PlatformIO** (VSCode Extension or CLI).
*   **Docker** & **Docker Compose** (for local web interface).

## Installation

1.  Clone the repository.
2.  Open the folder in VSCode with the PlatformIO extension installed.
3.  PlatformIO will automatically download dependencies (GxEPD2, ArduinoJson, etc.).

## Useful Commands

### Firmware

*   **Compile**: `make build`
*   **Upload**: `make flash`
*   **Serial Monitor**: `make monitor`

### Web Interface

To test the web interface without flashing the ESP32 (mocking) or for development:

```bash
make run-web
# or
docker compose up
```

The interface will be accessible at `http://localhost:8080`.
