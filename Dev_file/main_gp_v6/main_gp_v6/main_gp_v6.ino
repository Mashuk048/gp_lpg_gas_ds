#include <ESP8266WiFi.h>
#include <WiFiManager.h>  
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <RCSwitch.h>

#define DHTPIN 12     // D6 pin of esp8266
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define DEBUG 1

#if DEBUG
#define  P_R_I_N_T(x)   Serial.println(x)
#else
#define  P_R_I_N_T(x)
#endif


RCSwitch mySwitch = RCSwitch();

DHT dht(DHTPIN, DHTTYPE);
// Ticker for watchdog

#include <Ticker.h>
Ticker secondTick;
Ticker thirdTick;

//MQTT  credentials
//const char *mqtt_server = "broker.datasoft-bd.com";
const char *mqtt_server = "139.59.80.128";
const char *device_data   = "dsiot/gphouse/dbox/dev_data";
const char *device_config   = "dsiot/gphouse/dbox/dev_config";
const char *device_response = "dsiot/gphouse/dbox/dev_response";
const char *mqtt_user = "dsgp";
const char *mqtt_pass = "Gp@2o2o";

//-----device ID----------//
const char* dmac = "GP SensBoard 102";
const char* did = "gp102";

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
long eeprom_clr_button_time = 5000;

int sleep_time = 20; //in seconds
int tgs_upper_limit = 800;
int temp_upper_limit = 50;



//---------------Defining Sensor Pin----------------------------------------------//
int button = 4;
int button_state = 0; 
int analogPin = A0;
int Temperature = 0;
int Humidity = 0;
int methane = 0;


boolean eeprom_rst = false ;

//-------------------------Variable to store sensor data----------------------------//
int data1 = 0;
int data2 = 0;
int data3 = 0;
int temp_alarm = 0;
int tgs_alarm = 0;
int alarm_flag = 0;
//String did = "GP101";

String data = "";

volatile int watchdogCount = 0;
char sensorData[88];
char alertData[68];

//extern "C" {
//#include "user_interface.h"
//}



//--------------------Watchdog----------------------//

void ISRwatchdog() {
  watchdogCount++;
  if (watchdogCount == 180) {
    P_R_I_N_T("Watchdog bites!!!");
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

  watchdogCount = 0;
  P_R_I_N_T("");  
  button_state = digitalRead(button);
    
  unsigned long currentMillis = millis();
  thirdTick.attach(1.5, tick);
  if (button_state == HIGH) {
        digitalWrite(LED_BUILTIN, HIGH);
        P_R_I_N_T("LED HIGH");
        delay(250);
       }
     else {
      digitalWrite(LED_BUILTIN, LOW);
        previousMillis4 = millis();
        delay(250);
       }
  if (millis() - previousMillis4 > eeprom_clr_button_time) {
      P_R_I_N_T("Action !!!");
      eeprom_rst = true ;
      }

    if (eeprom_rst == true ){
        delay(1000);
        WiFi.disconnect();    
        delay(3000);
     if (WiFi.status() != WL_CONNECTED){
        P_R_I_N_T("wifi disconnected");
        delay(300);
        wifi_manager_reset ();
        delay(300);
        eeprom_rst= false ;
        }      
      }
  
  if(currentMillis - previousMillis3 > wifi_check_interval) {
    previousMillis3 = currentMillis;
    
//    P_R_I_N_T(previousMillis3); 
    
    if (WiFi.status() != WL_CONNECTED){ 
      //  set_wifi();
//      wifi_manager();
//      wifiManager.setConfigPortalTimeout(60);
      P_R_I_N_T(" unable to connect wifi, returning to loop");
      return loop();
    }
    else {
      P_R_I_N_T(" Wifi already connected");
    }
}

  if (!client.connected() && (WiFi.status() == WL_CONNECTED))
     {
      reconnect();
     }

   client.loop();

    //need to work here  
    data = sensor_data();
    
    
    data.toCharArray(sensorData,88);
    P_R_I_N_T("Sendor data: " + data);
    
    if(currentMillis - previousMillis > publish_interval) {
    
        previousMillis = currentMillis;
        P_R_I_N_T("Inside sensor data publish");
        
        if (!client.connected()){ reconnect();}
        
        int result = client.publish(device_data,sensorData); 
        
//        P_R_I_N_T(result);
        if (result ==1){
          P_R_I_N_T("Pulished successfully");
        }else{
            P_R_I_N_T("Unable to publish");
        }
    }// Timer ends
//  thirdTick.detach();  
}//LOOP ENDS

void rf_alarm(){
      
      delay(100);
      mySwitch.send(5555, 24);
      delay(500);
      P_R_I_N_T("RF is Triggered");
      
      if (!client.connected()){ reconnect();}
      int result = client.publish(device_data,sensorData);      
      if (result == 1){
          P_R_I_N_T("Pulished successfully");
          }else{
            P_R_I_N_T("Unable to publish");
           }
      delay(4000);
      
  }

//---------------------------------------Read Sensor Data---------------------------------------//
String sensor_data()
{ 
  P_R_I_N_T("Inside Sensor data read");
  
    data1 = temp(); 
   
    data2 = hum();  
      
    data3 = tgsVal();

//    if ( data1 > temp_upper_limit) {
//      temp_alarm= 1;
//      delay(100);
//      rf_alarm();
//      P_R_I_N_T("temp outta hand"); 
//      alarm_flag = 1;
//    }

    if ( data3 > tgs_upper_limit) {
      tgs_alarm= 1;
      delay(100);
      rf_alarm();
      P_R_I_N_T("LPG outta hand"); 
      alarm_flag = 1; 
    }

  String msg2 = "";
  msg2 = msg2 + "{\"did\":" + "\""+ did + "\""+ "," + "\"tmp\":" + data1 + "," + "\"hum\":" + data2 + "," + "\"ch4\":" + data3 + "," + "\"tmp_alrt\":" + temp_alarm +  "," + "\"tgs_alrt\":" + tgs_alarm + "," + "\"src\":" + "\""+ "sensor" + "\""+  "}";

if (alarm_flag == 1){
//  data = msg2;
  msg2.toCharArray(sensorData,100);
  int result = client.publish(device_data,sensorData);    
     if (result ==1){
          P_R_I_N_T("Pulished successfully");
          delay(500);
        }else{
            P_R_I_N_T("Unable to publish");
        }
  }

  temp_alarm = 0;
  tgs_alarm = 0;
  alarm_flag = 0;
  delay(200);       //........ 0.2 sec delay...........//
  return msg2;

}


//
//void deep_sleep_new(int stime){
//  P_R_I_N_T("Device is going into DEEEEEEEP_Sleep");
//  Serial.print("Sleep Time(second):");
//  P_R_I_N_T(stime);
//  ESP.deepSleep(stime * 1000000); //sleep for 10 seconds
//  delay (500);
//}


