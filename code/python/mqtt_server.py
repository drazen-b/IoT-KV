import paho.mqtt.client as mqtt
import mysql.connector
import time

# MQTT Settings
MQTT_BROKER = "192.168.0.105"
MQTT_PORT = 1883
MQTT_TOPICS = ["zendra/test/light_level", "zendra/test/is_raining", "zendra/test/proximity"]

# MySQL Settings
MYSQL_HOST = "172.28.1.2"
MYSQL_PORT = 3306
MYSQL_USER = "mqtt"
MYSQL_PASSWORD = "mqtt"
MYSQL_DB = "mqtt_database"

# Mapping of MQTT topics to their respective data types
TOPIC_DATA_TYPES = {
    "zendra/test/light_level": int,
    "zendra/test/is_raining": bool,
    "zendra/test/proximity": float,
}

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    for topic in MQTT_TOPICS:
        client.subscribe(topic)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(f"Message received on topic {msg.topic}: {msg.payload}")
    data_type = TOPIC_DATA_TYPES.get(msg.topic)
    if data_type:
        try:
            payload = msg.payload.decode().lower()
            if data_type == bool:
                value = payload == 'true'  # Convert to boolean based on string content
            else:
                value = data_type(payload)
            save_to_db(msg.topic, value)
        except ValueError:
            print(f"Failed to convert message to {data_type} on topic {msg.topic}")

def save_to_db(topic, value):
    try:
        connection = mysql.connector.connect(
            host=MYSQL_HOST,
            user=MYSQL_USER,
            password=MYSQL_PASSWORD,
            database=MYSQL_DB
        )
        cursor = connection.cursor()

        # Extract the last segment of the topic for table name
        table_name = "mqtt_messages_" + topic.split('/')[-1]

        query = f"INSERT INTO {table_name} (message) VALUES (%s)"
        cursor.execute(query, (value,))
        connection.commit()
        print(f"Message saved to {table_name} in the database")
    except mysql.connector.Error as error:
        print(f"Failed to insert into MySQL table: {error}")
    finally:
        if connection.is_connected():
            cursor.close()
            connection.close()

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Loop forever
client.loop_forever()