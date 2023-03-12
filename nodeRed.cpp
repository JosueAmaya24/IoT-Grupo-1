#include <WiFi.h>  
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include <Wire.h>


//DATOS PARA CONEXION WIFI Y ENVIO AL SERVIDOR
const char *ssid =  "Rosita";     // el ssid del wifi usado
const char *pass =  "R07101923.";
const char* mqtt_server = "broker.emqx.io"; 

const int bomba = 23;
const int sensorH = 33;
const int sensorPin = 14;
const int temperatura = 4;
OneWire oneWire(temperatura);
DallasTemperature tempSensor(&oneWire);

LiquidCrystal_I2C lcd(0x27, 16, 2);

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String messageTemp;
  String messageHum;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "esp32/output"){
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(temperatura,HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(temperatura,LOW);
    }
  }
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
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
    if (!client.connected()) {
    reconnect(); 
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
  
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
      char humidityStr[10]; // Crea un arreglo de caracteres para almacenar la cadena resultante
      dtostrf(humidity, 4, 2, humidityStr); // Convierte el valor de humidity a una cadena de caracteres
      client.publish("esp32/humidity", humidityStr); // Publica la cadena resultante en el tema "esp32/humidity"
      
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
  char temperatureStr[10];
  dtostrf(temperature, 4, 2, temperatureStr);
  client.publish("esp32/temperature", temperatureStr);
  
 // lcd.clear();
  

 // Agregamos espacios para asegurarnos de borrar el texto anterior
  delay(2000);

}
}