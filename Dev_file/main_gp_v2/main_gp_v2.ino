#include <ESP8266WiFi.h>
#include <WiFiManager.h>  
//#include <DNSServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHT.h"
#define DHTPIN 12     // D6 pin of esp8266
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);
// Ticker for watchdog

#include <Ticker.h>
Ticker secondTick;

//MQTT  credentials
const char *mqtt_server = "broker.datasoft-bd.com";
const char *public_mqtt_server = "broker.hivemq.com";
const int mqttPort = 1883;
int mqttTryCounter=0;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long previousMillis = 0;
unsigned long previousMillis2 = 0;
unsigned long previousMillis3 = 0;

long wifi_check_interval = 5000;
long publish_interval = 50000;
long deep_sleep_interval = 65000;



//char sleep_time[] = "20";
int sleep_time = 20; //in seconds



//---------------Difining Sensor Pin----------------------------------------------//
int analogPin = A0;
int Temperature = 0;
int Humidity = 0;
int methane = 0;
int D4 = 2;//led pin

//-------------------------Variable to store sensor data----------------------------//
int data1 = 0;
int data2 = 0;
int data5 = 0;
String did = "DB101";

String data = "";

volatile int watchdogCount = 0;
char sensorData[68];
char alertData[68];

extern "C" {
#include "user_interface.h"
}



//--------------------Watchdog----------------------//

void ISRwatchdog() {
  watchdogCount++;
  if (watchdogCount == 180) {
    Serial.println("Watchdog bites!!!");
    ESP.reset();
  }
}




//--------------------------------Main Setup----------------------------------------------------//

void setup() {

  pinMode(D4, OUTPUT);
  secondTick.attach(1, ISRwatchdog);// Attaching ISRwatchdog function
  Serial.begin(115200);
  dht.begin();

  //It is recommended to check return status on .begin(), but it is not required.
  pinMode(analogPin, INPUT);
 
  
  client.setServer(mqtt_server,mqttPort);//Connecting to broker
 
  client.setCallback(callback); // Attaching callback for subscribe mode
}//setup ends




//------------------------------------Main Loop--------------------------------------------------//
void loop(){

  watchdogCount = 0;
  Serial.println("");  
    
  unsigned long currentMillis = millis();
//  wifi_manager();
  if(currentMillis - previousMillis3 > wifi_check_interval) {
    previousMillis3 = currentMillis;
    
//    Serial.println(previousMillis3); 
    
    if (WiFi.status() != WL_CONNECTED){ 
      //  set_wifi();
      wifi_manager();
    }
    else {
      Serial.println(" Wifi already connected");
    }
}

  if (!client.connected() && (WiFi.status() == WL_CONNECTED))
     {
      reconnect();
     }

   client.loop();

    //need to work here  
    data = sensor_data();
    
    
    data.toCharArray(sensorData,68);
    Serial.println("Sendor data: " + data);
    
    if(currentMillis - previousMillis > publish_interval) {
    
        previousMillis = currentMillis;
        Serial.println("Ticking every 50 seconds"); 
        Serial.println("Inside sensor data publish");
        
        if (!client.connected()){ reconnect();}
        
        int result = client.publish("dsiot/gphouse/ws",sensorData); 
        
//        Serial.println(result);
        if (result ==1){
          Serial.println("Pulished successfully");
        }else{
            Serial.println("Unable to publish");
        }
    }// Timer ends


  if(currentMillis - previousMillis2 > deep_sleep_interval) {
    previousMillis2 = currentMillis;

//    Serial.println("Inside deep_sleep");
//    deep_sleep_new(sleep_time); 
  }
  
}//LOOP ENDS



//---------------------------------------Read Sensor Data---------------------------------------//
String sensor_data()
{ 
  Serial.println("Inside Sensor data read");
  
    data1 = temp(); 
    if ( data1 >30) {
    Serial.println("temp outta hand"); // need to intigrate mqtt + RF 
    if (!client.connected()){ reconnect();}

    int result = client.publish("dsiot/gphouse/ws",sensorData);    
    int result2 = client.publish("dsiot/gphouse/ws/alert","1");
     if (result2 ==1){
          Serial.println("Pulished successfully");
        }else{
            Serial.println("Unable to publish");
        }
    } // need to intigrate mqtt + RF 
    

    
  data2 = hum(); 
  if ( data2 >95) {
    Serial.println("hum outta hand"); // need to intigrate mqtt + RF 
    if (!client.connected()){ reconnect();}
    
    int result = client.publish("dsiot/gphouse/ws",sensorData);     
    int result2 = client.publish("dsiot/gphouse/ws/alert","2");
    
     if (result2 ==1){
          Serial.println("Pulished successfully");
        }else{
            Serial.println("Unable to publish");
        }
    }

    
  data5 = tgsVal();
  if ( data5 >750) {
    Serial.println("LPG outta hand"); // need to intigrate mqtt + RF 
    if (!client.connected()){ reconnect();}

    int result = client.publish("dsiot/gphouse/ws",sensorData);
    int result2 = client.publish("dsiot/gphouse/ws/alert","3");
     if (result2 ==1){
          Serial.println("Pulished successfully");
        }else{
            Serial.println("Unable to publish");
        }
    }

  String msg2 = "";
//  msg2 = msg2 + "{\"DID\":" + did + "," + "\"TMP\":" + data1 + "," + "\"HUM\":" + data2 + "," + "\"CO2\":" + data3 + "," + "\"VOC\":" + data4 + "," + "\"CH4\":" + data5 + "}";
  msg2 = msg2 + "{\"DID\":" + "\""+ did + "\""+ "," + "\"TMP\":" + data1 + "," + "\"HUM\":" + data2 + "," + "\"CH4\":" + data5 + "}";
  
  delay(200);       //........ 0.2 sec delay...........//
  return msg2;

}


void deep_sleep_new(int stime){
  Serial.println("Device is going into DEEEEEEEP_Sleep");
  Serial.print("Sleep Time(second):");
  Serial.println(stime);
  ESP.deepSleep(stime * 1000000); //sleep for 10 seconds
  delay (500);
}

void set_wifi() {
  delay(250);
  int tryCount = 0;
  Serial.println("");
  Serial.println("Connecting to WiFi");
  WiFi.begin("DataSoft_WiFi", "support123");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);        //........ 1 sec delay
    Serial.print(".");
    tryCount++;
    if (tryCount == 10) return loop(); //exiting loop after trying 10 times
  }
  Serial.println("");
  Serial.println("Connected");
  Serial.println(WiFi.localIP());
  delay(250);
  }
