#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include <Wire.h>
const int temperatura = 4;

OneWire oneWire(temperatura);
DallasTemperature tempSensor(&oneWire);

const int bomba = 23;
const int sensorH = 33;
const int sensorPin = 14;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  pinMode(bomba, OUTPUT);
  pinMode(sensorPin, INPUT);
  tempSensor.begin();
  digitalWrite(bomba, LOW);

  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear(); // Se agrega el comando para borrar la pantalla al inicio
}

void loop() {
  tempSensor.requestTemperatures();
  float temperature = tempSensor.getTempCByIndex(0);
  int humedad = analogRead(sensorH);
  int sensorValue = analogRead(sensorPin);
  float humidity = (100.0 / 1023.0) * humedad; // Convertir el valor de la lectura a un valor de humedad (0-100%)
  if (humidity >= 0 && humidity <= 100) {
      Serial.print("Humedad del suelo: ");
      Serial.print(humidity);
      Serial.println("%");
      lcd.setCursor(0, 1);
      lcd.print("Humedad: ");
      lcd.print(humidity);
      lcd.print("%"); // Agregamos espacios para asegurarnos de borrar el texto anterior
      delay(2000);
  } else {
      Serial.println("No se ha detectado humedad de suelo");
  }
  
  if (sensorValue < 500) {
    digitalWrite(bomba, LOW);
  //  lcd.setCursor(0, 0);
  //  lcd.print("LlD - RD");
    Serial.println("Lluvia detectada - Bomba apagada");
  }
  else {
    if (humidity >= 32 && humidity <= 100) {
      digitalWrite(bomba, LOW);
    //  lcd.setCursor(0, 0);
    //  lcd.print("Riego desactivado"); // Agregamos espacios para asegurarnos de borrar el texto anterior
      Serial.println("Exceso de Humedad - Bomba apagada");
    }
    else {
      digitalWrite(bomba, HIGH);
    //  lcd.setCursor(0, 0);
   //   lcd.print("No hay lluvia - Riego activado"); // Agregamos espacios para asegurarnos de borrar el texto anterior
      Serial.println("No hay lluvia - Bomba encendida");
    }
  }
  
  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.println(" C");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C    ");
  
  
 // lcd.clear();
  

 // Agregamos espacios para asegurarnos de borrar el texto anterior
  delay(2000);

}