// Incluye la librería WiFi para establecer una conexión inalámbrica
#include <WiFi.h>  
// Biblioteca para el uso de OneWire
#include <OneWire.h>
// Biblioteca para el uso de sensores de temperatura
#include <DallasTemperature.h>
// Biblioteca para el uso de pantallas LCD I2C
#include <LiquidCrystal_I2C.h>
// Incluye la librería PubSubClient para establecer una conexión MQTT
#include <PubSubClient.h>
// Biblioteca para la comunicación I2C
#include <Wire.h>


//DATOS PARA CONEXION WIFI Y ENVIO AL SERVIDOR
// Nombre de red Wi-Fi
const char *ssid =  "Rosita";
// Contraseña de red Wi-Fi
const char *pass =  "R07101923.";
// Variable que contiene la dirección del servidor MQTT
const char* mqtt_server = "broker.emqx.io"; 

// Pin para la bomba
const int bomba = 23;
// Pin para el sensor de humedad
const int sensorH = 33;
// Pin para el sensor de lluvia
const int sensorPin = 14;

// Pin de datos para el sensor de temperatura
const int temperatura = 4;
// Objeto OneWire para la comunicación con el sensor de temperatura
OneWire oneWire(temperatura);
// Objeto DallasTemperature para la lectura de la temperatura
DallasTemperature tempSensor(&oneWire);

// Objeto de pantalla LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Crea un objeto de la clase WiFiClient para establecer una conexión WiFi
WiFiClient espClient;
// Crea un objeto de la clase PubSubClient para establecer una conexión MQTT
// utilizando el objeto WiFiClient creado previamente
PubSubClient client(espClient);
// Variable que contiene el tiempo transcurrido desde el último mensaje MQTT enviado
unsigned long lastMsg = 0;
// Tamaño del buffer para los mensajes MQTT
#define MSG_BUFFER_SIZE  (50)
// Variable que contiene el mensaje MQTT
char msg[MSG_BUFFER_SIZE];

int value = 0;

void setup_wifi() {
	// Retardo de 10ms para asegurar que la conexión WiFi se establezca correctamente
	delay(10);
	// Empleamos la conexion a la red WiFi
	Serial.println();
	Serial.print("Connecting to ");
	// Imprime el SSID de la red WiFi a la que se está conectando
	Serial.println(ssid);
	// Intenta conectarse a la red WiFi utilizando el SSID y la contraseña proporcionados
	WiFi.begin(ssid, pass);
	
	// Espera hasta que se establezca la conexión WiFi
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	// Inicializa el generador de números aleatorios con la marca de tiempo actual en microsegundos
	randomSeed(micros());

	Serial.println("");
	// Indica que la conexión WiFi se ha establecido correctamente
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	// Imprime la dirección IP asignada al dispositivo
	Serial.println(WiFi.localIP());
}

// La función callback es llamada cuando un mensaje es recibido en el topic suscrito
void callback(char* topic, byte* message, unsigned int length) {
	
	// Se imprime en el monitor serial el mensaje de llegada y el topic correspondiente
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	  
	// Se crean dos objetos String vacíos para almacenar la temperatura y humedad en el mensaje
	String messageTemp;
	String messageHum;

	 // Se itera sobre el mensaje recibido byte por byte
	for (int i = 0; i < length; i++) {
		// Se imprime cada byte como un carácter en el monitor serial
		Serial.print((char)message[i]);
		// Se agrega cada byte al objeto String correspondiente
		messageTemp += (char)message[i];
	}
	
	// Se imprime un salto de línea en el monitor serial para separar el mensaje recibido del siguiente mensaje
	Serial.println();

	// Si el topic corresponde a "esp32/output", se cambia la salida según el mensaje recibido
	if (String(topic) == "esp32/output"){
		// Se imprime en el monitor serial un mensaje indicando que se está cambiando la salida
		Serial.print("Changing output to ");
		// Si el mensaje recibido es "on", se enciende el pin temperatura
		if(messageTemp == "on"){
			Serial.println("on");
			digitalWrite(temperatura,HIGH);
		}
		// Si el mensaje recibido es "off", se apaga el pin temperatura
		else if(messageTemp == "off"){
			Serial.println("off");
			digitalWrite(temperatura,LOW);
		}
	}
}

// Función para intentar reconectar al cliente MQTT si la conexión falla
void reconnect() {
  // Bucle hasta que se restablezca la conexión
	while (!client.connected()) {
		// Se imprime en el monitor serial un mensaje indicando que se está intentando conectar al servidor MQTT
		Serial.print("Attempting MQTT connection...");
		// Se crea un ID de cliente aleatorio
		String clientId = "ESP8266Client-";
		clientId += String(random(0xffff), HEX);
		// Se intenta conectar al servidor MQTT
		if (client.connect(clientId.c_str())) {
			// Si la conexión es exitosa, se imprime un mensaje en el monitor serial y se suscribe al topic "esp32/output"
			Serial.println("connected");
			client.subscribe("esp32/output");
		} else {
			// Si la conexión falla, se imprime un mensaje en el monitor serial con el código de error
			// y se espera 5 segundos antes de intentarlo de nuevo
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Esperar 5 segundos antes de repetir el ciclo
			delay(5000);
		}
	}
}

void setup() {
	// Inicia el monitor serie
	Serial.begin(115200);
	// Configurar la conexión WiFi
	setup_wifi();
	// Se establece el servidor MQTT y se asigna el número del puerto utilizado para la conexión
	client.setServer(mqtt_server, 1883);
	client.setCallback(callback);
	// Configura el pin de la bomba como salida
	pinMode(bomba, OUTPUT);
	// Configura el pin del sensor de lluvia como entrada
	pinMode(sensorPin, INPUT);
	tempSensor.begin();
	// Apaga la bomba
	digitalWrite(bomba, LOW);

	// Inicia el bus I2C
	Wire.begin();
	lcd.init();
	lcd.backlight();
	// Se agrega el comando para borrar la pantalla al inicio
	lcd.clear(); 
}

void loop() {
	// Verifica si el cliente MQTT está conectado, en caso contrario, intenta reconectar
	if (!client.connected()) {
		reconnect(); 
	}
	// El cliente MQTT se mantiene activo mientras espera nuevos mensajes o instrucciones
	client.loop();

	// Almacena el tiempo actual en milisegundos
	unsigned long now = millis();
	
	// Verifica si ha pasado un tiempo determinado (5 segundos) desde la última vez que se enviaron los datos
	if (now - lastMsg > 5000) {
		lastMsg = now;
		// Lee la temperatura del sensor de temperatura
		tempSensor.requestTemperatures();
		float temperature = tempSensor.getTempCByIndex(0);
		// Lee el valor del sensor de humedad del suelo y lo convierte a un valor de humedad (0-100%)
		int humedad = analogRead(sensorH);
		int sensorValue = analogRead(sensorPin);
		float humidity = (100.0 / 1023.0) * humedad; // Convertir el valor de la lectura a un valor de humedad (0-100%)
		
		// Verifica si el valor de humedad está dentro del rango válido
		if (humidity >= 0 && humidity <= 100) {
			Serial.print("Humedad del suelo: ");
			Serial.print(humidity);
			Serial.println("%");

			// Escribe la lectura de humedad en la pantalla LCD
			lcd.setCursor(0, 1);
			lcd.print("Humedad: ");
			lcd.print(humidity);
			lcd.print("%"); 
			// Convierte el valor de humedad en una cadena de caracteres y lo publica en el tema "esp32/humidity"
			// Crea un arreglo de caracteres para almacenar la cadena resultante
			char humidityStr[10]; 
			// Convierte el valor de humidity a una cadena de caracteres
			dtostrf(humidity, 4, 2, humidityStr);
			client.publish("esp32/humidity", humidityStr); // Publica la cadena resultante en el tema "esp32/humidity"

			delay(2000);
	} else {
		Serial.println("No se ha detectado humedad de suelo");
	}

	// Verifica si se detecta lluvia a través del sensor de lluvia
	if (sensorValue < 500) {
		digitalWrite(bomba, LOW);
		Serial.println("Lluvia detectada - Bomba apagada");
	}
	else {
		// Verifica si la humedad del suelo está por encima del umbral máximo y apaga la bomba de riego
		if (humidity >= 32 && humidity <= 100) {
			digitalWrite(bomba, LOW);
			Serial.println("Exceso de Humedad - Bomba apagada");
		}
		// Verifica si no hay lluvia y la humedad del suelo está dentro del rango adecuado para activar la bomba de riego
		else {
			digitalWrite(bomba, HIGH);
			Serial.println("No hay lluvia - Bomba encendida");
		}
	}

	// Escribe la temperatura en el monior serial
	Serial.print("Temperatura: ");
	Serial.print(temperature);
	Serial.println(" C"); 
	
	// Escribe la temperatura en la pantalla LCD
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Temp: ");
	lcd.print(temperature);
	lcd.print("C    ");
	
	// Convierte la temperatura en una cadena de caracteres y lo publica en el tema "esp32/temperature"
	char temperatureStr[10];
	dtostrf(temperature, 4, 2, temperatureStr);
	client.publish("esp32/temperature", temperatureStr);


	// Esperar 2 segundos antes de repetir el ciclo
	delay(2000);

	}
}
