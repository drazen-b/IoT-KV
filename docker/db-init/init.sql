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