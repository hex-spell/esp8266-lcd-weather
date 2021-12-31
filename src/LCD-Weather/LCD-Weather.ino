#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoWiFiServer.h>
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiGratuitous.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiClient.h>
#include <string>
#define ARDUINOJSON_USE_DOUBLE 0

// Define lcd dimentions
#define LCD_DISPLAY_ROWS 20
#define LCD_DISPLAY_COLS 4

// Define wifi credentials
const char* ssid = "wifi_name";
const char* password = "wifi_password";

// Define LCD pinout
const int  en = 2, rw = 1, rs = 0, d4 = 4, d5 = 5, d6 = 6, d7 = 7, bl = 3;

// Define I2C Address - change if reqiuired
const int i2c_addr = 0x27;

// Define switch pin
int PWMPin = 5;

// Define labels
const char* connecting = "Conectando a la red:";
const char* loading = "Cargando clima...";
const char* temperatureLabel = "Temperatura";
const char* pressureLabel = "Presion";
const char* HumidityLabel = "Humedad";

// Define api endpoint
const char* apiEndpoint = "https://node-weather-challenge.herokuapp.com/weather?q=necochea&appid=df23c001-5989-4fae-8f2a-505bfdb61b34";

LiquidCrystal_I2C lcd(i2c_addr, en, rw, rs, d4, d5, d6, d7, bl, POSITIVE);
WiFiClientSecure wifiClient;
boolean programRunning;

void printLn(const char* str, int line) {
  lcd.setCursor(0, line);
  for (int i = 0; i < strlen(str); i++) {
    if (i != 0 && i % 20 == 0) {
      lcd.setCursor(0, ++line);
    }
    lcd.print(str[i]);
  }
}

void printLn(const float flt, int line) {
  lcd.setCursor(0, line);
  lcd.print(flt);
}

void printLabeledFloatLn(const char* label, const float value, const char* unit, int line) {
  lcd.setCursor(0, line);
  lcd.print(label);
  lcd.print(": ");
  lcd.print((int)value);
  lcd.print(unit);
}

void printConnection() {
  lcd.setCursor(0, 0);
  lcd.print(connecting);
  const char* connection = ssid;
  printLn(connection, 1);
  lcd.setCursor(0, 3);
  int limit = 20;
  int xCounter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (xCounter < limit) {
      lcd.print(".");
    }
    delay(400);
  }
  lcd.clear();
}

void startProgram() {
  programRunning = true;
  lcd.on();
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(loading);

  if (WiFi.status() == WL_CONNECTED) {
    wifiClient.setInsecure();
    HTTPClient http;
    http.begin(wifiClient, apiEndpoint);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      StaticJsonDocument<1024> doc;
      DeserializationError err = deserializeJson(doc, payload);

      if (!err) {
        const char* name = doc["name"];
        const float temp = doc["main"]["temp"];
        const float pressure = doc["main"]["pressure"];
        const float humidity = doc["main"]["humidity"];
        lcd.clear();
        printLn(name, 0);
        // kelvin to celsius
        printLabeledFloatLn(temperatureLabel, temp  - 273.15, "C", 1);
        printLabeledFloatLn(pressureLabel, pressure, "Psi", 2);
        printLabeledFloatLn(HumidityLabel, humidity, "%", 3);
      }
      else {
        Serial.println(err.c_str());
        lcd.print("error");
      }
      Serial.println(payload);
    }

    http.end();   //Close connection
  }
}
void setup()
{
  Serial.begin(9600);
  lcd.begin(LCD_DISPLAY_ROWS, LCD_DISPLAY_COLS);
  WiFi.begin(ssid, password);
  printConnection();
  pinMode(D5, OUTPUT);
  startProgram();
}


void loop()
{
  if (digitalRead(D5)) {
    if (programRunning) {
      lcd.off();
      programRunning = false;
      delay(500);
    }
    else {
      startProgram();
    }
  }
}
