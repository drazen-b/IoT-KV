#include <Servo.h>

#define LIGHT_SENSOR_PIN A0    // Analog pin for the light sensor
#define CAR_LIGHTS_DIODE 13    // LED pin
#define TRIGGER_PIN 12
#define ECHO_PIN 14
#define BUZZER_PIN 4
#define RAIN_SENSOR_PIN 5
#define SERVO_MOTOR_PIN 16

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
const long servoIntervalMillis = 1000;    // Defines interval length

void setup() {
  Serial.begin(115200);
  pinMode(CAR_LIGHTS_DIODE, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT);
  myservo.attach(SERVO_MOTOR_PIN);
}

void loop() {
  
  executeAfterDelay();
  cleanWindows();  

}

void checkLightLevels() {
  lightLevel = analogRead(LIGHT_SENSOR_PIN); 

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

  if (distance < 50) {
    digitalWrite(BUZZER_PIN, HIGH); // Activate the buzzer
  } else {
    digitalWrite(BUZZER_PIN, LOW); // Deactivate the buzzer
  }

}

void checkRain() {

  isRaining = digitalRead(RAIN_SENSOR_PIN);

  if(isRaining == LOW) {
    Serial.println("It is raining");
  } else {
    Serial.println("No rain detected");
  }

}

void executeAfterDelay() {

  currentTimeMillis = millis();

    if(currentTimeMillis - lastTimeMillis >= intervalMillis) {
      checkLightLevels(); 
      checkcarDistance();
      checkRain();
    }

}

void moveServo(int angle) {
  myservo.write(angle);
}

void cleanWindows() {

  if(!isRaining) {
    servoCurrentTimeMillis = millis();

    if(servoCurrentTimeMillis - servoLastTimeMillis >= servoIntervalMillis) {

      if(!servoMoved) {
        moveServo(180);
        servoMoved = 1;
      }
      else {
        moveServo(0);
        servoMoved = 0;
      }      

    }

  }
  else {
    if(servoMoved) {
      moveServo(0);
      servoMoved = 0;
    }
  }

}