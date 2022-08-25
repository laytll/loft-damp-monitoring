#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "SHT85.h"
#include <ArduinoJson.h>

#define SHT85_ADDRESS  0x44

const char* ssid = "Tenda_22A550";
const char* password = "pitch-fuse-how";
const char* mqtt_server = "192.168.0.100";

WiFiClient espClient;
PubSubClient client(espClient);
SHT85 sht;

char out[256]; //MQTT lib needs char not String. would be smart to check char length before storage

unsigned long previousMillis = 0;        // will store last time message was sent
const long interval = 600000;           // interval at which to send message = 5mins

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
    // Create a random client ID
    String clientId = "loftHum";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("loft/status", "1 starting");
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
  sht.begin(SHT85_ADDRESS);
  Wire.setClock(100000);

}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;     // save the last time you sent the message

    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    //get and store sensor values
    sht.read();
    float floatTemp = sht.getTemperature();
    float floatHum = sht.getHumidity();

    //prepare object
    StaticJsonDocument<256> doc;
    doc["type"] = "loft";
    doc["temperature"] = dtostrf(floatTemp, 5, 2, msgBuffer);
    doc["humidity"] = dtostrf(floatHum, 5, 2, msgBuffer);

    //now its json
    serializeJson(doc, out);

    client.publish("loft", out);
    
    Serial.print("Message sent: ");
    Serial.println(out);
    
  }
}
