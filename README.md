# TogEsp32 Train Control System

A complete IoT system for ESP32-based train control with MQTT messaging, REST API, and PostgreSQL database. This repository contains three integrated projects:

## Projects

### 1. ESP32 Firmware (`esp32/`)
ESP32-based train control system with infrared sensor detection, dual relay control, OLED display, and MQTT integration.

**Features:**
- Infrared sensor detection for train passage
- Dual relay control (random stops + time-based activation)
- OLED display for real-time status
- MQTT publishing to Home Assistant
- NTP time synchronization

[Read more →](esp32/README.md)

### 2. REST API (`api/`)
Java Spring Boot REST API for managing date-time records and train detection data.

**Features:**
- RESTful endpoints for date-time records
- Train detection data logging
- PostgreSQL database integration
- Fully containerized with Docker

[Read more →](api/README.md)

### 3. MQTT Bridge (`mqtt-bridge/`)
Java Spring Boot service that bridges MQTT messages to REST API calls.

**Features:**
- Subscribes to MQTT topics
- Automatic message parsing and forwarding
- Configurable via environment variables
- Auto-reconnection to MQTT broker

[Read more →](mqtt-bridge/README.md)

## Architecture

```
┌─────────────┐
│  ESP32      │ ──MQTT──┐
│  Device     │         │
└─────────────┘         │
                        ▼
                 ┌──────────────┐
                 │ MQTT Broker  │
                 │ (Mosquitto)  │
                 └──────────────┘
                        │
                        │ Subscribe
                        ▼
                 ┌──────────────┐
                 │ MQTT Bridge  │
                 │   Service    │
                 └──────────────┘
                        │
                        │ HTTP POST
                        ▼
                 ┌──────────────┐
                 │  REST API    │
                 └──────────────┘
                        │
                        ▼
                 ┌──────────────┐
                 │  PostgreSQL  │
                 │   Database   │
                 └──────────────┘
```

## Quick Start

### Prerequisites

- Docker and Docker Compose
- For ESP32 development: PlatformIO

### 1. Start Backend Services

```bash
docker-compose up --build
```

This starts:
- PostgreSQL database (port 5432)
- REST API (port 8080)
- Mosquitto MQTT broker (port 1883)
- MQTT bridge service (port 8081)

### 2. Configure ESP32

Navigate to the ESP32 project:

```bash
cd esp32
cp src/secrets.h.example src/secrets.h
```

Edit `src/secrets.h` with your WiFi and MQTT credentials.

### 3. Flash ESP32

```bash
cd esp32
pio run -t upload
pio device monitor
```

## Services

| Service | Port | Description |
|---------|------|-------------|
| REST API | 8080 | HTTP REST API endpoints |
| MQTT Broker | 1883 | Mosquitto MQTT broker |
| MQTT Bridge | 8081 | MQTT to REST bridge service |
| PostgreSQL | 5432 | Database |

## API Endpoints

### DateTime Records

- `POST /api/datetime` - Create date-time record
- `GET /api/datetime` - Get all records
- `GET /api/datetime/{id}` - Get specific record
- `GET /api/datetime/latest` - Get latest record
- `PUT /api/datetime/{id}` - Update record
- `DELETE /api/datetime/{id}` - Delete record

### Train Detection

- `POST /api/train/detection` - Log train detection
- `GET /api/train/detection` - Get all detections
- `GET /api/train/detection/latest` - Get latest detection
- `GET /api/train/detection/{id}` - Get specific detection
- `GET /api/train/stats` - Get statistics

## Testing

### Test MQTT Publishing

Publish a test message:

```bash
mosquitto_pub -h localhost -t homeassistant/togstyring/ir_sensor \
  -m '{"state":"detected","rounds":5,"relay":"activated","timestamp":"2025-12-07T20:15:30"}'
```

### Verify Data Storage

Check if the message was stored:

```bash
curl http://localhost:8080/api/train/detection/latest
```

### Get Statistics

```bash
curl http://localhost:8080/api/train/stats
```

## Configuration

### Environment Variables

Create a `.env` file (see `.env.example`):

```env
# MQTT Configuration
MQTT_BROKER_URL=tcp://mqtt-broker:1883
MQTT_TOPIC=homeassistant/togstyring/ir_sensor
MQTT_USERNAME=
MQTT_PASSWORD=

# API Configuration
API_BASE_URL=http://api:8080

# Database Configuration
POSTGRES_DB=datetime_db
POSTGRES_USER=postgres
POSTGRES_PASSWORD=postgres
```

### Using External MQTT Broker

To use Home Assistant's MQTT broker:

```yaml
mqtt-bridge:
  environment:
    MQTT_BROKER_URL: tcp://your-homeassistant-ip:1883
    MQTT_USERNAME: "your_mqtt_user"
    MQTT_PASSWORD: "your_mqtt_password"
```

## Project Structure

```
.
├── api/                          # REST API Service
│   ├── src/
│   │   └── main/
│   │       ├── java/com/datetime/api/
│   │       └── resources/
│   ├── Dockerfile
│   ├── pom.xml
│   └── README.md
├── mqtt-bridge/                  # MQTT Bridge Service
│   ├── src/
│   │   └── main/
│   │       ├── java/com/mqtt/bridge/
│   │       └── resources/
│   ├── Dockerfile
│   ├── pom.xml
│   └── README.md
├── esp32/                        # ESP32 Firmware
│   ├── src/
│   │   ├── TogMedDisplay.ino
│   │   └── secrets.h.example
│   ├── platformio.ini
│   ├── deploy.bat
│   ├── deploy.ps1
│   └── README.md
├── docker-compose.yml            # Docker orchestration
├── .env.example                  # Environment variables template
├── .gitignore
└── README.md                     # This file
```

## Technology Stack

### Backend
- Java 17
- Spring Boot 3.2.0
- Spring Data JPA
- Spring Integration MQTT
- PostgreSQL 16
- Eclipse Mosquitto
- Maven

### ESP32
- Arduino Framework
- PlatformIO
- Adafruit libraries (GFX, SSD1306)
- PubSubClient (MQTT)
- NTPClient

### DevOps
- Docker & Docker Compose
- Multi-stage builds

## Development

### Building Individual Services

**API:**
```bash
cd api
mvn clean package
```

**MQTT Bridge:**
```bash
cd mqtt-bridge
mvn clean package
```

**ESP32:**
```bash
cd esp32
pio run
```

## Stopping Services

Stop all Docker services:

```bash
docker-compose down
```

Stop and remove all data:

```bash
docker-compose down -v
```

## Troubleshooting

### MQTT Connection Issues

Check if broker is running:
```bash
docker logs mqtt-broker
```

### Database Connection Issues

Verify PostgreSQL is healthy:
```bash
docker ps | grep postgres
```

### API Not Responding

Check API logs:
```bash
docker logs datetime-api
```

### ESP32 Not Publishing

1. Verify WiFi credentials in `esp32/src/secrets.h`
2. Check serial monitor output: `pio device monitor`
3. Ensure MQTT broker is accessible from ESP32's network

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is open source and available for personal and educational use.

## Author

Brian Hauge ([@brianhauge](https://github.com/brianhauge))
