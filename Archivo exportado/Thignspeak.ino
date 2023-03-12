#include "ThingSpeak.h"   //Se incluye la biblioteca ThingSpeak.h para enviar los datos //a la plataforma IoT ThingSpeak.
#include <WiFi.h>   //Se incluye la biblioteca WiFi.h para conectarse a una red //Wi-Fi.
#include <OneWire.h>
#include <DallasTemperature.h>


//DATOS PARA CONEXION WIFI Y ENVIO AL SERVIDOR
const char *API_Key = "ARJ1MKEIHGJIKV7C"; 
const char *ssid =  "Rosita";     // el ssid del wifi usado
const char *pass =  "R07101923.";
const char *server = "api.thingspeak.com";
//Se establecen las variables necesarias para conectarse a la red Wi-Fi y la plataforma ThingSpeak, como la clave de API, el nombre de la red Wi-Fi, la contraseña de la red Wi-Fi, el ID del canal en ThingSpeak y el cliente Wi-Fi./
 

unsigned  long Channel_ID = 3;
WiFiClient client;

unsigned long last_time = 0;
unsigned long Delay = 10000;

//CONFIGURACIÓN DEL SENSOR DS18B20 Temperatura de suelo
const int temperature = 4;  //Pin de conexión del sensor DS18B20 
OneWire oneWire(temperature);
DallasTemperature sensors (&oneWire);

//CONFIGURACIÓN DEL SENSOR HW-080 Humedad de suelo
const int humedad = 33; //Pin de conexión del sensor HW-080
int sensorH;


void setup() {
       Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);

// INICIAR SENSORES O CONFIGURAMOS PINES DE LECTURA
  Serial.begin(115200);
  sensors.begin();  //Sensor DS18B20 temperatura de suelo
  pinMode(sensorH,INPUT); //Configurar pin de lectura del Sensor HW-080 humedad de suelo 
 
  
}

void loop() {
  if ((millis() - last_time) > Delay){
  if(WiFi.status()!= WL_CONNECTED){
    Serial.println("Connecting to ");
    while(WiFi.status()!= WL_CONNECTED){
              WiFi.begin(ssid, pass);
        delay(500);
        }
        Serial.println("WiFi connected");
 }
  int sensorH = analogRead(humedad); // Leer el valor del sensor HW-080 Humedad de suelo
  float humidity = (100.0 / 1023.0) * sensorH; // Convertir el valor de la lectura a un valor de humedad (0-100%)
  Serial.print("Humedad del suelo: ");
  Serial.print(humidity);
  Serial.println("%");
  delay(2000); 
      
  
  Serial.print("Mandando comandos a los sensores ");  //Leer la temperatura
  sensors.requestTemperatures();  //Lectura en grados celsius
  float temperature = sensors.getTempCByIndex(0); //Escribir los datos en el monitor de serie
  Serial.print("Temperatura sensor : ");
  Serial.print(temperature);
  Serial.println("°C");
  delay(5000);

  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity); 

                         
  int Data = ThingSpeak.writeFields(Channel_ID, API_Key);
                            
  if(Data == 200){
    Serial.println("Channel updated successfully!");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(Data));
  } 
  last_time = millis();
  }
}
