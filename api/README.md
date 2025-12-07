# DateTime API with PostgreSQL

A simple Java Spring Boot REST API for managing begin and end date-time records, backed by PostgreSQL database. Includes ESP32 train detection integration.

## Features

- RESTful API for creating, reading, updating, and deleting date-time records
- ESP32 train detection data logging (compatible with TogEsp32 project)
- MQTT bridge service for automatic message forwarding
- Built-in Mosquitto MQTT broker
- PostgreSQL database for persistent storage
- Fully containerized with Docker
- Configurable external volume for database persistence

## API Endpoints

### DateTime Endpoints

#### Create a new date-time record
```bash
POST /api/datetime
Content-Type: application/json

{
  "beginDateTime": "2025-12-07T10:00:00",
  "endDateTime": "2025-12-07T18:00:00"
}
```

### Get a specific record by ID
```bash
GET /api/datetime/{id}
```

### Get all records
```bash
GET /api/datetime
```

### Get the latest record
```bash
GET /api/datetime/latest
```

### Update a record
```bash
PUT /api/datetime/{id}
Content-Type: application/json

{
  "beginDateTime": "2025-12-07T09:00:00",
  "endDateTime": "2025-12-07T17:00:00"
}
```

#### Delete a record
```bash
DELETE /api/datetime/{id}
```

### ESP32 Train Detection Endpoints

These endpoints are compatible with the TogEsp32 train control system format.

#### Post a train detection (ESP32 format)
```bash
POST /api/train/detection
Content-Type: application/json

{
  "state": "detected",
  "rounds": 42,
  "relay": "activated",
  "timestamp": "2025-12-07T19:45:23"
}
```

Response:
```json
{
  "state": "detected",
  "rounds": 42,
  "relay": "activated",
  "timestamp": "2025-12-07T19:45:23"
}
```

#### Get latest train detection
```bash
GET /api/train/detection/latest
```

#### Get all train detections
```bash
GET /api/train/detection
```

#### Get train detection by ID
```bash
GET /api/train/detection/{id}
```

#### Get train statistics
```bash
GET /api/train/stats
```

Response:
```json
{
  "totalDetections": 150,
  "activatedCount": 30,
  "notActivatedCount": 120,
  "maxRounds": 150
}
```

## Prerequisites

- Docker
- Docker Compose

## Getting Started

### 1. Build and run the application

```bash
docker-compose up --build
```

This will:
- Build the Java API and MQTT bridge applications
- Start a PostgreSQL database container
- Start a Mosquitto MQTT broker
- Start the API container
- Start the MQTT bridge container
- Create persistent volumes for database and MQTT broker

### 2. Access the services

The services will be available at:
- **REST API**: `http://localhost:8080`
- **MQTT Broker**: `tcp://localhost:1883`
- **MQTT Bridge**: `http://localhost:8081`

### 3. Test the API

#### DateTime API:
Create a new record:
```bash
curl -X POST http://localhost:8080/api/datetime \
  -H "Content-Type: application/json" \
  -d "{\"beginDateTime\":\"2025-12-07T10:00:00\",\"endDateTime\":\"2025-12-07T18:00:00\"}"
```

Get all records:
```bash
curl http://localhost:8080/api/datetime
```

Get the latest record:
```bash
curl http://localhost:8080/api/datetime/latest
```

#### ESP32 Train Detection API:
Post a train detection:
```bash
curl -X POST http://localhost:8080/api/train/detection \
  -H "Content-Type: application/json" \
  -d "{\"state\":\"detected\",\"rounds\":42,\"relay\":\"activated\",\"timestamp\":\"2025-12-07T19:45:23\"}"
```

Get all train detections:
```bash
curl http://localhost:8080/api/train/detection
```

Get latest detection:
```bash
curl http://localhost:8080/api/train/detection/latest
```

Get statistics:
```bash
curl http://localhost:8080/api/train/stats
```

### 4. Test MQTT Bridge

Publish a message to MQTT broker (requires mosquitto-clients):

```bash
mosquitto_pub -h localhost -t homeassistant/togstyring/ir_sensor \
  -m '{"state":"detected","rounds":5,"relay":"activated","timestamp":"2025-12-07T20:15:30"}'
```

The MQTT bridge will automatically receive this message and post it to the API. Verify:

```bash
curl http://localhost:8080/api/train/detection/latest
```

## Configuration

### Database Configuration

The PostgreSQL database configuration can be modified in `docker-compose.yml`:

```yaml
environment:
  POSTGRES_DB: datetime_db
  POSTGRES_USER: postgres
  POSTGRES_PASSWORD: postgres
```

### Volume Configuration

The database data is stored in a Docker volume named `postgres_data`. This ensures data persistence even when containers are stopped or removed.

To use a custom location for the database files, modify the `docker-compose.yml`:

```yaml
volumes:
  postgres_data:
    driver: local
    driver_opts:
      type: none
      o: bind
      device: /path/to/your/data/directory
```

Or use a bind mount directly:

```yaml
volumes:
  - /path/to/your/data/directory:/var/lib/postgresql/data
```

## Stopping the Application

```bash
docker-compose down
```

To stop and remove volumes (deletes all data):
```bash
docker-compose down -v
```

## MQTT Bridge

The MQTT bridge service automatically listens to MQTT messages and forwards them to the REST API. This is perfect for ESP32 devices that publish via MQTT.

### Configuration

Configure the MQTT bridge in `docker-compose.yml`:

```yaml
mqtt-bridge:
  environment:
    MQTT_BROKER_URL: tcp://mqtt-broker:1883
    MQTT_TOPIC: homeassistant/togstyring/ir_sensor
    MQTT_USERNAME: ""  # Set if using authentication
    MQTT_PASSWORD: ""  # Set if using authentication
    API_BASE_URL: http://api:8080
```

### Using an External MQTT Broker

To use Home Assistant's MQTT broker instead of the built-in one:

1. Update the `mqtt-bridge` service in `docker-compose.yml`:
   ```yaml
   mqtt-bridge:
     environment:
       MQTT_BROKER_URL: tcp://your-home-assistant-ip:1883
       MQTT_USERNAME: "your_mqtt_user"
       MQTT_PASSWORD: "your_mqtt_password"
   ```

2. Remove or comment out the `mqtt-broker` service if not needed.

## ESP32 Integration

This API is compatible with the [TogEsp32 train control system](https://github.com/brianhauge/togstyring). 

### Option 1: Using MQTT (Recommended)

Your ESP32 already publishes to MQTT. Just ensure it publishes to the same broker and topic configured in the MQTT bridge:

- **Topic**: `homeassistant/togstyring/ir_sensor`
- **Broker**: Your Home Assistant MQTT broker or the built-in Mosquitto broker

The MQTT bridge will automatically capture and store all messages in the database.

### Option 2: Direct HTTP POST

Alternatively, modify the ESP32 code to make HTTP POST requests directly to the API:

```cpp
#include <HTTPClient.h>

// In your detection code:
HTTPClient http;
http.begin("http://your-api-server:8080/api/train/detection");
http.addHeader("Content-Type", "application/json");

String payload = "{\"state\":\"detected\",\"rounds\":" + String(rounds) + 
                 ",\"relay\":\"" + relayState + "\",\"timestamp\":\"" + dateTimeStr + "\"}";
int httpCode = http.POST(payload);
http.end();
```

## Project Structure

```
.
├── src/                          # Main API application
│   └── main/
│       ├── java/com/datetime/api/
│       │   ├── DateTimeApiApplication.java
│       │   ├── DateTimeRecord.java
│       │   ├── DateTimeRepository.java
│       │   ├── DateTimeController.java
│       │   ├── TrainDetection.java
│       │   ├── TrainDetectionRepository.java
│       │   └── TrainDetectionController.java
│       └── resources/
│           └── application.properties
├── mqtt-bridge/                  # MQTT Bridge service
│   ├── src/
│   │   └── main/
│   │       ├── java/com/mqtt/bridge/
│   │       │   ├── MqttBridgeApplication.java
│   │       │   ├── MqttConfig.java
│   │       │   ├── MqttMessageHandler.java
│   │       │   └── TrainDetectionDto.java
│   │       └── resources/
│   │           └── application.properties
│   ├── Dockerfile
│   ├── pom.xml
│   └── README.md
├── Dockerfile
├── docker-compose.yml
├── pom.xml
└── README.md
```

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

## Technology Stack

- Java 17
- Spring Boot 3.2.0
- Spring Data JPA
- Spring Integration MQTT
- Eclipse Paho MQTT Client
- PostgreSQL 16
- Eclipse Mosquitto MQTT Broker
- Maven
- Docker
