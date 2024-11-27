#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <HX711.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Wi-Fi credentials
const char* ssid = "kong";            // Replace with your Wi-Fi SSID
const char* password = "password08";  // Replace with your Wi-Fi password

// MQTT Broker settings
const char* mqtt_server = "broker.hivemq.com";  // MQTT broker address
const int mqtt_port = 1883;                     // MQTT port
const char* mqtt_topic = "kec/sensors/data";    // Topic for publishing GPS and weight data

// GPS module pins and settings
const int RX_PIN = 3;               // GPS RX pin
const int TX_PIN = 4;               // GPS TX pin
const uint32_t GPS_BAUD = 9600;     // GPS baud rate
TinyGPSPlus gps;                    // TinyGPS++ object
SoftwareSerial gpsSerial(RX_PIN, TX_PIN);  // GPS serial interface

// Load cell pins and settings
const int LoadCellDoutPin = 12;     // GPIO 12 (D6 on most ESP8266 boards)
const int LoadCellSckPin = 14;      // GPIO 14 (D5 on most ESP8266 boards)
HX711 scale;                        // HX711 instance

// MQTT and Wi-Fi client
WiFiClient espClient;
PubSubClient client(espClient);

// Connect to Wi-Fi
void setup_wifi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Reconnect to MQTT broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266_Client")) {  // Unique client ID
      Serial.println(" Connected to MQTT broker.");
    } else {
      Serial.print(" Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Trying again in 5 seconds...");
      delay(5000);
    }
  }
}

// Callback for MQTT messages (not used here)
void callback(char* topic, byte* payload, unsigned int length) {}

// Initialize components
void setup() {
  Serial.begin(115200);
  gpsSerial.begin(GPS_BAUD);
  scale.begin(LoadCellDoutPin, LoadCellSckPin);
  scale.set_scale();  // Set scale factor (calibrate as needed)
  scale.tare();       // Reset to 0
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

// Read GPS data and format as JSON
String getGPSData() {
  String gpsData = "";
  if (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        gpsData += "\"latitude\":" + String(gps.location.lat(), 6);
        gpsData += ",\"longitude\":" + String(gps.location.lng(), 6);
      } else {
        gpsData += "\"latitude\":\"INVALID\",\"longitude\":\"INVALID\"";
      }
    }
  }
  return gpsData;
}

// Read load cell data
String getWeightData() {
  String weightData = "";
  if (scale.is_ready()) {
    long weight = scale.get_units(10);  // Average of 10 readings
    weightData += "\"weight_kg\":" + String(weight);
  } else {
    weightData += "\"weight_kg\":\"ERROR\"";
  }
  return weightData;
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  String gpsData = getGPSData();
  String weightData = getWeightData();
  String payload = "{" + gpsData + "," + weightData + "}";
  Serial.println("Payload: " + payload);

  client.publish(mqtt_topic, payload.c_str());
  delay(2000);
}
