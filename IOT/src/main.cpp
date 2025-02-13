#include "ECE140_WIFI.h"
#include "ECE140_MQTT.h"

// MQTT client - using descriptive client ID and topic
#define CLIENT_ID "esp32-sensors"
#define TOPIC_PREFIX "github/ece140/sensors"
#define SENSOR_TOPIC "readings"

ECE140_MQTT mqtt(CLIENT_ID, TOPIC_PREFIX);
ECE140_WIFI wifi;

// WiFi credentials
const char* ucsdUsername = UCSD_USERNAME;
const char* ucsdPassword = UCSD_PASSWORD;
const char* wifiSsid = WIFI_SSID;
const char* nonEnterpriseWifiPassword = NON_ENTERPRISE_WIFI_PASSWORD;


unsigned long lastPublishTime = 0;
const int publishInterval = 5000; 

void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
    Serial.print("[MQTT] Message received on topic: ");
    Serial.println(topic);

    Serial.print("[MQTT] Message: ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void scanWiFiNetworks() {
    Serial.println("Scanning WiFi networks...");
    int numNetworks = WiFi.scanNetworks();
    for (int i = 0; i < numNetworks; i++) {
        Serial.print("Network ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(WiFi.SSID(i));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("[Setup] Connecting to WiFi...");
    //wifi.connectToWPAEnterprise(wifiSsid, ucsdUsername, ucsdPassword);
    scanWiFiNetworks();
    //ESP 32 only work on 2.4GHZ!!!
    wifi.connectToWiFi(wifiSsid, nonEnterpriseWifiPassword);

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[Setup] WiFi Connected Successfully!");
    } else {
        Serial.println("[Setup] WiFi Connection Failed! Retrying...");
        while (WiFi.status() != WL_CONNECTED) {
            Serial.print(".");
            //wifi.connectToWPAEnterprise(wifiSsid, ucsdUsername, ucsdPassword);
            wifi.connectToWiFi(wifiSsid, nonEnterpriseWifiPassword);
            scanWiFiNetworks();
            delay(2000);
        }
        Serial.println("\n[Setup] Connected!");
    }

    mqtt.setCallback(mqttCallback);
    if (!mqtt.connectToBroker(1883)) {
        Serial.println("MQTT Connection failed!");
    }
}

void loop() {
    mqtt.loop();
    // Publish sensor data every 5000ms
    if (millis() - lastPublishTime >= publishInterval) {
        lastPublishTime = millis();

        int hallValue = hallRead();  // Read built-in hall sensor
        float temperature = random(20, 30) + random(0, 10) * 0.1;  // Mock temperature data

        // Format the sensor data as a JSON-like string (without external JSON library)
        String message = "{";
        message += "\"timestamp\":" + String(millis()) + ",";
        message += "\"hall_sensor\":" + String(hallValue) + ",";
        message += "\"temperature\":" + String(temperature);
        message += "}";

        // Publish data to MQTT
        if (mqtt.publishMessage(SENSOR_TOPIC, message)) {
            Serial.println("[MQTT] Sensor data published: " + message);
        } else {
            Serial.println("[MQTT] Failed to publish message!");
        }
    }
}