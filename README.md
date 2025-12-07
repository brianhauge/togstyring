# Togstyring - ESP32 Train Control System

ESP32-based train control system with infrared sensor detection, dual relay control, OLED display, and Home Assistant MQTT integration.

## Features

- **Infrared Sensor Detection**: Automatically detects train passage using IR sensor on GPIO18
- **Dual Relay Control**:
  - Relay 1 (GPIO23): Random train stop with 1/5 activation chance (5-10 second delays)
  - Relay 2 (GPIO22): Time-based control (active 16:05-21:15 daily)
- **OLED Display**: Real-time status display showing WiFi, time, sensor status, and round count
- **MQTT Integration**: Publishes detection events to Home Assistant with relay state
- **NTP Time Sync**: Automatic time synchronization with configurable timezone
- **5-Second Grace Period**: Debounce protection for sensor readings

## Hardware Requirements

- ESP32 Development Board
- Infrared sensor module (connected to GPIO18)
- 2x Relay modules (GPIO23 and GPIO22)
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

The system publishes JSON messages to the topic `homeassistant/togstyring/ir_sensor` whenever the IR sensor detects the train:

```json
{
  "state": "detected",
  "rounds": 42,
  "relay": "activated"
}
```

or

```json
{
  "state": "detected",
  "rounds": 43,
  "relay": "not_activated"
}
```

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
    condition:
      - condition: template
        value_template: "{{ trigger.payload_json.relay == 'activated' }}"
    action:
      - service: notify.notify
        data:
          message: "Train passed and stopped! Round {{ trigger.payload_json.rounds }}"
```

## Pin Configuration

| Component | GPIO Pin |
|-----------|----------|
| IR Sensor | GPIO18 |
| Relay 1 (Train Stop) | GPIO23 |
| Relay 2 (Time Control) | GPIO22 |
| OLED SDA | GPIO32 |
| OLED SCL | GPIO33 |

## Configuration Options

### Time-Based Control (Relay 2)

Edit in `src/TogMedDisplay.ino`:

```cpp
const int startHour = 16;     // Start hour (24-hour format)
const int startMinute = 05;   // Start minute
const int stopHour = 21;      // Stop hour
const int stopMinute = 15;    // Stop minute
```

### Grace Period

Edit in `src/TogMedDisplay.ino`:

```cpp
const unsigned long debounceDelay = 5000;  // ms (5 seconds)
```

### Random Activation Probability

Edit in `src/TogMedDisplay.ino`:

```cpp
bool willActivateRelay = (random(5) == 0);  // 1/5 chance = 20%
```

Change `5` to adjust probability (e.g., `4` = 25%, `10` = 10%)

## Serial Monitor Output

When running, you'll see output like:

```
Starter togstyring...
Forbinder til WiFi: YourNetwork
..
WiFi forbundet!
IP: 192.168.1.120
MQTT konfigureret
Forbinder til MQTT...MQTT forbundet!
19:44 - tidsvindue åbent, relæ 2 AKTIVERET
MQTT besked sendt: {"state":"detected","rounds":1,"relay":"activated"}
IR sensor aktiveret - aktiverer relæ 1 i 7000 ms (1/5 chance)
Deaktiverer relæ 1.
```

## Dependencies

All dependencies are automatically managed by PlatformIO:

- Adafruit GFX Library (^1.11.3)
- Adafruit SSD1306 (^2.5.7)
- NTPClient (^3.2.1)
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
