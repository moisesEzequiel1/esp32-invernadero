#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define LOWVALUE_SENSOR 1970
#define HIGHVALUE_SENSOR 3450 
struct vent {
  byte pina1;
  byte pina2;
  char descripcion[100];
};

struct sensor {
  byte pina;
  byte low;
  byte high;
  char descripcion[100];
};

// Update these with values suitable for your network.
const char* ssid = "Puro Sinaloa";
const char* password = "";

const char* mqtt_server = "54.225.233.86";

WiFiClient espClient;
PubSubClient client(espClient);

// Create variable to hold mqtt messages
#define MSG_BUFFER_SIZE  (1000)
char msg[MSG_BUFFER_SIZE];

// Connecting to the WIFI network
void setup_wifi() {
  delay(10);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

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

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), "mqtt", "")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("esp32/outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("esp32/inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
double readadc(byte pin, float highinterval, float lowinternval){
  double ADC_VAL = analogRead(pin);
  double mapvalue = map(ADC_VAL, highinterval, lowinternval, 0, 100);
  return mapvalue; 
}
double readadc420ma(byte pin, float lowinterval, float highinterval, float min, float max){
  double ADC_VAL = analogRead(pin);
  pinMode(pin, INPUT_PULLDOWN);
  double mapvalue = map(ADC_VAL, lowinterval, highinterval, min, max);
  return mapvalue; 
}
double sensorinit(byte analogpin){
  pinMode(analogpin, INPUT_PULLDOWN);
  double sensor_value = readadc(analogpin, HIGHVALUE_SENSOR, LOWVALUE_SENSOR);
  return sensor_value; 

  
}
// double sensorinit_currentsensors(byte analogpin){
//   pinMode(analogpin, INPUT_PULLDOWN);
//   double sensor_value = readadc(analogpin, );

// }


void configVents(struct vent p){
  pinMode(p.pina1, OUTPUT);
  pinMode(p.pina2, OUTPUT);
}


void setup() {
  Serial.begin(9600);


  struct vent p1;
  p1.pina1 = 23;
  p1.pina2 = 22;
  struct vent p2;
  p2.pina1 = 1;
  p2.pina2 = 3;

  
  configVents(p1);
  configVents(p2);
    
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  // Connect to the mqtt client
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // lectura de los sensores de humedad de suelo 
  double objt = sensorinit(33);
  double objt2 = sensorinit(32);
  double objt3 = sensorinit(35);
  
//   It transmits current temperature/humidity to other devices(PC, recorder, etc.) and outputs DC4-20mA.
// It outputs DC4mA at -19.9℃ of temperature and 0%RH of humidity, DC20mA at 60℃ of temperature and 99.9%RH of
// humidity. The temperature and humidity output are separated and the resolution is divisible by 1,000.


//   Wide range of temp./humidity measurement
// -19.9 to 60.0℃ / 0.0 to 99.9%RH
  // sensore de humedad y temperatura 
  double objt4 = readadc420ma(39, 4095, 819, 0, 99);
  double objt5 = readadc420ma(39, 4095, 819, -19.9, 60);

  

  
  
  // Pines de control de los ventiladores 
  // 23,22,1,3
  digitalWrite(23, LOW);
  digitalWrite(22, LOW);
  digitalWrite(01, LOW);
  digitalWrite(3, LOW);
  // Pines disponibles para distintos menesteres 
  digitalWrite(21, HIGH);
  digitalWrite(19, HIGH);
  digitalWrite(18, HIGH);
  digitalWrite(5, HIGH);
  // Control de la bomba pin no 17 esp32
  digitalWrite(17, LOW);   
  
  //  creado la cadena a enviar mediante mqtt
  String message = String("weather,location=us moist1="+String(objt)+",moist2="+String(objt2)+",moist3="+String(objt3)+",moist4="+String(objt4)+",t="+String(objt5));
  message.toCharArray(msg, message.length());
  Serial.println(msg);

  // enviando la cadena al tema destino en el broker mqtt
  client.publish("esp32/sensors", msg);
  delay(1000); 
}
