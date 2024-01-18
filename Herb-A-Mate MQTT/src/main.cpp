#include <Arduino.h>
#include <string>
#include "DHTesp.h"
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Function declarations
void printdht11(float temp, float humid);
void printMoist(int moistureValue);
void TogglePin2();
void setupWifi();
void reconnect();
void relayCall(char *topic, byte *payload, unsigned int length);
void soilReading();

// Wifi id and password
const char *ssid = "KaZ Home";
const char *password = "***********";

// Set MQTT username and password
const char *mqttUser = "mqtt_user";
const char *mqttPass = "*******";
const char *mqtt_server = "192.168.1.129";

WiFiClient espClient;
PubSubClient client(espClient);
const int THRESHOLD = 2842;
DHTesp dht;
const int relayPin = 2;
const int moistureSensor = 34;
int _moisture = 0, moistureValue = 0;
String output2state = "OFF";
boolean pintoggle = false, sensorEngaged = true;

void setup()
{
  Serial.begin(115200);

  setupWifi();

  client.setServer(mqtt_server, 1883);

  Serial.println();
  Serial.println("Welcome to Herb-a-Mate");
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");
  String thisBoard = ARDUINO_BOARD;
  Serial.println(thisBoard);

  dht.setup(17, DHTesp::DHT11);

  // set-up Relay pin
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
}

void loop()
{

  // Connecting loop
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  // DHT11 sample collection
  delay(dht.getMinimumSamplingPeriod());
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  // MQTT topics declaration
  char temp_topic[] = "Comp Room temperature";
  char hum_topic[] = "Comp Room humidity";
  char mist_topic[] = "Moisture value";

  // MQTT push to broker
  client.publish(temp_topic, String(temperature).c_str());
  client.publish(hum_topic, String(humidity).c_str());
  client.publish(mist_topic, String(_moisture).c_str());

  // Print Sensor data to serial
  printdht11(temperature, humidity);

  // soil moisture
soilReading();

  delay(2000);
}
/*
Sensor testing.
float wet = 2168.33;
float half = (2364+2419+2480+2535+2686+2639+2647)/6; 2961.666667
float three_quarters = (2852+2883+2919+2945+2977+2950+3058)/6; 3430.667
float air = 4095;
4095 - 2168 = 1927 * .65 = 1252.55. 4095 - 1252.55 = 2842
Threshold = 2842 == sweet spot
Avergae of all the combined measurements = 3163.9
*/
void soilReading()
{
  moistureValue = analogRead(moistureSensor);
  _moisture = (100 - ((moistureValue / 4095.00) * 100));
  Serial.print("Moisture Value: ");
  Serial.print(_moisture);
  Serial.println("%");
  Serial.println(moistureValue);

  if (moistureValue > 300)
  {
    printMoist(moistureValue);
  }
  else
  {
    Serial.println("Moisture sensor not in contact with soil.");
    sensorEngaged = false;
  }

  // Toggle relay if mositure level uis lower than 65%
  if (moistureValue > THRESHOLD && !pintoggle)

  {
    TogglePin2();
    Serial.println("Moisture levels low, pump engaged");
  }
  else if (moistureValue < THRESHOLD && pintoggle)
  {
    TogglePin2();
    Serial.println("Moisture levels are great");
  }
}
void TogglePin2()
{
  if (!pintoggle)
  {
    Serial.println("GPIO 2 on");
    output2state = "on";
    digitalWrite(relayPin, HIGH);
    pintoggle = true;
    client.publish("home/pump/state", output2state.c_str());
  }
  else if (pintoggle)
  {
    Serial.println("GPIO 2 off");
    output2state = "off";
    digitalWrite(relayPin, LOW);
    pintoggle = false;
    client.publish("home/pump/state", output2state.c_str());
  }
}

void printdht11(float temperature, float humidity)
{
  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");
  Serial.print(dht.toFahrenheit(temperature), 1);
  Serial.print("\t\t");
  Serial.print(dht.computeHeatIndex(temperature, humidity, false), 1);
  Serial.print("\t\t");
  Serial.println(dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true), 1);
  delay(2000);
}

void printMoist(int moistureValue)
{
  Serial.print("Moisture Level: ");
  Serial.println(moistureValue);
}

void setupWifi()
{

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("Your Local IP address is: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    const char *mqttUser = "mqtt_user";
    const char *mqttPass = "hello22";
    if (client.connect("ESP32Client", mqttUser, mqttPass))
    {
      Serial.println("connected");
      client.subscribe("home/pump/set");
      client.setCallback(relayCall);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void relayCall(char *topic, byte *payload, unsigned int length)
{
  String topicStr = String((char *)topic);
  if (topicStr == "home/pump/set")
  {
    String payloadStr = "";
    for (int i = 0; i < length; i++)
    {
      payloadStr += (char)payload[i];
    }
    Serial.print("Received payload: ");
    Serial.println(payloadStr);

    if (payloadStr == "ON")
    {
      pintoggle = false;
      TogglePin2();
      Serial.println("Pin 2 ON");
    }
    if (payloadStr == "OFF")
    {
      pintoggle = true;
      TogglePin2();
      Serial.println("Pin 2 OFF");
    }
  }
}
