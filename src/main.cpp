/*
 const char* ssid = "Linksys03578";
 const char* password = "phh6hhzi04"; 
 * 
  *  Json parametric GET REST response with ArduinoJSON library
  *  by Mischianti Renzo <https://www.mischianti.org>
 *
 *  https://www.mischianti.org/
 *
 *
 *  https://www.mischianti.org/2020/07/15/how-to-create-a-rest-server-on-esp8266-or-esp32-cors-request-option-and-get-part-4/
 */


/*
*   D5 - RX from VINDRIKTNING  
*   D6 - DS1820
*   D2 - SDA
*   D1 - SLC
*
*
*/
 
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>

#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>


#include <ArduinoOTA.h>


#include <SimplePgSQL.h>





#include <SerialCom.h>
#include <Types.h>

          

const char* ssid = "Linksys03578";
//const char* ssid = "cam_net_rear";
const char* password = "phh6hhzi04"; 
char identifier[24];


WiFiClientSecure secureclient;
WiFiClient client;
HTTPClient httpsClient;

char buffer[1024];

int NODB = 1;


 
ESP8266WebServer server(80);



// Set GPIOs for LED and PIR Motion Sensor
const int led = 12;
const int motionSensor = 5;


const long timerInterval = 60000;
const long blinkInterval  = 1000;
unsigned long previousBlinkTimer = 0;


int ledState = LOW; 
int displayIDX;

unsigned long previousTimer = 0;
unsigned long sensorTimer = 0;
unsigned long lastTrigger = 0;
boolean startTimer = false;

#include <queue>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

unsigned long startTime;

String getCurrentUTCTime() {
  return timeClient.getFormattedTime();
}







std::queue<String> datetimeQueue;

const int MAX_QUEUE_SIZE = 50;

void addDatetimeToQueue(String datetime) {
  if (datetimeQueue.size() >= MAX_QUEUE_SIZE) {
    datetimeQueue.pop();
  }
  datetimeQueue.push(datetime);
}



String getDatetimeFromQueue() {
  if (!datetimeQueue.empty()) {
    String datetime = datetimeQueue.front();
    datetimeQueue.pop();
    return datetime;
  }
  return "";
}
int getDateQueueSize() {
  return datetimeQueue.size();
}
 
void clearDateQueue() {
  while (!datetimeQueue.empty()) {
    datetimeQueue.pop();
  }
}
 


IRAM_ATTR void detectsMovement() {
  addDatetimeToQueue(String(getCurrentUTCTime()));

  digitalWrite(led, HIGH);
  startTimer = true;
  lastTrigger = millis();
}



void setCrossOrigin(){
    server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    server.sendHeader(F("Access-Control-Max-Age"), F("600"));
    server.sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
    server.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
};








void ProcessRequest()
{

  
 Serial.println(String(getCurrentUTCTime()));
  Serial.print(F("&"));


 
  DynamicJsonDocument doc(256);

  

  

 Serial.print(F("^"));

 
  
  setCrossOrigin();
 
 
 

Serial.print(F("%"));



  

 
  doc["ip"] = WiFi.localIP().toString();
  doc["macAddress"] = WiFi.macAddress();
  doc["SSID"] = WiFi.SSID();
  doc["RSSI"] = WiFi.RSSI();
  doc["queueSize"] = getDateQueueSize();
  doc["lastevent"] = getDatetimeFromQueue();
  clearDateQueue();



 



  Serial.print(F("*"));
  String buf;
  serializeJson(doc, buf);
  server.send(200, "application/json", buf);
  
Serial.println(F("$"));

// Turn off the LED after the number of seconds defined in the timeSeconds variable
//  if(startTimer && (millis() - lastTrigger > (blinkInterval ))) {
    digitalWrite(led, LOW);
//    startTimer = false;
 // }


// blip the light
digitalWrite(LED_BUILTIN, LOW);
delay(50);
digitalWrite(LED_BUILTIN, HIGH);
}

void sendCrossOriginHeader(){
    Serial.println(F("sendCORSHeader"));
 
    server.sendHeader(F("access-control-allow-credentials"), F("false"));
 
    setCrossOrigin();
 
    server.send(204);
}
 
// Define routing
void restServerRouting() {
    // server.on("/", HTTP_GET, []() {server.send(200, F("text/html"),F("Welcome to the REST Web Server"));});

    
 server.on(F("/ProcessRequest"), HTTP_OPTIONS, sendCrossOriginHeader);
 server.on(F("/ProcessRequest"), HTTP_GET, ProcessRequest);
    

}
 
// Manage not found URL
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  server.send(404, "text/plain", message);


  Serial.println(message);
}


void setupOTA() {
    ArduinoOTA.onStart([]() { Serial.println("Start"); });
    ArduinoOTA.onEnd([]() { Serial.println("\nEnd"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
   
 
        }
    });

    ArduinoOTA.setHostname(identifier);

    // This is less of a security measure and more a accidential flash prevention
//    ArduinoOTA.setPassword(identifier);
    ArduinoOTA.begin();
}

void setup(void) {


  

  
  pinMode(motionSensor,INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  pinMode(LED_BUILTIN, OUTPUT);
 

  previousTimer = millis();
  



  

  // Set LED to HIGH
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
   
  Serial.begin(9600);
  SerialCom::setup();



  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("-");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("+");
  }

  timeClient.begin();

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

 
 


  
 
  // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostmane esp8266.local
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  } 
 
  // Set server routing
  restServerRouting();

  
  // Set not found response
  server.onNotFound(handleNotFound);
  // Start server
  server.begin();
  Serial.println("HTTP server started");
 
 

  
  
 
  setupOTA();

  
 

   
} 
 
void loop(void) {

  String msg;
  unsigned long currentTimer = millis();

  
  server.handleClient();

  ArduinoOTA.handle();

  timeClient.update();

    if (currentTimer - previousTimer >= timerInterval) {
      previousTimer = currentTimer;

      
    //  ProcessRequest();

  
    }

  if (currentTimer - previousBlinkTimer >= blinkInterval) {
    // save the last time you blinked the LED
    previousBlinkTimer = currentTimer;

   // ESP.wdtFeed();


  
  


    // set the LED with the ledState of the variable:
    //digitalWrite(LED_BUILTIN, ledState);
  }
  

  
  
 
 
}
