# IoT-KV

## Pinout

![](./images/schematic.png)

## Nacin rada sklopovlja


Sustav se sastoji od:
- Croduino Nova2 - ESP8266 razvojna plocica
    - povezana sa senzorima, te metrike salje na Raspberry Pi 3 putem MQTT i WiFi-ja
- Raspberry Pi 3
    - subscribean je na MQTT topic i slusa podatke koje mu ESP publisha, spojen je na istu mrezu kao i ESP, ali zicom
    - hosta webserver i mariadb bazu podataka
- fotootpornik - senzor svjetlosti
- YL-83 - senzor kise
- servo motor - imitira funkciju brisaca
- buzzer - kao zvucno upozorenje
- LED - kao vizualna obavijest
- senzor udaljenosti

### Croduino NOVA 2

Na njoj se vrti Arduino skripta koja prikuplja metrike sa senzora. Plocica je spojena na Wifi sto je prikazano u sljedecem bloku koda.

```c
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
```

U ovom bloku koda publishanmo podatke na MQTT topic (3 topica):

```c
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
  ```

  ### Raspberry Pi

  Na njemu se vrti MQTT broker, konkretno `mosquitto`. Te se na njemu nalazi MariaDB baza podataka. 

  Baza podataka je definirana na ovaj nacin:

  ```sql
  CREATE DATABASE IF NOT EXISTS mqtt_database;

USE mqtt_database;

CREATE TABLE IF NOT EXISTS mqtt_messages_light_level (
    id INT AUTO_INCREMENT PRIMARY KEY,
    message INT NOT NULL,
    received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS mqtt_messages_is_raining (
    id INT AUTO_INCREMENT PRIMARY KEY,
    message BOOL NOT NULL,
    received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS mqtt_messages_proximity (
    id INT AUTO_INCREMENT PRIMARY KEY,
    message FLOAT NOT NULL,
    received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE USER 'exporter'@'localhost' IDENTIFIED BY 'exporter' WITH MAX_USER_CONNECTIONS 3;
GRANT PROCESS, REPLICATION CLIENT, SELECT ON *.* TO 'exporter'@'localhost';

CREATE USER 'mqtt'@'%' IDENTIFIED BY 'mqtt';
GRANT ALL PRIVILEGES ON mqtt_database.* TO 'mqtt'@'%';
FLUSH PRIVILEGES;
```

Takodjer je pokrenuta `web_server.py` python skripta koja obavlja dvije funkcije.

1. Subscribe na MQTT topice koje slusa te zapisuje u bazu podataka i salje na webserver.

2. Hosta webserver.

