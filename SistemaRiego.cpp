#include "ThingSpeak.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

const int temperatura = 4;
OneWire oneWire(temperatura);
DallasTemperature tempSensor(&oneWire);

const int bomba = 23;
const int sensorH = 33;
const int sensorPin = 14;

const char *API_Key = "ARJ1MKEIHGJIKV7C"; 
const char *ssid =  "Rosita";
const char *pass =  "R07101923.";
const char *server = "api.thingspeak.com";

unsigned long Channel_ID = 3;
WiFiClient client;

unsigned long last_time = 0;
unsigned long delay_time = 10000;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);

  pinMode(bomba, OUTPUT);
  pinMode(sensorPin, INPUT);
  tempSensor.begin();
  digitalWrite(bomba, LOW);

  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void loop() {
  if ((millis() - last_time) > delay_time) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Connecting to ");
      Serial.println(ssid);
      while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid, pass);
        delay(500);
      }
      Serial.println("WiFi connected");
    }
    
    tempSensor.requestTemperatures();
    float temperature = tempSensor.getTempCByIndex(0);
    int humedad = analogRead(sensorH);
    int sensorValue = analogRead(sensorPin);
    float humidity = (100.0 / 1023.0) * humedad;
    
    if (humidity >= 0 && humidity <= 100) {
      Serial.print("Humedad del suelo: ");
      Serial.print(humidity);
      Serial.println("%");
      lcd.setCursor(0, 1);
      lcd.print("Humedad: ");
      lcd.print(humidity);
      lcd.print("%     ");
    } else {
      Serial.println("No se ha detectado humedad de suelo");
    }
    
    if (sensorValue < 500) {
      digitalWrite(bomba, LOW);
      Serial.println("Lluvia detectada - Bomba apagada");
    } else {
      if (humidity >= 26.8 && humidity <= 100) {
        digitalWrite(bomba, LOW);
        Serial.println("Exceso de Humedad - Bomba apagada");
      } else {
        digitalWrite(bomba, HIGH);
        Serial.println("No hay lluvia - Bomba encendida");
      }
    }
    
    Serial.print("Temperatura: ");
    Serial.print(temperature);
    Serial.println(" C");
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print("C     ");

    ThingSpeak.setField(1, temperature);
    ThingSpeak.setField(2, humidity);
    ThingSpeak.setField(4, sensorValue);

    int response = ThingSpeak.writeFields(Channel_ID, API_Key);

    if (response == 200) {
      Serial.println("Channel updated successfully!");
    } else {
      Serial.println("Problem updating channel. HTTP error code " + String(response));
    }

    last_time = millis();
  }
}