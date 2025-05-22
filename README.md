# ESP8266 MQTT-Controlled USB Hub

This project is an MQTT-controlled USB hub using an ESP8266 microcontroller. You can remotely enable or disable individual USB ports via MQTT messages. It supports OTA updates and static IP configuration.

---

## Configuration

Before building the project, create a file named `config.h` in the root of the repository with your specific WiFi and MQTT settings.

Example `config.h`:

```cpp
#ifndef CONFIG_H
#define CONFIG_H

// WiFi credentials
#define C_SSID       //"SSID"
#define C_SSPW       //"SSPW"

// MQTT client settings
#define C_CLIENT     "HUB-esp8266"
#define C_TOPIC      "hub/control"

#define C_ADDRESS    150

// OTA password hash
#define C_HASH       "34198f19ea955f43b29c557796064064"

// IP config
#define C_IP_1       //IP
#define C_IP_2       //IP
#define C_IP_3       //IP
#define C_IP_4       //IP

// Gateway config
#define GATEWAY_1    //G_IP
#define GATEWAY_2    //G_IP
#define GATEWAY_3    //G_IP
#define GATEWAY_4    //G_IP

#define SUBNET_1     255
#define SUBNET_2     255
#define SUBNET_3     255
#define SUBNET_4     0

#define C_PORT       1883

#endif
```

---

## Usage

### Start the MQTT broker

If you have `mosquitto` installed, run:

```bash
mosquitto -c ./mosquitto.conf
```

### Compile and Upload

Use `arduino-cli` to compile and upload the sketch:

```bash
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2
arduino-cli upload -p /dev/cu.usbserial-FTB6SPL3 --fqbn esp8266:esp8266:nodemcuv2
```

### Monitor Serial Output

```bash
arduino-cli monitor -p /dev/cu.usbserial-FTB6SPL3 -c 74880
```

---

## Sending MQTT Commands

Use `mosquitto_pub` to control the USB ports:

```bash
mosquitto_pub -h 192.168.1.6 -t hub/control -m "1:1"
```

### Format

```
[port number]:[state]
```

- `1:1` → Opens port 1  
- `3:0` → Closes port 3  
- `0:1` → Opens **all** ports
