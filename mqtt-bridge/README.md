# MQTT Bridge Service

A Java Spring Boot service that bridges MQTT messages to REST API calls. This service listens to MQTT topics and forwards train detection messages to the DateTime API.

## Features

- Subscribes to MQTT topics for train detection events
- Automatically parses JSON messages from ESP32 devices
- Posts data to the REST API
- Automatic reconnection to MQTT broker
- Configurable via environment variables
- Fully containerized with Docker

## Architecture

```
ESP32 Device → MQTT Broker → MQTT Bridge → REST API → PostgreSQL
```

## Configuration

The service can be configured using environment variables:

| Variable | Description | Default |
|----------|-------------|---------|
| `MQTT_BROKER_URL` | MQTT broker URL | `tcp://mqtt-broker:1883` |
| `MQTT_CLIENT_ID` | MQTT client identifier | `mqtt-bridge-client` |
| `MQTT_TOPIC` | MQTT topic to subscribe | `homeassistant/togstyring/ir_sensor` |
| `MQTT_USERNAME` | MQTT username (optional) | `""` |
| `MQTT_PASSWORD` | MQTT password (optional) | `""` |
| `API_BASE_URL` | Base URL of the REST API | `http://api:8080` |

## Message Format

The bridge expects JSON messages in the following format:

```json
{
  "state": "detected",
  "rounds": 42,
  "relay": "activated",
  "timestamp": "2025-12-07T19:45:23"
}
```

## Running Standalone

### Prerequisites

- Java 17 or higher
- Maven 3.6+
- Access to an MQTT broker
- Running DateTime API instance

### Build

```bash
cd mqtt-bridge
mvn clean package
```

### Run

```bash
java -jar target/mqtt-bridge-1.0.0.jar \
  --mqtt.broker.url=tcp://localhost:1883 \
  --mqtt.topic=homeassistant/togstyring/ir_sensor \
  --api.base.url=http://localhost:8080
```

## Running with Docker

The MQTT bridge is included in the main docker-compose.yml file and runs automatically when you start the stack:

```bash
cd ..
docker-compose up --build
```

### Services Started

1. **PostgreSQL** - Database on port 5432
2. **API** - REST API on port 8080
3. **MQTT Broker** - Mosquitto broker on port 1883
4. **MQTT Bridge** - Bridge service on port 8081

## Testing

### 1. Start all services

```bash
docker-compose up --build
```

### 2. Publish a test message to MQTT

Using mosquitto_pub (install mosquitto-clients):

```bash
mosquitto_pub -h localhost -t homeassistant/togstyring/ir_sensor \
  -m '{"state":"detected","rounds":1,"relay":"activated","timestamp":"2025-12-07T19:45:23"}'
```

### 3. Verify the message was stored

```bash
curl http://localhost:8080/api/train/detection/latest
```

Expected response:
```json
{
  "state": "detected",
  "rounds": 1,
  "relay": "activated",
  "timestamp": "2025-12-07T19:45:23"
}
```

## Logs

View MQTT bridge logs:

```bash
docker logs mqtt-bridge
```

Follow logs in real-time:

```bash
docker logs -f mqtt-bridge
```

## Troubleshooting

### Connection to MQTT broker failed

**Problem**: Bridge cannot connect to MQTT broker

**Solution**: 
- Ensure the MQTT broker is running: `docker ps | grep mqtt-broker`
- Check broker logs: `docker logs mqtt-broker`
- Verify network connectivity between containers

### Messages not reaching API

**Problem**: MQTT messages are received but not posted to API

**Solution**:
- Check API is running: `curl http://localhost:8080/actuator/health`
- Verify message format matches expected JSON structure
- Check bridge logs for error messages

### Authentication errors

**Problem**: MQTT broker requires authentication

**Solution**:
Update docker-compose.yml with credentials:

```yaml
mqtt-bridge:
  environment:
    MQTT_USERNAME: "your_username"
    MQTT_PASSWORD: "your_password"
```

## Technology Stack

- Java 17
- Spring Boot 3.2.0
- Spring Integration MQTT
- Eclipse Paho MQTT Client
- Jackson for JSON processing
- Maven
- Docker

## Project Structure

```
mqtt-bridge/
├── src/
│   └── main/
│       ├── java/com/mqtt/bridge/
│       │   ├── MqttBridgeApplication.java
│       │   ├── MqttConfig.java
│       │   ├── MqttMessageHandler.java
│       │   └── TrainDetectionDto.java
│       └── resources/
│           └── application.properties
├── Dockerfile
└── pom.xml
```

## Integration with Home Assistant

If you're using Home Assistant with MQTT, the bridge will automatically capture messages published by your ESP32 device and store them in the database while still allowing Home Assistant to process them.

## License

This project is open source and available for personal and educational use.
