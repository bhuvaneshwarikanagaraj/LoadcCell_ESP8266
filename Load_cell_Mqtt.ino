#include <ESP8266WiFi.h>
#include "HX711.h"
#include <Wire.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

HX711 scale;

float weight;
float calibration_factor = 115620;

const char* ssid = "mqtt";
const char* password = "mosquitto";
const char* mqtt_server = "192.168.43.205";

// Initialize wifi & PubSub client
WiFiClient espClient;
PubSubClient client(espClient);



void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("senseclient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(1000);
    }
  }
}


void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  weight_func();
}
void weight_func(){
  scale.begin(D5, D6);
  scale.set_scale();
  scale.tare();
  long zero_factor = scale.read_average();
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(5000);
  DynamicJsonDocument doc(1024);
  JsonObject obj = doc.as<JsonObject>();
  scale.set_scale(calibration_factor);
  weight = scale.get_units(5);
  doc["weight"] = weight;
  char jsonStr[60];
  serializeJson(doc, jsonStr);
  Serial.print(jsonStr);
  Serial.println();
  Serial.print(weight);
  client.publish("testing", jsonStr);
}
