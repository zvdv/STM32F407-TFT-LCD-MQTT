/*
MQTT example
STM32 sends temperature XX.X from sensor to ESP32 via UART
ESP32 forms JSON and publishes
ESP32 reads hardware button and publishes as JSON
ESP32 subscribes to same button
ESP32 sends button state to STM32 via UART
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "driver/uart.h"

#define RXPIN     4     // GPIO 4 => RX for Serial1
#define TXPIN     5     // GPIO 5 => TX for Serial1
#define BAUD      115200  
#define NUM_TEMPERATURE_BYTES 4
#define MSG_BUFFER_SIZE  40

// update ssid and password for your network
const char* ssid = "CST-WIFI";
const char* password = "WIFI@CST";
//const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* mqtt_server = "broker.emqx.io";

const int buttonPin = 3;  // the number of the pushbutton pin - must be different from UART pins

// variable for storing the pushbutton status 
uint8_t buttonState = LOW; // by default not pressed
uint8_t lastButtonState = LOW;

uint8_t bright, red, green, blue;

bool timer_flag = false;
bool rx_flag = false;
size_t available;

hw_timer_t *timer = NULL;

// allocate the JSON document
JsonDocument doc;
const char* json; 

WiFiClient espClient;
PubSubClient client(espClient);

char msg[MSG_BUFFER_SIZE];

char newTemp[NUM_TEMPERATURE_BYTES+1]="0.0"; // one extra byte for null terminator

void setup_wifi() {

  delay(10);
  // connect to a WiFi network
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

void onTimer() {
  if(timer_flag == false) {
    timer_flag = true;
  }
}

void onReceiveFunction() {
  if(rx_flag == false) {
    rx_flag = true;
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.println();
  // deserialize the JSON document
  DeserializationError error = deserializeJson(doc, (char *) payload);
  // test if parsing succeeds
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  if(strcmp(topic,"esp32c3/Joyce/led") == 0) {
    bright = doc["bright"];
    red = doc["red"];
    green = doc["green"];
    blue = doc["blue"];
    rgbLedWrite(RGB_BUILTIN, (red*bright/255)&255, (green*bright/255)&255, (blue*bright/255)&255); 
  }
  else if(strcmp(topic,"esp32c3/Joyce/button") == 0) {
    buttonState = doc["buttonState"];
    // write data to UART
    Serial1.write(buttonState);
  }
}

void reconnect() {
  // loop until reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("esp32c3/Joyce", "MQTT program");
      Serial.println("MQTT started");
      // ... and resubscribe
      client.subscribe("esp32c3/Joyce/led"); // sample publish message: {"bright": 202, "red": 34, "green": 68, "blue": 102}
      client.subscribe("esp32c3/Joyce/button"); // sample publish message: {"buttonState":true}
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

  pinMode(BUILTIN_LED, OUTPUT);  // initialize the BUILTIN_LED pin as an output
  pinMode(buttonPin, INPUT);     // initialize pin connected to pushbutton as an input

  timer = timerBegin(1000000); //timer frequency
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 2000000, true, 0); // number of ticks; timer interrupts once per second (1000000/1000000 = 1)

  // set up Serial0 on other GPIO pins to accept user button messages from STM32
  
  // GPIO4 is Rx and GPIO5 is Tx
  Serial1.begin(BAUD, SERIAL_8N1, RXPIN, TXPIN);  // Rx = 4, Tx = 5 will work for ESP32, S2, S3 and C3
  Serial1.setRxBufferSize(NUM_TEMPERATURE_BYTES);
  Serial1.onReceive(onReceiveFunction, false);    // sets a RX callback function for Serial 1

  // send initial button state
  Serial1.write(buttonState);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // read the state of the pushbutton value
    buttonState = digitalRead(buttonPin);
  // if the pushbutton is pressed, buttonState is HIGH (pull-up)
  if (buttonState == HIGH && buttonState != lastButtonState) {
      snprintf (msg, MSG_BUFFER_SIZE, "{\"buttonState\":true}");
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("esp32c3/Joyce/button", msg);
      lastButtonState = buttonState;
  } else if (buttonState == LOW && buttonState != lastButtonState){
      snprintf (msg, MSG_BUFFER_SIZE, "{\"buttonState\":false}");
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("esp32c3/Joyce/button", msg);
      lastButtonState = buttonState;
  }
  
  if(rx_flag == true) {
    if(timer_flag == true) {
      available = Serial1.available();
      
      Serial.printf("onTimer callback:: %d bytes available: ", available); // serial receive buffer is 256 bytes max
      int i=0;
      while (available--) {
        if(i < NUM_TEMPERATURE_BYTES) {
          newTemp[i] = (char)Serial1.read();
          Serial.print(newTemp[i++]);
        }
        else
          //Serial.print((char)Serial1.read());
          Serial1.read();
      }
      newTemp[i]='\0';

      Serial.println();
      snprintf (msg, MSG_BUFFER_SIZE, "{\"temperature\":%s}", newTemp);
      Serial.print("Publish message: ");
      Serial.println(msg);
      Serial.println();
      client.publish("esp32c3/Joyce/temperature", msg);
     
      timer_flag = false;
    }
    else {
      available = Serial1.available();
      while (available--) {
        Serial1.read();
      }
    }
    rx_flag = false;
  } 
}
