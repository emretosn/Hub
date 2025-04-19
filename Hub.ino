#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "config.h"

#define PORT1 5  // D1 = GPIO05
#define PORT2 4  // D2 = GPIO04
#define PORT3 14 // D5 = GPIO14
#define PORT4 12 // D6 = GPIO12
#define PORT5 13 // D7 = GPIO13

const bool DEBUG = true;

WiFiClient clientWifi;
PubSubClient clientPubSub(clientWifi);
IPAddress ip(C_IP_1, C_IP_2, C_IP_3, C_ADDRESS);
IPAddress gateway(GATEWAY_1, GATEWAY_2, GATEWAY_3, GATEWAY_4);
IPAddress subnet(SUBNET_1, SUBNET_2, SUBNET_3, SUBNET_4);
unsigned long millis_previous = 0L;
unsigned long millis_current = 0L;

void rx_usbhub(int _port, int _state) {
  switch (_port) {
    case 1: {
      digitalWrite(PORT1, _state);
      if (DEBUG) {
        Serial.print("PORT digitalWrite");
      }
      break;
    }
    case 2: {
      digitalWrite(PORT2, _state);
      break;
    }
    case 3: {
      digitalWrite(PORT3, _state);
      break;
    }
    case 4: {
      digitalWrite(PORT4, _state);
      break;
    }
    case 5: {
      digitalWrite(PORT5, _state);
      break;
    }
    // '0' is used to turn all ports on/off
    default: {
        digitalWrite(PORT1, _state);
        digitalWrite(PORT2, _state);
        digitalWrite(PORT3, _state);
        digitalWrite(PORT4, _state);
        digitalWrite(PORT5, _state);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (DEBUG) {Serial.print("callback(): "); Serial.println(topic);}

  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  if (DEBUG) {
    Serial.print("Payload received: ");
    Serial.println(msg);
  }

  // Expecting format: "<device_number>:<state>"
  int sepIndex = msg.indexOf(':');
  if (sepIndex > 0) {
    int port = msg.substring(0, sepIndex).toInt();
    int state = msg.substring(sepIndex + 1).toInt();

    if (DEBUG) {
      Serial.print("Parsed port: ");
      Serial.print(port);
      Serial.print(", state: ");
      Serial.println(state);
    }

    if (port >= 0 && port <= 5 && (state == 0 || state == 1)) {
        rx_usbhub(port, state);
    } else {
        if (DEBUG) Serial.println("Invalid port or state");
    }

  } else {
    if (DEBUG) {
      Serial.println("Invalid payload format");
    }
  }
}

void setup() {
  if (DEBUG) Serial.begin(74880);

  pinMode(PORT1, OUTPUT);
  digitalWrite(PORT1, LOW); // OFF
  pinMode(PORT2, OUTPUT);
  digitalWrite(PORT2, LOW); // OFF
  pinMode(PORT3, OUTPUT);
  digitalWrite(PORT3, LOW); // OFF
  pinMode(PORT4, OUTPUT);
  digitalWrite(PORT4, LOW); // OFF
  pinMode(PORT5, OUTPUT);
  digitalWrite(PORT5, LOW); // OFF

  // WIFI
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(C_SSID, C_SSPW);

  // OTA
  ArduinoOTA.setPasswordHash(C_HASH);
  ArduinoOTA.onStart([]() {
    if (DEBUG) Serial.println("OTA.onStart()");
  });
  ArduinoOTA.onEnd([]() {
    if (DEBUG) Serial.println("OTA.onEnd()");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if (DEBUG) Serial.printf("OTA.onProgress(): %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    if (DEBUG) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();

  // MQTT
  clientPubSub.setServer(IPAddress(C_IP_1, C_IP_2, C_IP_3, C_IP_4), C_PORT);
  clientPubSub.setCallback(callback);

  // DEBUG
  if (DEBUG) {
    Serial.println("");
    Serial.print("DEBUG: ");
    Serial.print(WiFi.localIP());
    Serial.print(" / ");
    Serial.println(String(WiFi.macAddress()));
  }
}

void handlePubSub() {
  clientPubSub.connect(C_CLIENT);
  delay(1000);
  if (clientPubSub.connected()) {
    if (DEBUG) Serial.println("handlePubSub(): connected to MQTT broker");
    clientPubSub.subscribe(C_TOPIC, 1);
    if (DEBUG) Serial.println("handlePubSub(): subscribed to topic");
  }
}

void handleWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!clientPubSub.connected()) {
      handlePubSub();
    }
  } else {
    WiFi.reconnect();
    if (WiFi.waitForConnectResult(5000) == WL_CONNECTED) {
      handlePubSub();
    } else {
      if (DEBUG) Serial.println("handleWiFi(): unable to (re)connect to WiFi");
    }
  }
}

void loop() {
  millis_current = millis();

  if (millis_current - millis_previous < 0) {
    millis_previous = millis_current;
    handleWiFi();
  } else if (millis_current - millis_previous >= 5000L) {
    millis_previous = millis_current;
    handleWiFi();
  }

  if (WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle();

    clientPubSub.loop();
  }
}

