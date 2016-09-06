/*
  This sketch is based on the basic MQTT example by
  http://knolleary.github.io/pubsubclient/
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include "DHT.h"

#define ON 1
#define OFF 0
#define LAMPADA 3

#define DEBUG 1 // Debug output to serial console
#define DHTPIN 2 

// Device settings
IPAddress deviceIp(192, 168, 1, 43);
byte deviceMac[] = { 0xAB, 0xCD, 0xFE, 0xFE, 0xFE, 0xFE };
char* deviceId  = "sensor01"; // Name of the sensor
char* stateTopic = "home-assistant/sensor01/brightness"; // MQTT topic where values are published
char* stateTopic2 = "home-assistant/sensor01/temperatura"; // MQTT topic where values are published
char* stateTopic3 = "home-assistant/sensor01/umidade"; // MQTT topic where values are published
char* stateTopic4 = "home-assistant/sensor01/lampada"; // MQTT topic where values are published
char* stateTopic5 = "home-assistant/sensor01/lampada/command"; // MQTT topic where values are publishe
int sensorPin = A0; // Pin to which the sensor is connected to
char buf[4]; // Buffer to store the sensor value
int updateInterval = 500; // Interval in miliseconds
char message_buff[100];
volatile unsigned long ul_PreviousMillis = 0;

// MQTT server settings
IPAddress mqttServer(192, 168, 1, 1);
int mqttPort = 1883;

EthernetClient ethClient;
PubSubClient client(ethClient);

DHT dht;

void reconnect() {
  while (!client.connected()) {
#if DEBUG
    Serial.print("Attempting MQTT connection...");
#endif
    if (client.connect(deviceId, "ID", "Passwordr") && client.subscribe("home-assistant/sensor01")) {
#if DEBUG
      Serial.println("connected");
#endif
    } else {
#if DEBUG
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
#endif
      delay(5000);
    }
  }
}

void callback(char* topic, byte* p_payload, unsigned int length) {
  String payload;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)p_payload[i]);
    payload.concat((char)p_payload[i]);
  }
  if (String(stateTopic5).equals(topic)) {
      if (payload.equals("ON")) {
        digitalWrite(LAMPADA, ON);
      } else if (payload.equals("OFF")) {
        digitalWrite(LAMPADA, OFF);
      }
  }
  Serial.println();
}

void setup() {
  pinMode(LAMPADA, OUTPUT);
  Serial.begin(57600);
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  Ethernet.begin(deviceMac, deviceIp);
  delay(1500);
  dht.setup(DHTPIN); // data pin 2
}

void loop() {
  unsigned long ul_CurrentMillis = millis(); 
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.getHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.getTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  
  int sensorValue = analogRead(sensorPin);
#if DEBUG
  char* state = digitalRead(LAMPADA) ? state = "ON" : state = "OFF";
#endif
  if( ul_CurrentMillis - ul_PreviousMillis > updateInterval || ul_CurrentMillis - ul_PreviousMillis > 0)
  {
      //     Set the previous time to the current time.
      //     A delay by code will shift everyting further in time.
      ul_PreviousMillis = ul_CurrentMillis;
    client.publish(stateTopic, itoa(sensorValue, buf, 10));
    client.publish(stateTopic2, itoa(t, buf, 10));
    client.publish(stateTopic3, itoa(h, buf, 10));
    client.publish(stateTopic4, state);
  }
}
