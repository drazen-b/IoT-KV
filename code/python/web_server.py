import time
from flask import Flask, render_template
from flask_socketio import SocketIO
import paho.mqtt.client as mqtt
import mysql.connector
import threading

app = Flask(__name__)
socketio = SocketIO(app)

# MQTT Settings
MQTT_BROKER = "192.168.0.105"
MQTT_PORT = 1883
MQTT_TOPICS = ["zendra/test/light_level", "zendra/test/is_raining", "zendra/test/proximity"]

# MySQL Settings
MYSQL_HOST = "172.28.1.2"
MYSQL_USER = "mqtt"
MYSQL_PASSWORD = "mqtt"
MYSQL_DB = "mqtt_database"

# Data store
data_store = {
    "zendra/test/light_level": None,
    "zendra/test/is_raining": None,
    "zendra/test/proximity": None
}

# Store last message and timestamp
last_message = {
    topic: {"value": None, "timestamp": 0} for topic in MQTT_TOPICS
}

# Time threshold in seconds to ignore duplicate messages
TIME_THRESHOLD = 1

# Mapping of MQTT topics to their respective data types
TOPIC_DATA_TYPES = {
    "zendra/test/light_level": int,
    "zendra/test/is_raining": bool,
    "zendra/test/proximity": float,
}

def save_to_db(topic, value):
    try:
        connection = mysql.connector.connect(
            host=MYSQL_HOST,
            user=MYSQL_USER,
            password=MYSQL_PASSWORD,
            database=MYSQL_DB
        )
        cursor = connection.cursor()
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

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    for topic in MQTT_TOPICS:
        client.subscribe(topic)

def on_message(client, userdata, msg):
    print(f"Message received on topic {msg.topic}: {msg.payload}")
    data_type = TOPIC_DATA_TYPES.get(msg.topic)
    if data_type:
        try:
            payload = msg.payload.decode().lower()
            if data_type == bool:
                value = payload == 'true'
            else:
                value = data_type(payload)

            current_time = time.time()
            last_msg = last_message[msg.topic]
            if last_msg["value"] != value or current_time - last_msg["timestamp"] > TIME_THRESHOLD:
                save_to_db(msg.topic, value)
                last_msg["value"] = value
                last_msg["timestamp"] = current_time

                data_store[msg.topic] = value
                socketio.emit('mqtt_update', {msg.topic: value})
        except ValueError:
            print(f"Failed to convert message to {data_type} on topic {msg.topic}")
            
def run_mqtt():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    client.loop_forever()

@app.route('/')
def index():
    return render_template('index.html', data=data_store)

if __name__ == '__main__':
    threading.Thread(target=run_mqtt, daemon=True).start()
    socketio.run(app, debug=True, host='0.0.0.0', port=5000)