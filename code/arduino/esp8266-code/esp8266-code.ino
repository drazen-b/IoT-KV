#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

#define LIGHT_SENSOR_PIN A0    // Analog pin for the light sensor
#define CAR_LIGHTS_DIODE 13    // LED pin
#define TRIGGER_PIN 12
#define ECHO_PIN 14
#define BUZZER_PIN 15
#define RAIN_SENSOR_PIN 5
#define SERVO_MOTOR_PIN 16

// Network and MQTT setup

// Update these with your WiFi credentials
const char* ssid = "Bertic";
const char* password = "UEKYRGPF";

// Update with your MQTT broker details
const char* mqtt_server = "192.168.0.105"; // Replace with your MQTT broker address
const int mqtt_port = 1883; // Default MQTT port

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0;
const long interval = 1000; // Interval for sending messages (30 seconds)

// Sensors setup

Servo myservo;

int lightLevel;
float duration, distance;
bool isRaining;
bool servoMoved;

unsigned int lastTimeMillis = 0;    // Stores last time when HC-SR04 was triggered
unsigned int currentTimeMillis = 0; // Stores current time
const long intervalMillis = 500;    // Defines interval length


unsigned int servoLastTimeMillis = 0;    // Stores last time when HC-SR04 was triggered
unsigned int servoCurrentTimeMillis = 0; // Stores current time
const long servoIntervalMillis = 2000;    // Defines interval length

void setup() {
  Serial.begin(115200);

  // Setup Pins
  pinMode(CAR_LIGHTS_DIODE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT);
  myservo.attach(SERVO_MOTOR_PIN);

  // Setup WiFi and MQTT
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  executeAfterDelay();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    unsigned long secondsPassed = currentMillis / 1000;

    String payload = "Time passed since power on: " + String(secondsPassed) + " seconds";
    client.publish("zendra/test/time", payload.c_str());
    client.publish("zendra/test/is_raining", String(isRaining).c_str());
    client.publish("zendra/test/light_level", String(lightLevel).c_str());
    client.publish("zendra/test/proximity", String(distance).c_str());
  }


  cleanWindows();  

}

void checkLightLevels() {
  lightLevel = analogRead(LIGHT_SENSOR_PIN); 
  Serial.print("Light: ");
  Serial.println(lightLevel);

  if(lightLevel < 500) {
    digitalWrite(CAR_LIGHTS_DIODE, HIGH); 
  } else {
    digitalWrite(CAR_LIGHTS_DIODE, LOW);  
  }
}

void checkcarDistance() {
  
  // Clear the trigger pin
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  // Set the trigger pin high for 10 microseconds
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  // Read the duration of the echo pulse
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)

  // Print the distance to the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance < 50.0) {
    digitalWrite(BUZZER_PIN, HIGH); // Activate the buzzer
  } else {
    digitalWrite(BUZZER_PIN, LOW); // Deactivate the buzzer
  }

}

void checkRain() {

  isRaining = digitalRead(RAIN_SENSOR_PIN);

  if(isRaining == LOW) {
    Serial.println("Rain: True");
  } else {
    Serial.println("Rain: False");
  }

}

void executeAfterDelay() {

  currentTimeMillis = millis();

    if(currentTimeMillis - lastTimeMillis >= intervalMillis) {
      checkLightLevels(); 
      checkcarDistance();
      checkRain();
      lastTimeMillis = currentTimeMillis;
    }

}

void moveServo(int angle) {
  myservo.write(angle);
}

void cleanWindows() {
  if (!isRaining) {
    servoCurrentTimeMillis = millis();
    if (servoCurrentTimeMillis - servoLastTimeMillis >= servoIntervalMillis) {
      if (!servoMoved) {
        moveServo(180);
        servoMoved = true;
      } else {
        moveServo(0);
        servoMoved = false;
      }
      servoLastTimeMillis = servoCurrentTimeMillis; // Update the servoLastTimeMillis
    }
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
