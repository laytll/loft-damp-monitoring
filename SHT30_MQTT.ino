#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "SHT85.h"

#define SHT30_ADDRESS  0x44

const char* ssid = "router";
const char* password = "password";
const char* mqtt_server = "mqtt-broker-ip-address";

WiFiClient espClient;
PubSubClient client(espClient);
SHT85 sht;

char msgBuffer[20];

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

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "loftHum";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("loft/readings", "starting");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  Wire.begin();
  sht.begin(SHT30_ADDRESS);
  Wire.setClock(100000);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  sht.read();
  float    floatTemp = sht.getTemperature();
  float    floatHum = sht.getHumidity();
  
  client.publish("loft/1/temp", dtostrf(floatTemp, 5, 2, msgBuffer));
  delay(100);
  client.publish("loft/1/hum", dtostrf(floatHum, 5, 2, msgBuffer));
  delay(5000);
}
