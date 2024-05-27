#include <Arduino.h>
#include <string>
#include "DHTesp.h"
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "html.h"

WebServer server(80);

// Function declarations
void printdht11(float temp, float humid);
void flashLed();
void printMoist(int moistureValue);
void SoilMoisture();
void HumidityReading();
void TemperatureReading();
void TogglePin2();
void updateButtonStatus();
void setupWifi();
void reconnect();

// Wifi id and password
const char *ssid = "KaZ Home";
const char *password = "**********";

DHTesp dht;
const int ledPin = 4, relayPin = 2;
const int moistureSensor = A0;
int _moisture, moistureValue;
String output2state = "OFF";
boolean pintoggle = false;

void setup()
{
  Serial.begin(115200);

  setupWifi();

  server.on("/", HTTP_GET, []()
            { server.send(200, "text/html", html_page); }); // Serve the htmlpage
  server.on("/togglePin2", HTTP_GET, []()
            {
    TogglePin2();
    server.send(200, "text/html", "Pin 2 toggled");
    updateButtonStatus(); }); // togglePinm 2 and run cpp function
  server.on("/readMoisture", SoilMoisture);
  server.on("/readTemperature", HTTP_GET, TemperatureReading); // update temp
  server.on("/readHumidity", HTTP_GET, HumidityReading);       // update humidity
  server.on("/buttonstatus", HTTP_GET, updateButtonStatus);    // update button status?
  server.begin();
  delay(1000);

  Serial.println();
  Serial.println("Welcome to the Herb-a-Mate");
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");
  String thisBoard = ARDUINO_BOARD;
  Serial.println(thisBoard);

  dht.setup(17, DHTesp::DHT11);
  // set-up pins
  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
}

void loop()
{
  // Retrieve DHT11 info
  delay(dht.getMinimumSamplingPeriod());
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature(); // Serial print Temp and humidity
  printdht11(temperature, humidity);

  moistureValue = analogRead(moistureSensor);
  _moisture = (100 - ((moistureValue / 4095.00) * 100));
  server.handleClient();

  Serial.print("Moisture Value: ");
  Serial.println(_moisture);
  Serial.println("%");

  if (moistureValue > 0)
  {
    printMoist(moistureValue);
  }
  else
  {
    Serial.println("Moisture sensor not in contact with soil.");
  }

  delay(2000);
  flashLed();
}

void updateButtonStatus()
{
  String buttonStatus = pintoggle ? "ON" : "OFF";
  String script = "<script>document.getElementById('PIN2').innerText = 'Turn " + buttonStatus + " PIN2';</script>";
  server.sendContent(script);
}

void TogglePin2()
{
  if (!pintoggle)
  {
    Serial.println("GPIO 2 on");
    output2state = "on";
    digitalWrite(relayPin, HIGH);
    pintoggle = true;
  }
  else if (pintoggle)
  {
    Serial.println("GPIO 2 off");
    output2state = "off";
    digitalWrite(relayPin, LOW);
    pintoggle = false;
  }
}
void SoilMoisture()
{
  String MoistuValue = String(_moisture);
  server.send(200, "text/plane", MoistuValue);
}

void HumidityReading()
{
  String HumidityValue = String(dht.getHumidity());
  server.send(200, "text/plane", HumidityValue);
}

void TemperatureReading()
{
  String TemperatureValue = String(dht.getTemperature());
  server.send(200, "text/plane", TemperatureValue);
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

void flashLed()
{
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  delay(500);
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
