# Togstyring - ESP32 Train Control System

ESP32-based train control with a start button, a timed relay, an OLED display, and optional MQTT logging to Home Assistant / the backend.

## Features

- **Start button** (GPIO18): one press switches relay 2 on for 5 seconds
- **Lockout**: presses while relay 2 is on are ignored, so repeated presses never extend or restart the 5 seconds
- **Debounced input**: a press must be stable for 50 ms; holding the button counts as one press
- **Non-blocking**: the main loop never waits, so the button and relay work even with no WiFi or MQTT
- **OLED display**: WiFi, time, MQTT status, relay countdown and press count
- **MQTT logging** (optional): each accepted press is published; if MQTT is down the message is dropped
- **Time sync**: built-in SNTP with Danish time zone, used for the MQTT timestamp

## Hardware Requirements

- ESP32 Development Board
- Start button (Biltema art. 43-085 car starter button), powered from the ESP32's 3V3 - see [Button wiring](#button-wiring)
- Relay module on GPIO22 (relay 2). GPIO23 (relay 1) is no longer used and is held LOW
- SSD1306 OLED Display (128x64, I2C on GPIO32/33)
- Power supply for ESP32 and relays

## Software Requirements

- [PlatformIO](https://platformio.org/)
- Python 3.x (for PlatformIO)
- Home Assistant with Mosquitto MQTT broker (optional for MQTT features)

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/brianhauge/togstyring.git
cd togstyring
```

### 2. Configure Credentials

Copy the example secrets file and add your credentials:

```bash
cp src/secrets.h.example src/secrets.h
```

Edit `src/secrets.h` with your WiFi and MQTT settings:

```cpp
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
#define MQTT_SERVER "192.168.1.xxx"
#define MQTT_PORT 1883
#define MQTT_USER "your_mqtt_username"
#define MQTT_PASSWORD "your_mqtt_password"
```

### 3. Build and Upload

#### Quick Deploy (Recommended)

Use the included deployment scripts for a streamlined workflow:

**Windows (Batch):**
```batch
deploy.bat          # Build, upload, and monitor (default)
deploy.bat build    # Build only
deploy.bat upload   # Build and upload
deploy.bat monitor  # Serial monitor only
deploy.bat clean    # Clean build files
deploy.bat help     # Show help
```

**Windows/Linux/macOS (PowerShell):**
```powershell
./deploy.ps1          # Build, upload, and monitor (default)
./deploy.ps1 build    # Build only
./deploy.ps1 upload   # Build and upload
./deploy.ps1 monitor  # Serial monitor only
./deploy.ps1 clean    # Clean build files
./deploy.ps1 help     # Show help
```

`deploy.bat` is a thin wrapper around `deploy.ps1`, so both accept the same commands.

The serial port is auto-detected by PlatformIO. To pin it, pass `-Port` or set the
`ESP32_PORT` environment variable:

```powershell
./deploy.ps1 upload -Port COM7
```

#### Manual PlatformIO Commands

```bash
# Install PlatformIO if not already installed
pip install -U platformio

# Build the project
pio run

# Upload to ESP32 (ensure ESP32 is connected via USB)
pio run -t upload

# Monitor serial output
pio device monitor -b 115200
```

## MQTT Integration

MQTT is used for logging only. Each accepted button press publishes to the topic `homeassistant/togstyring/ir_sensor`:

```json
{
  "state": "button_pressed",
  "rounds": 42,
  "relay": "activated",
  "timestamp": "2026-11-14T19:45:23"
}
```

- `rounds` is the number of presses that activated relay 2 since boot. Ignored presses are not counted or published.
- `timestamp` is ISO 8601 local time. It is left out until the clock has synced after boot.
- The fields match the old IR message, so `mqtt-bridge` and the API accept it unchanged.

MQTT runs in its own FreeRTOS task. A press only drops the message into a queue without waiting; if WiFi or MQTT is down the message is dropped, and the button and relay are not affected. The task retries the broker connection every 5 seconds.

### Home Assistant Setup

1. Install Mosquitto MQTT broker:
   - Go to **Settings** → **Add-ons** → **Add-on Store**
   - Search for "Mosquitto broker" and install
   - Configure with your MQTT credentials
   - Start the add-on

2. Add MQTT integration:
   - Go to **Settings** → **Devices & Services**
   - Add **MQTT** integration
   - Configure with broker details

3. Listen to messages:
   - Go to **Settings** → **Devices & Services** → **MQTT** → **Configure**
   - Click "Listen to a topic"
   - Enter topic: `homeassistant/togstyring/ir_sensor`

### Example Automation

```yaml
automation:
  - alias: "Train Detection Notification"
    trigger:
      - platform: mqtt
        topic: "homeassistant/togstyring/ir_sensor"
    action:
      - service: notify.notify
        data:
          message: "Start button pressed ({{ trigger.payload_json.rounds }} times)"
```

## Pin Configuration

| Component | GPIO Pin |
|-----------|----------|
| Start button, pin 2 | GPIO18 |
| Relay 1 (unused, held LOW) | GPIO23 |
| Relay 2 (button-activated) | GPIO22 |
| OLED SDA | GPIO32 |
| OLED SCL | GPIO33 |

## Button wiring

The button is a Biltema starter button (art. 43-085). The pin numbers below are the ones printed on its box, which differ from the website:

| Button pin | Box label | Connect to ESP32 |
|---|---|---|
| 1 | Battery positive | **3V3** |
| 2 | To device (live while pressed) | **GPIO18** |
| 3 | Battery negative (lamp) | **GND** |

Pressing the button connects pin 1 to pin 2: GPIO18 goes from LOW to 3.3 V and the lamp lights. The firmware uses the internal pull-down and treats HIGH as pressed (`buttonActiveHigh = true`).

> **Warning:** never wire the button to 5 V or 12 V while pin 2 goes to GPIO18. ESP32 pins tolerate at most 3.3 V. If the button must run on 12 V, put an optocoupler (e.g. PC817) between it and GPIO18 and set `buttonActiveHigh = false`.

## Configuration Options

Edit in `src/TogMedDisplay.ino`:

```cpp
const unsigned long relayOnTime = 5000;        // ms relay 2 stays on per press
const unsigned long debounceTime = 50;         // ms the button must be stable
const unsigned long mqttRetryInterval = 5000;  // ms between MQTT reconnect attempts
```

## Display

```
WiFi: 192.168.1.120
Tid:  19:45:23
MQTT: OK

Relæ: AKTIV 3 sek
Tryk: 42
```

`Relæ` shows `klar` (ready) when relay 2 is off and a countdown while it is on.

## Serial Monitor Output

```
Starter togstyring...
Forbinder til WiFi: YourNetwork
MQTT konfigureret
Forbinder til MQTT...MQTT forbundet!
Knap trykket - relæ 2 aktiveret i 5000 ms
MQTT besked sendt: {"state":"button_pressed","rounds":1,"relay":"activated","timestamp":"2026-11-14T19:45:23"}
Knap trykket - ignoreret, relæ 2 er allerede aktivt
Relæ 2 deaktiveret
```

## Dependencies

All dependencies are automatically managed by PlatformIO:

- Adafruit GFX Library (^1.11.3)
- Adafruit SSD1306 (^2.5.7)
- PubSubClient (^2.8)

## Troubleshooting

### MQTT Connection Failed (rc=-2)
- MQTT broker is not reachable
- Check network connectivity
- Verify MQTT server IP address

### MQTT Connection Failed (rc=5)
- Authentication failed
- Check MQTT username and password in `secrets.h`
- Ensure Mosquitto broker is configured with credentials

### Display Not Found
- The controller keeps running without the display; the button and relay still work
- Check I2C connections (SDA/SCL)
- Verify display I2C address (default: 0x3C)
- Check power supply to display

### WiFi Connection Issues
- Verify SSID and password in `secrets.h`
- Check WiFi signal strength
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)

## License

This project is open source and available for personal and educational use.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Author

Brian Hauge ([@brianhauge](https://github.com/brianhauge))
