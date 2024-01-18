To send data from an Arduino to a Raspberry Pi over USB and then save that data into a MySQL database hosted on the Raspberry Pi, you can follow these steps:

### Arduino Side

First, program the Arduino to send data through its USB connection, which the Raspberry Pi will recognize as a serial connection.

#### Arduino Sketch Example:

Here's a basic example where the Arduino sends some dummy data over USB:

```cpp
void setup() {
  Serial.begin(9600);  // Start the serial communication
}

void loop() {
  // Example data
  int lightLevel = analogRead(A0);  // Read analog value from a sensor connected to A0
  float duration = 123.45;
  float distance = 10.2;
  bool isRaining = true;
  bool servoMoved = false;

  // Sending data as a comma-separated string
  Serial.print(lightLevel);
  Serial.print(",");
  Serial.print(duration);
  Serial.print(",");
  Serial.print(distance);
  Serial.print(",");
  Serial.print(isRaining);
  Serial.print(",");
  Serial.println(servoMoved);

  delay(2000); // Send data every 2 seconds
}
```

### Raspberry Pi Side

On the Raspberry Pi, you need to write a Python script that reads this serial data and inserts it into a MySQL database.

#### Setup MySQL Database

Make sure MySQL is installed on the Raspberry Pi, and create a database and a table to store your data:

```sql
CREATE DATABASE example_db;
USE example_db;

CREATE TABLE sensor_data (
    id INT AUTO_INCREMENT PRIMARY KEY,
    lightLevel INT,
    duration FLOAT,
    distance FLOAT,
    isRaining BOOLEAN,
    servoMoved BOOLEAN,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

#### Python Script

1. **Install Required Libraries**: You'll need `pyserial` for serial communication and `mysql-connector-python` for database interaction.

    ```bash
    pip3 install pyserial mysql-connector-python
    ```

2. **Python Script to Read Serial Data and Insert into MySQL**:

    ```python
    import serial
    import mysql.connector
    from mysql.connector import Error

    def insert_into_database(data):
        try:
            conn = mysql.connector.connect(host='localhost', database='example_db', user='your_user', password='your_password')
            if conn.is_connected():
                cursor = conn.cursor()
                query = """INSERT INTO sensor_data (lightLevel, duration, distance, isRaining, servoMoved) 
                           VALUES (%s, %s, %s, %s, %s);"""
                cursor.execute(query, data)
                conn.commit()
                cursor.close()
                conn.close()
        except Error as e:
            print("Error while connecting to MySQL", e)

    def main():
        ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
        ser.flush()

        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8').rstrip()
                parts = line.split(',')

                if len(parts) == 5:
                    data = (int(parts[0]), float(parts[1]), float(parts[2]), parts[3] == '1', parts[4] == '1')
                    insert_into_database(data)

    if __name__ == '__main__':
        main()
    ```

    Replace `'localhost'`, `'example_db'`, `'your_user'`, and `'your_password'` with your actual MySQL server details. Adjust the serial port (`/dev/ttyACM0`) as per your setup.

### Running the Setup

1. **Upload the Arduino Sketch** to the Arduino.
2. **Run the Python Script** on the Raspberry Pi.

Now, the Arduino will send data over USB every 2 seconds, the Raspberry Pi will read this data from its serial port, and then the Python script will insert the data into the MySQL database.

### Notes

- Make sure the Raspberry Pi and Arduino are correctly connected via USB.
- The serial port name (`/dev/ttyACM0`) might vary. Check the available ports on the Raspberry Pi with `ls /dev/tty*`.
- In the Python script, the data is parsed based on the expected format sent from the Arduino. Ensure that this format is maintained consistently.