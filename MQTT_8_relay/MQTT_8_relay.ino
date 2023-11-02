#include <ESP8266WiFi.h> // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h> // Allows us to connect to, and publish to the MQTT broker
#include <DHT.h>        // DHT11 temperature and humidity sensor Predefined library

#define DHTTYPE DHT22
//#define PUMP D7
//#define dht_dpin D3

const int ledPin = 0; // This code uses the built-in led for visual feedback that the button has been pressed
const int buttonPin = 13; // Connect your button to pin #13
float t = 0.0;

// WiFi
// Make sure to update this for your own WiFi network!
const char* ssid = "FASTWEB-D82B93";
const char* wifi_password = "W1JA3M3R2A";

// MQTT
// Make sure to update this for your own MQTT Broker!
// TODO: externalize parameters!!!
const char* mqtt_server = "192.168.1.12";
const char* mqtt_moisture_topic = "moisture";
const char* mqtt_temperature_topic = "temperature";
const char* mqtt_sub_topic = "pump_activation";
const char* status_topic = "smart_garden_status";
const char* mqtt_username = "rio";
const char* mqtt_password = "onslario89";
const int mqtt_port = 1883; //choose MQTT port
// The client id identifies the ESP8266 device. Think of it a bit like a hostname (Or just a name, like Greg).
const char* clientID = "ClientID";
const char* ok_message = "ON";

//relay water pump control
int RelayControl1 = 13; //D7 
int RelayControl2 = 15; //D8
int RelayControl3 = 14; //D5
int RelayControl4 = 12; //D6
int RelayControl5 = 16; //D0

//multiplexer control
int MultiplexerControl4 = 2; //D4
int MultiplexerControl3 = 0; //D3
int MultiplexerControl2 = 4; //D2

WiFiClient espClient;
PubSubClient client(espClient);
//DHT dht(dht_dpin, DHTTYPE);


void callback(char* topic, byte* payload, unsigned int length) {

  String topic_str = String(topic); //onverting from character array pointer to string
  String string = "";
  for (int i = 0; i < length; i++) { //getting received message
    //Serial.print((char)payload[i]);
    string.concat((char)payload[i]);
  }

  //pump activation topic
  if (topic_str.equals(mqtt_sub_topic)) {
    Serial.println("Message received from " + topic_str + " topic, activating pump: " + string);

    if (string.equals("1")) {
      digitalWrite(RelayControl1, LOW);
      delay(5000);
      digitalWrite(RelayControl1, HIGH);
    }
    else if (string.equals("2")) {
      digitalWrite(RelayControl2, LOW);
      delay(5000);
      digitalWrite(RelayControl2, HIGH);
    }
    else if (string.equals("3")) {
      digitalWrite(RelayControl3, LOW);
      delay(5000);
      digitalWrite(RelayControl3, HIGH);
    }
    else if (string.equals("4")) {
      digitalWrite(RelayControl4, LOW);
      delay(5000);
      digitalWrite(RelayControl4, HIGH);
    }
    else if (string.equals("5")) {
      digitalWrite(RelayControl5, LOW);
      delay(5000);
      digitalWrite(RelayControl5, HIGH);
    }
  }
}


int mutiplexerReading(boolean A, boolean B, boolean C) {
  digitalWrite(MultiplexerControl4, A);
  digitalWrite(MultiplexerControl3, B);
  digitalWrite(MultiplexerControl2, C);
  float moisture_value = moistureSensor(A0);
  char cstr[16];
  //Serial.print("Moisture value: ");
  //Serial.println(moisture_value);

  return moisture_value;
}


float moistureSensor(char inputPin) {
  int dryValue = 1023;
  int wetValue = 320;
  int friendlyDryValue = 0;
  int friendlyWetValue = 100;
  //function which calculates the moisture sensor output value
  int rawValue = analogRead(inputPin); //Read the analog value
  Serial.print("Raw value: ");
  Serial.print(rawValue);
  int percentage_value = map(rawValue, dryValue, wetValue, friendlyDryValue, friendlyWetValue); //Calibrazione sensore: https://greensense.github.io/Blog/2017/02/17/Arduino-Soil-Moisture-Sensor-Calibration/
  Serial.print(", moisture pecentage: ");
  Serial.print(percentage_value);
  Serial.print("%");
  Serial.print("\n");
  return percentage_value;
}



void setup() {
  //configuring Serial, WIFI, outputs
  Serial.begin(115200);

  WiFi.begin(ssid, wifi_password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  //Configuring MQTT client
  Serial.println("Configuring MQTT client..");
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP32Client", mqtt_username, mqtt_password )) {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  //Subscribing to MQTT queue
  client.subscribe(mqtt_sub_topic);

  //Initializing 4051 control pins
  pinMode(MultiplexerControl4, OUTPUT);
  pinMode(MultiplexerControl3, OUTPUT);
  pinMode(MultiplexerControl2, OUTPUT);

  //Itilializing relay outputs
  pinMode(RelayControl1, OUTPUT);
  pinMode(RelayControl2, OUTPUT);
  pinMode(RelayControl3, OUTPUT);
  pinMode(RelayControl4, OUTPUT);
  pinMode(RelayControl5, OUTPUT);

   //turning off relays
  digitalWrite(RelayControl1, HIGH);
  digitalWrite(RelayControl2, HIGH);
  digitalWrite(RelayControl3, HIGH);
  digitalWrite(RelayControl4, HIGH);
  digitalWrite(RelayControl5, HIGH);

}




void loop() {

 
  //1.
  Serial.println("Checking pump activation..");
  client.loop();
  delay(5000);

  //2. Analog readings..
  //2.1. Moisture sensors
  //sensor 1
  int value_1 = mutiplexerReading(LOW, LOW, LOW);
  //sensor 2
  int value_2 = mutiplexerReading(LOW, LOW, HIGH);
  //sensor 3
  int value_3 = mutiplexerReading(LOW, HIGH, LOW);
  //sensor 4
  int value_4 = mutiplexerReading(HIGH, HIGH, LOW);
  //sensor 5
  int value_5 = mutiplexerReading(HIGH, LOW, HIGH);


  //3. Send sensor values through MQTT..
  String moisture_topic_str = String(mqtt_moisture_topic);
  int values[]={value_1, value_2, value_3, value_4, value_5};
  for (int i=0; i<=4; i++) {
    int subtopic = i+1;
    String moisture_topic=moisture_topic_str+"/"+subtopic;
    char cstr[16];
    if (client.publish(moisture_topic.c_str(), itoa(values[i], cstr, 10))) { 
    }
    else {
    Serial.println("Message failed to send.");
    //}
    }  
  }

}
