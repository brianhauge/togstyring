#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>
#include "secrets.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// -------------------------
// Hardware opsætning
// -------------------------

const int irSensorPin = 18;    // GPIO18 – infrared sensor DO
const int relay1Pin = 23;  // GPIO23 – relæ 1 (togstop)
const int relay2Pin = 22;  // GPIO22 – relæ 2 (tidsstyret)

// -------------------------
// WiFi
// -------------------------

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// -------------------------
// MQTT
// -------------------------

const char* mqtt_server = MQTT_SERVER;
const int mqtt_port = MQTT_PORT;
const char* mqtt_user = MQTT_USER;
const char* mqtt_password = MQTT_PASSWORD;
const char* mqtt_client_id = "ESP32_TogstyringMQTT";
const char* mqtt_topic = "homeassistant/togstyring/ir_sensor";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// -------------------------
// NTP klient
// -------------------------

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7200); 

// -------------------------
// Infrared sensor debounce
// -------------------------

bool lastIrState = HIGH;
unsigned long lastChangeTime = 0;
const unsigned long debounceDelay = 7500;  // ms (7,5 sekunder grace periode)

// -------------------------
// Tidsstyring for relæ 2
// -------------------------

struct tm timeinfo;
const int startHour = 16;
const int startMinute = 05;
const int stopHour = 21;
const int stopMinute = 15;
bool relay2State = false;
String timerM = "";
int rounds = 0;

// -------------------------
// Setup
// -------------------------

void setup() {
  Wire.begin(32, 33);
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Display not found!");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  delay(1000);
  Serial.println("\nStarter togstyring...");
  display.println("Starter...");
  display.display();

  pinMode(irSensorPin, INPUT_PULLUP);
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  digitalWrite(relay1Pin, LOW);
  digitalWrite(relay2Pin, LOW);

  // Forbind WiFi
  Serial.print("Forbinder til WiFi: ");
  display.println("WiFi forbinder...");
  display.display();
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
  }
  Serial.println("\nWiFi forbundet!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi forbundet!");
  display.println(WiFi.localIP());
  display.display();

  // Start NTP
  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");

  // Initialiser tilfældig generator
  randomSeed(analogRead(0));

  // Setup MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
  Serial.println("MQTT konfigureret");
}

// -------------------------
// MQTT forbindelse
// -------------------------
void reconnectMQTT() {
  // Loop indtil vi er forbundet
  if (!mqttClient.connected()) {
    Serial.print("Forbinder til MQTT...");
    
    // Forsøg at forbinde
    bool connected = false;
    if (strlen(mqtt_user) > 0) {
      connected = mqttClient.connect(mqtt_client_id, mqtt_user, mqtt_password);
    } else {
      connected = mqttClient.connect(mqtt_client_id);
    }
    
    if (connected) {
      Serial.println("MQTT forbundet!");
    } else {
      Serial.print("MQTT forbindelse fejlet, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" prøver igen om 5 sekunder");
    }
  }
}

// -------------------------
// Hjælpefunktion: Opdater OLED
// -------------------------
void showStatus(String wifiStatus, String timeStr, String irM, String timerM) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("WiFi: " + wifiStatus);
  display.println("Tid:  " + timeStr);
  display.println("");
  display.println("IR:   " + irM);
  display.println("Tog:  " + timerM);
  display.println("Omg:  " + String(rounds));
  display.display();
}


// -------------------------
// Loop
// -------------------------

void loop() {
  // Sikre MQTT forbindelse
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  // Hent aktuel tid
  int hour, minute, second, day, month, year;
  String timeStr = "";
  String dateTimeStr = "";
  String irM = "";
    

  if (getLocalTime(&timeinfo)) {
    hour   = timeinfo.tm_hour;
    minute = timeinfo.tm_min;
    second = timeinfo.tm_sec;
    day    = timeinfo.tm_mday;
    month  = timeinfo.tm_mon + 1;  // tm_mon er 0-11
    year   = timeinfo.tm_year + 1900;  // tm_year er år siden 1900
    
    timeStr = String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute) + ":" + (second < 10 ? "0" : "") + String(second);
    
    // ISO 8601 format: YYYY-MM-DDTHH:MM:SS
    dateTimeStr = String(year) + "-" + 
                  (month < 10 ? "0" : "") + String(month) + "-" + 
                  (day < 10 ? "0" : "") + String(day) + "T" + 
                  (hour < 10 ? "0" : "") + String(hour) + ":" + 
                  (minute < 10 ? "0" : "") + String(minute) + ":" + 
                  (second < 10 ? "0" : "") + String(second);
    
    //Serial.printf("Time: %02d:%02d:%02d\n", hour, minute, second);
  } else {
    Serial.println("Failed to obtain time");
  }

  // --- Opdater OLED med status ---
  String wifiStatus;
  if (WiFi.status() == WL_CONNECTED) {
    wifiStatus = WiFi.localIP().toString();
  } else {
    wifiStatus = "Ikke forbundet";
  }

  // Læs infrared sensor
  int irState = digitalRead(irSensorPin);
  unsigned long now = millis();

  // Debounce med 1/5 tilfældig chance
  if (irState != lastIrState && (now - lastChangeTime > debounceDelay)) {
    
    lastChangeTime = now;
    lastIrState = irState;


    if (irState == LOW) {
      rounds++;
      
      // Tjek om relæ skal aktiveres
      bool willActivateRelay = (random(5) == 0);
      
      // Send MQTT besked når IR sensor aktiveres med relay status og timestamp
      if (mqttClient.connected()) {
        String relayState = willActivateRelay ? "activated" : "not_activated";
        String payload = "{\"state\":\"detected\",\"rounds\":" + String(rounds) + ",\"relay\":\"" + relayState + "\",\"timestamp\":\"" + dateTimeStr + "\"}";
        mqttClient.publish(mqtt_topic, payload.c_str());
        Serial.println("MQTT besked sendt: " + payload);
      }
      
      if (willActivateRelay) {
        int randomDelay = random(4, 8) * 1000;  // Tilfældig tid mellem 4-7 sekunder
        Serial.printf("IR sensor aktiveret - aktiverer relæ 1 i %d ms (1/5 chance)\n", randomDelay);
        irM = String(randomDelay/1000) + " sek";
        showStatus(wifiStatus, timeStr, irM, timerM);
        digitalWrite(relay1Pin, HIGH);
        delay(randomDelay);
        digitalWrite(relay1Pin, LOW);
        Serial.println("Deaktiverer relæ 1.");
      } else {
        Serial.println("IR sensor aktiveret - ingen handling");
      }
    }
  }

  // -------------------------
  // Tidsstyret relæ 2
  // -------------------------
  int currentTotal = hour * 60 + minute;
  int startTotal = startHour * 60 + startMinute;
  int stopTotal = stopHour * 60 + stopMinute;
  bool withinActiveTime = (currentTotal >= startTotal && currentTotal < stopTotal);

  if (withinActiveTime && !relay2State) {
    relay2State = true;
    digitalWrite(relay2Pin, HIGH);
    Serial.printf("%02d:%02d - tidsvindue åbent, relæ 2 AKTIVERET\n", hour, minute);
    timerM = "Fut Fut";
  } else if (!withinActiveTime && relay2State) {
    relay2State = false;
    digitalWrite(relay2Pin, LOW);
    Serial.printf("%02d:%02d - udenfor tidsvindue, relæ 2 DEAKTIVERET\n", hour, minute);
    timerM = "Garage";
  }
  showStatus(wifiStatus, timeStr, irM, timerM);
}
