/************************************************************/
/*                                                          */
/************************************************************/

// increment on change
#define SOFTWARE_VERSION "20161109_01"

#include "config.h"

/**********************************************/
/* WiFi declarations
  /**********************************************/
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>

ESP8266WiFiMulti wifiMulti;
WiFiClient wifiClient;

PubSubClient mqttClient(wifiClient, mqtt_broker);

/**********************************************/
/* The Setup
  /**********************************************/
void setup() {
  Serial.begin(9600); //Output to Serial at 9600 baud
  delay(10);

  // Mac address
  WiFi.macAddress(mac);
  byteToHexString(strMac, mac, 6, ":");

  Serial.println("");
  Serial.println("## MuMaLab Space Status ##");
  Serial.print("Software version: ");
  Serial.println(SOFTWARE_VERSION);
  Serial.print("Device Identifier (MAC): ");
  Serial.println(strMac);

  WIFI_MULTI_APS

  connectWifi(); // Start ConnecWifi
}

/**********************************************/
/* And action
  /**********************************************/
void loop() {
  if (wifiMulti.run() != WL_CONNECTED) {
    connectWifi();
  }
  if (mqttClient.connected() != 1) {
    connectMqtt();
  }

  if (nextCheck <= millis() ) {
    nextCheck = millis() + 3 * 1000;
    State = digitalRead(buttonPin);
    if ( State != StateOld ) {
      mqttClient.publish(MQTT::Publish( topic, String(State) )
                         .set_retain()
                         .set_qos(0)
                         .set_dup()
                        );
      StateOld = State;
      Serial.println("State published " + String(State) );
    }
  }
  mqttClient.loop();
}

/**********************************************/
/* WiFi connecting script
  /**********************************************/
void connectWifi() {
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting ");
  while (wifiMulti.run() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("WiFi connected to %s\n", WiFi.SSID().c_str());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
void connectMqtt() {
  while (mqttClient.connected() != 1) {
    if ( WiFi.status() == WL_CONNECTED ) {
      mqttClient.connect(MQTT::Connect( strMac )
                         .set_clean_session()
                         .set_will( topic, "unknown", 0, 1 ) // 0,1 = QOS(0), retain=(1)
                         .set_auth(mqtt_user, mqtt_pass)
                         .set_keepalive(30)
                        );
      delay(100);

      if ( mqttClient.connected() ) {
        Serial.println("MQTT connected to " + String(mqtt_broker) + ":" + String(mqtt_port) );
        delay(200);
        StateOld = -1;
        return;
      }
    }
  }
}

void byteToHexString(String &dataString, byte *uidBuffer, byte bufferSize, String strSeperator) {
  dataString = "";
  for (byte i = 0; i < bufferSize; i++) {
    if (i > 0) {
      dataString += strSeperator;
      if (uidBuffer[i] < 0x10)
        dataString += String("0");
    }
    dataString += String(uidBuffer[i], HEX);
  }
  dataString.toUpperCase();
}


