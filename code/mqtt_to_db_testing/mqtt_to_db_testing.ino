#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with your WiFi credentials
const char* ssid = "Bertic";
const char* password = "UEKYRGPF";

// Update with your MQTT broker details
const char* mqtt_server = "192.168.0.105"; // Replace with your MQTT broker address
const int mqtt_port = 1883; // Default MQTT port

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0;
const long interval = 30000; // Interval for sending messages (30 seconds)

void setup() {
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    unsigned long secondsPassed = currentMillis / 1000;

    String payload = "Time passed since power on: " + String(secondsPassed) + " seconds";
    client.publish("zendra/test/time", payload.c_str());
  }

}

void printWiFiNetworks() {

  Serial.println("Scanning for WiFi networks...");
  int numberOfNetworks = WiFi.scanNetworks();

  Serial.println("Number of networks found: " + String(numberOfNetworks));

  // Loop through networks and print their SSID and RSSI (signal strength)
  for (int i = 0; i < numberOfNetworks; i++) {
    Serial.print("Network Name: ");
    Serial.println(WiFi.SSID(i));
    Serial.print("Signal Strength: ");
    Serial.println(WiFi.RSSI(i));
    Serial.println("-----------------------");
  }


}

void setup_wifi() {
  delay(10);
  // Connecting to a WiFi network
  // Serial.println(password);
  Serial.println();
  Serial.println("Connecting to ");
  Serial.println(ssid);

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
    // Attempt to connect without username and password
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

