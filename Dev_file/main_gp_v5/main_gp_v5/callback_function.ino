//-----------------------Callback function-------------------------------------//

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  
//-------------------------------------resetting the device---------------------//

if(strcmp(topic, device_config) == 0){
  
  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload, length);

  String devId = doc["did"];
  int rst = doc["reset"];
  int g_limit = doc["gas_limit"];
  int t_limit = doc["temp_limit"];
  int pub_intrvl = doc["publish_interval"];
  
  Serial.println(devId);
  Serial.println(rst);
  Serial.println(g_limit);
  Serial.println(t_limit);
  Serial.println(pub_intrvl);

  
  if(( devId == did) || (devId == "GP000")){
      if(rst == 1){
        Serial.println("RESET");

        char buffer[256];
        doc["status"] = "success";
        size_t n = serializeJson(doc, buffer);
        client.publish(device_response,buffer, n);
        delay(2000);
        ESP.reset();
      }
      else if(rst == 0){
        Serial.println("DEVICE NOT RESETED");
        }
      else{
        Serial.println("INVALID RESET COMMAND");

        char buffer[256];
        doc["status"] = "failed";
        size_t n = serializeJson(doc, buffer);
        client.publish(device_response,buffer, n);
        delay(1500);
      }

  if( g_limit >=5 && g_limit <= 1000){ //sleep time within 2 hours
          Serial.println("GAS LIMIT UPDATED");
          tgs_upper_limit = g_limit;
          delay(2000);
          char buffer[256];
          doc["status"] = "success";
          size_t n = serializeJson(doc, buffer);
          client.publish(device_response,buffer, n);  
        }
        else{
          Serial.println("INVALID GAS LIMIT");
          char buffer[256];
          doc["status"] = "failed";
          size_t n = serializeJson(doc, buffer);
          client.publish(device_response,buffer, n);
          delay(1000);
        }
  if( t_limit >=5 && t_limit <= 60){ //sleep time within 2 hours
          Serial.println("TEMPARETURE LIMIT UPDATED");
          temp_upper_limit = t_limit;
          delay(2000);
          char buffer[256];
          doc["status"] = "success";
          size_t n = serializeJson(doc, buffer);
          client.publish(device_response,buffer, n);  
        }
        else{
          Serial.println("INVALID TEMPARETURE LIMIT");
          char buffer[256];
          doc["status"] = "failed";
          size_t n = serializeJson(doc, buffer);
          client.publish(device_response,buffer, n);
          delay(1000);
        }

  if( pub_intrvl >=15 && pub_intrvl <= 7200){ //sleep time within 2 hours
          Serial.println("DATA PUBLISH INTERVAL UPDATED");
          publish_interval = ((pub_intrvl*1000) -2.5);
          delay(2000);
          char buffer[256];
          doc["status"] = "success";
          size_t n = serializeJson(doc, buffer);
          client.publish(device_response,buffer, n);  
        }
        else{
          Serial.println("INVALID DATA PUBLISH INTERVAL");
          char buffer[256];
          doc["status"] = "failed";
          size_t n = serializeJson(doc, buffer);
          client.publish(device_response,buffer, n);
          delay(1000);
        }
  }

else{
      //do nothing
  }
  
}//strcomp ends
}//callback function ends

