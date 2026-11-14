#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>
#include <time.h>
#include "secrets.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// -------------------------
// Hardware opsætning
// -------------------------

const int buttonPin = 18;  // GPIO18 – startknap (Biltema 43-085, se README)
// Knappen forsynes fra ESP32'ens 3V3: ben 1 = 3V3, ben 2 = GPIO18, ben 3 = GND.
// Trykket giver ben 2 3,3 V, så knappen er aktiv HIGH med intern pull-down.
// ALDRIG 5 V eller 12 V på GPIO18 – pinnen tåler højst 3,3 V.
const bool buttonActiveHigh = true;
const int buttonIdleLevel = buttonActiveHigh ? LOW : HIGH;
const int relay1Pin = 23;  // GPIO23 – relæ 1 (bruges ikke, holdes slukket)
const int relay2Pin = 22;  // GPIO22 – relæ 2 (aktiveres af knappen)

// -------------------------
// Knap og relæ 2
// -------------------------

const unsigned long relayOnTime = 5000;   // ms relæ 2 er aktivt pr. tryk
const unsigned long debounceTime = 50;    // ms knappen skal være stabil

int lastButtonReading = buttonIdleLevel;
int stableButtonState = buttonIdleLevel;
unsigned long lastButtonFlip = 0;

bool relay2Active = false;
unsigned long relay2OnAt = 0;
int presses = 0;  // antal tryk der har aktiveret relæ 2

// Fejlsøgning: logger hvert skift på knap-pinnen og et livstegn hvert 2. sekund
const bool debugLog = true;
const unsigned long heartbeatInterval = 2000;
unsigned long lastHeartbeat = 0;

// -------------------------
// WiFi
// -------------------------

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// -------------------------
// MQTT (kun til logning)
// -------------------------
// MQTT kører i sin egen FreeRTOS-task. Knappen lægger beskeder i en kø
// uden at vente, så en manglende WiFi- eller MQTT-forbindelse aldrig
// forsinker knappen eller relæet. Beskeder der ikke kan sendes, droppes.

const char* mqtt_server = MQTT_SERVER;
const int mqtt_port = MQTT_PORT;
const char* mqtt_user = MQTT_USER;
const char* mqtt_password = MQTT_PASSWORD;
const char* mqtt_client_id = "ESP32_TogstyringMQTT";
const char* mqtt_topic = "homeassistant/togstyring/ir_sensor";

const unsigned long mqttRetryInterval = 5000;  // ms mellem forbindelsesforsøg

WiFiClient espClient;
PubSubClient mqttClient(espClient);

struct MqttMessage {
  char payload[160];
};
QueueHandle_t mqttQueue;
volatile bool mqttConnected = false;  // skrives af MQTT-task, læses af display

// -------------------------
// Display
// -------------------------

bool displayOk = false;
const unsigned long displayInterval = 250;  // ms mellem opdateringer
unsigned long lastDisplayUpdate = 0;

// -------------------------
// MQTT task
// -------------------------

void mqttTask(void* parameter) {
  unsigned long lastAttempt = 0;
  bool firstAttempt = true;
  MqttMessage msg;

  for (;;) {
    if (WiFi.status() == WL_CONNECTED) {
      if (!mqttClient.connected() &&
          (firstAttempt || millis() - lastAttempt >= mqttRetryInterval)) {
        firstAttempt = false;
        lastAttempt = millis();
        Serial.print("Forbinder til MQTT...");
        bool ok = strlen(mqtt_user) > 0
          ? mqttClient.connect(mqtt_client_id, mqtt_user, mqtt_password)
          : mqttClient.connect(mqtt_client_id);
        if (ok) {
          Serial.println("MQTT forbundet!");
        } else {
          Serial.printf("MQTT forbindelse fejlet, rc=%d - prøver igen om %lu sekunder\n",
                        mqttClient.state(), mqttRetryInterval / 1000);
        }
      }
      mqttClient.loop();
    }
    mqttConnected = mqttClient.connected();

    // Send ventende beskeder, eller drop dem hvis der ikke er forbindelse
    while (xQueueReceive(mqttQueue, &msg, 0) == pdTRUE) {
      if (mqttConnected && mqttClient.publish(mqtt_topic, msg.payload)) {
        Serial.printf("MQTT besked sendt: %s\n", msg.payload);
      } else {
        Serial.printf("MQTT ikke forbundet - besked droppet: %s\n", msg.payload);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// Læg en logbesked i køen. Venter aldrig; er køen fuld, droppes beskeden.
void queueButtonMessage() {
  MqttMessage msg;
  struct tm timeinfo;
  char timestamp[24] = "";
  if (getLocalTime(&timeinfo, 0)) {
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  }

  // Samme felter som før (state/rounds/relay/timestamp), så mqtt-bridge og API
  // kan modtage beskeden uændret. timestamp udelades hvis tiden ikke er kendt.
  if (timestamp[0] != '\0') {
    snprintf(msg.payload, sizeof(msg.payload),
             "{\"state\":\"button_pressed\",\"rounds\":%d,\"relay\":\"activated\",\"timestamp\":\"%s\"}",
             presses, timestamp);
  } else {
    snprintf(msg.payload, sizeof(msg.payload),
             "{\"state\":\"button_pressed\",\"rounds\":%d,\"relay\":\"activated\"}",
             presses);
  }

  if (xQueueSend(mqttQueue, &msg, 0) != pdTRUE) {
    Serial.println("MQTT kø fuld - besked droppet");
  }
}

// -------------------------
// Setup
// -------------------------

void setup() {
  // Relæer først, så de aldrig står tændt under opstart
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  digitalWrite(relay1Pin, LOW);
  digitalWrite(relay2Pin, LOW);
  pinMode(buttonPin, buttonActiveHigh ? INPUT_PULLDOWN : INPUT_PULLUP);

  Wire.begin(32, 33);
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nStarter togstyring...");
  Serial.printf("Knap GPIO%d ved opstart: %s\n", buttonPin, digitalRead(buttonPin) == LOW ? "LOW" : "HIGH");

  displayOk = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if (displayOk) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Starter...");
    display.display();
  } else {
    Serial.println("Display not found! Fortsætter uden display.");
  }

  // WiFi forbinder i baggrunden; knappen virker også uden netværk
  Serial.print("Forbinder til WiFi: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);

  // Tid via indbygget SNTP med dansk tidszone (bruges kun til MQTT-tidsstempel)
  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");

  // MQTT i egen task på core 0 (loop() kører på core 1)
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setSocketTimeout(2);
  mqttQueue = xQueueCreate(8, sizeof(MqttMessage));
  xTaskCreatePinnedToCore(mqttTask, "mqtt", 8192, NULL, 1, NULL, 0);
  Serial.println("MQTT konfigureret");
}

// -------------------------
// Knap: returnerer true én gang pr. tryk (debounced)
// -------------------------

bool buttonPressed(unsigned long now) {
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonReading) {
    lastButtonReading = reading;
    lastButtonFlip = now;
    if (debugLog) {
      Serial.printf("[pin] GPIO%d -> %s @%lu ms\n", buttonPin, reading == LOW ? "LOW" : "HIGH", now);
    }
  }
  if (now - lastButtonFlip >= debounceTime && reading != stableButtonState) {
    stableButtonState = reading;
    return stableButtonState != buttonIdleLevel;  // skift væk fra hvileniveau = tryk
  }
  return false;
}

// -------------------------
// Hjælpefunktion: Opdater OLED
// -------------------------

void showStatus(unsigned long now) {
  if (!displayOk) return;

  String wifiStatus = WiFi.status() == WL_CONNECTED
    ? WiFi.localIP().toString()
    : "forbinder...";

  struct tm timeinfo;
  char timeStr[9] = "--:--:--";
  if (getLocalTime(&timeinfo, 0)) {
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  }

  String relayStatus = "klar";
  if (relay2Active) {
    unsigned long remaining = relayOnTime - (now - relay2OnAt);
    relayStatus = "AKTIV " + String((remaining + 999) / 1000) + " sek";
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("WiFi: " + wifiStatus);
  display.println("Tid:  " + String(timeStr));
  display.println("MQTT: " + String(mqttConnected ? "OK" : "nede"));
  display.println("");
  display.println("Rel\x91: " + relayStatus);  // \x91 = æ i standardfonten
  display.println("Tryk: " + String(presses));
  display.display();
}

// -------------------------
// Loop – blokerer aldrig
// -------------------------

void loop() {
  unsigned long now = millis();

  if (buttonPressed(now)) {
    if (!relay2Active) {
      relay2Active = true;
      relay2OnAt = now;
      digitalWrite(relay2Pin, HIGH);
      presses++;
      Serial.printf("Knap trykket - relæ 2 aktiveret i %lu ms\n", relayOnTime);
      queueButtonMessage();
    } else {
      Serial.println("Knap trykket - ignoreret, relæ 2 er allerede aktivt");
    }
  }

  if (relay2Active && now - relay2OnAt >= relayOnTime) {
    relay2Active = false;
    digitalWrite(relay2Pin, LOW);
    Serial.println("Relæ 2 deaktiveret");
  }

  if (now - lastDisplayUpdate >= displayInterval) {
    lastDisplayUpdate = now;
    showStatus(now);
  }

  if (debugLog && now - lastHeartbeat >= heartbeatInterval) {
    lastHeartbeat = now;
    Serial.printf("[hb] %lu ms pin=%s relay2=%d presses=%d wifi=%d mqtt=%d display=%d\n",
                  now, digitalRead(buttonPin) == LOW ? "LOW" : "HIGH", relay2Active, presses,
                  WiFi.status() == WL_CONNECTED, (int)mqttConnected, displayOk);
  }
}
