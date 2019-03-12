#include <ArduinoJson.h>
#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <WiFi101.h>

#include "arduino_secrets.h"

/////// Pinmappings
const int valve0Pin = 0;
const int valve1Pin = 1;
const int valve2Pin = 2;

/////// Enter your sensitive data in arduino_secrets.h
const char ssid[]        = SECRET_SSID;
const char pass[]        = SECRET_PASS;
const char broker[]      = SECRET_BROKER;
const char* certificate  = SECRET_CERTIFICATE;

WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
MqttClient    mqttClient(sslClient);

unsigned long lastMillis = 0;
bool requestRecipes = true;
bool recvRdy = false;
String receipts[255];
int recieptCount = 0;
String receivedMessage = "";
//struct Drink drinks[4];

StaticJsonDocument<200> doc;
DeserializationError dsError;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1);
  }

  // Set a callback to get the current time
  // used to validate the servers certificate
  ArduinoBearSSL.onGetTime(getTime);

  // Set the ECCX08 slot to use for the private key
  // and the accompanying public certificate for it
  sslClient.setEccSlot(0, certificate);

  // Optional, set the client id used for MQTT,
  // each device that is connected to the broker
  // must have a unique client id. The MQTTClient will generate
  // a client id for you based on the millis() value if not set
  //
  // mqttClient.setId("clientId");

  // Set the message callback, this function is
  // called when the MQTTClient receives a message
  mqttClient.onMessage(onMessageReceived);

  // parse recipes
  /*
  dsError = deserializeJson(doc, recipes);
  if(dsError){
    Serial.println("Recipe deserialization error!");
  }else{
    JsonObject jsonobj = doc.as<JsonObject>();
    int i = 0;
    for(JsonPair p : jsonobj){
      struct Drink drnk;
      drnk.id = p.key().c_str();
      drnk.valve0 = p.value()["valve0"];
      drnk.valve1 = p.value()["valve1"];
      drnk.valve2 = p.value()["valve2"];
      drinks[i] = drnk;
      i++;
    }
    recieptCount = i;
  }*/
  pinMode(valve0Pin, OUTPUT);
  pinMode(valve1Pin, OUTPUT);
  pinMode(valve2Pin, OUTPUT);

  Serial.println("Recipes parsed");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    // MQTT client is disconnected, connect
    connectMQTT();
  }

  // poll for new MQTT messages and send keep alives
  mqttClient.poll();
  // handle booze order from AWS-IoT
  if(recvRdy){
    Serial.println("Recv OK");
    recvRdy = false;
    dsError = deserializeJson(doc, receivedMessage)  ;
    if(dsError){
      Serial.println("Parse error");
    }else{
      String drink;
      drink = doc["drink"].as<String>();
      Serial.println("Making drink " + drink);
      // search drink from reciepts
      if(drink.equals("rumandcoke")){
        Serial.println("Rum and coke");
        getBooze(valve2Pin, 5);
        getBooze(valve1Pin, 5);
      }else if(drink.equals("maitojailla")){
        Serial.println("Maitoo jäillä");
        getBooze(valve0Pin, 5);
        getBooze(valve1Pin, 5);
      }else if(drink.equals("gintonic")){
        Serial.println("Gin tonicci");
        getBooze(valve2Pin, 2);
        getBooze(valve1Pin, 1);
      }else{
        Serial.println("failed");
      }
    }
  }
}

unsigned long getTime() {
  // get the current time from the WiFi module
  return WiFi.getTime();
}

void connectWiFi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.print(ssid);
  Serial.print(" ");

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the network");
  Serial.println();
}

void connectMQTT() {
  Serial.print("Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  while (!mqttClient.connect(broker, 8883)) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe("ibartender/order");
}

void publishMessage(String payload, String topic) {
  Serial.println("Publishing message");
  Serial.println(topic + ':' + payload);
  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage(topic);
  mqttClient.print(payload);
  mqttClient.endMessage();
}

void onMessageReceived(int messageSize) {
  // we received a message, print out the topic and contents
  receivedMessage = "";
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    char chr = (char)mqttClient.read();
    receivedMessage = receivedMessage + chr;
    Serial.print(chr);
  }
  recvRdy = true;
  Serial.println();

  Serial.println();
}

void getBooze(int valve, int timeInSec){
  digitalWrite(valve, HIGH);
  delay(1000 * timeInSec);
  digitalWrite(valve, LOW);
}
