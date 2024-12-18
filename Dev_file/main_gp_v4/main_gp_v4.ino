#include <ESP8266WiFi.h>
#include <WiFiManager.h>  
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHT.h"
#define DHTPIN 12     // D6 pin of esp8266
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

DHT dht(DHTPIN, DHTTYPE);
// Ticker for watchdog

#include <Ticker.h>
Ticker secondTick;

//MQTT  credentials
const char *mqtt_server = "broker.datasoft-bd.com";
const char *device_config = "dsiot/gphouse/dbox/dev_config";
const char *device_response = "dsiot/gphouse/dbox/dev_response";
//const char *public_mqtt_server = "broker.hivemq.com";

const int mqttPort = 1883;
int mqttTryCounter=0;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0;
unsigned long previousMillis2 = 0;
unsigned long previousMillis3 = 0;
unsigned long previousMillis4 = 0;

long wifi_check_interval = 50000;
long publish_interval = 60000;
long deep_sleep_interval = 65000;
long eeprom_clr_button_time = 5000;

int sleep_time = 20; //in seconds
int tgs_upper_limit = 750;
int temp_upper_limit = 45;

//---------------Difining Sensor Pin----------------------------------------------//
int button = 4;
int button_state = 0; 
int analogPin = A0;
int Temperature = 0;
int Humidity = 0;
int methane = 0;

//boolean rf_alarm = false ;
boolean eeprom_rst = false ;

//-------------------------Variable to store sensor data----------------------------//
int data1 = 0;
int data2 = 0;
int data3 = 0;
String did = "GP101";

String data = "";

volatile int watchdogCount = 0;
char sensorData[68];
char alertData[68];

//extern "C" {
//#include "user_interface.h"
//}



//--------------------Watchdog----------------------//

void ISRwatchdog() {
  watchdogCount++;
  if (watchdogCount == 120) {
    Serial.println("Watchdog bites!!!");
    ESP.reset();
  }
}




//--------------------------------Main Setup----------------------------------------------------//

void setup() {
  delay(1500);
  pinMode(D4, OUTPUT);
  secondTick.attach(1, ISRwatchdog);// Attaching ISRwatchdog function
  Serial.begin(115200);
  dht.begin();
  pinMode(button, INPUT); // declare push button as input
  //It is recommended to check return status on .begin(), but it is not required.
  pinMode(analogPin, INPUT);
 
  mySwitch.enableTransmit(D3); //gpio 0 // RF pin
  
  client.setServer(mqtt_server,mqttPort);//Connecting to broker
 
  client.setCallback(callback); // Attaching callback for subscribe mode

  wifi_manager();
  delay(50);
  
}//setup ends




//------------------------------------Main Loop--------------------------------------------------//
void loop(){

//  if (rf_alarm == true ){
//      delay(100);
//      mySwitch.send(5555, 24);
//      delay(4000);
//      Serial.print("RF is Triggered");
//      rf_alarm= false ;
//
//      }

  watchdogCount = 0;
  Serial.println("");  
  button_state = digitalRead(button);
    
  unsigned long currentMillis = millis();

  if (button_state == HIGH) {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("LED HIGH");
        delay(250);
       }
     else {
      digitalWrite(LED_BUILTIN, LOW);
        previousMillis4 = millis();
        delay(250);
       }
  if (millis() - previousMillis4 > eeprom_clr_button_time) {
      Serial.println("Action !!!");
      eeprom_rst = true ;
      }

    if (eeprom_rst == true ){
        delay(1000);
        WiFi.disconnect();    
        delay(3000);
     if (WiFi.status() != WL_CONNECTED){
        Serial.println("wifi disconnected");
        delay(300);
        wifi_manager_reset ();
        delay(300);
        eeprom_rst= false ;
        }      
      }
  
  if(currentMillis - previousMillis3 > wifi_check_interval) {
    previousMillis3 = currentMillis;
    
//    Serial.println(previousMillis3); 
    
    if (WiFi.status() != WL_CONNECTED){ 
      //  set_wifi();
//      wifi_manager();
//      wifiManager.setConfigPortalTimeout(60);
      Serial.println(" unable to connect wifi, returning to loop");
      return loop();
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
  
}//LOOP ENDS

void rf_alarm(){
      
      delay(100);
      mySwitch.send(5555, 24);
      delay(4000);
      Serial.print("RF is Triggered");
//      rf_alarm= false ;  
  
  }

//---------------------------------------Read Sensor Data---------------------------------------//
String sensor_data()
{ 
  Serial.println("Inside Sensor data read");
  
    data1 = temp(); 
    if ( data1 > temp_upper_limit) {
//    rf_alarm =true ;
    delay(100);
    rf_alarm();
    Serial.println("temp outta hand"); // need to intigrate mqtt + RF 
    if (!client.connected()){ reconnect();}

    int result = client.publish("dsiot/gphouse/dbox",sensorData);    
    int result2 = client.publish("dsiot/gphouse/dbox/alert","1");
     if (result2 ==1){
          Serial.println("Pulished successfully");
        }else{
            Serial.println("Unable to publish");
        }
    } // need to intigrate mqtt + RF 
    

    
  data2 = hum(); 
  if ( data2 >95) {
    
//    rf_alarm =true ;
    Serial.println("hum outta hand"); // need to intigrate mqtt + RF 
    if (!client.connected()){ reconnect();}
    
    int result = client.publish("dsiot/gphouse/dbox",sensorData);     
    int result2 = client.publish("dsiot/gphouse/dbox/alert","3");
    
     if (result2 ==1){
          Serial.println("Pulished successfully");
        }else{
            Serial.println("Unable to publish");
        }
    }

    
  data3 = tgsVal();
  if ( data3 > tgs_upper_limit) {
    delay(200);
//    rf_alarm =true ;
    rf_alarm();
    delay(200);
    Serial.println("LPG outta hand"); // need to intigrate mqtt + RF 
    if (!client.connected()){ reconnect();}

    int result = client.publish("dsiot/gphouse/dbox",sensorData);
    int result2 = client.publish("dsiot/gphouse/dbox/alert","3");
     if (result2 ==1){
          Serial.println("Pulished successfully");
        }else{
            Serial.println("Unable to publish");
        }
    }

  String msg2 = "";
//  msg2 = msg2 + "{\"DID\":" + did + "," + "\"TMP\":" + data1 + "," + "\"HUM\":" + data2 + "," + "\"CO2\":" + data3 + "," + "\"VOC\":" + data4 + "," + "\"CH4\":" + data3 + "}";
  msg2 = msg2 + "{\"DID\":" + "\""+ did + "\""+ "," + "\"TMP\":" + data1 + "," + "\"HUM\":" + data2 + "," + "\"CH4\":" + data3 + "," + "\"alert\":" + data3 + "}";
  
  delay(200);       //........ 0.2 sec delay...........//
  return msg2;

}


//
//void deep_sleep_new(int stime){
//  Serial.println("Device is going into DEEEEEEEP_Sleep");
//  Serial.print("Sleep Time(second):");
//  Serial.println(stime);
//  ESP.deepSleep(stime * 1000000); //sleep for 10 seconds
//  delay (500);
//}


