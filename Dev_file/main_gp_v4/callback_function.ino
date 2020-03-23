//-----------------------Callback function-------------------------------------//

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  
//-------------------------------------resetting the device---------------------//

if(strcmp(topic, device_config) == 0){
  
  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload, length);

  String devId = doc["did"];
  int cmd = doc["cmd"];
  int g_limit = doc["gas_limit"];
  int t_limit = doc["temp_limit"];

  Serial.println(devId);
  Serial.println(cmd);
  Serial.println(g_limit);
  Serial.println(t_limit);

  
  if(( devId == did) || (devId == "GP000")){
      if(cmd == 1){
        Serial.println("RESET");

        char buffer[256];
        doc["status"] = "reset_success";
        size_t n = serializeJson(doc, buffer);
        client.publish(device_response,buffer, n);
        delay(2000);
        ESP.reset();
      }
      else{
        Serial.println("INVALID RESET COMMAND");

        char buffer[256];
        doc["status"] = "reset_failed";
        size_t n = serializeJson(doc, buffer);
        client.publish(device_response,buffer, n);
        delay(2000);
      }

  if( g_limit >=5 && g_limit <= 1000){ //sleep time within 2 hours
          Serial.println("GAS LIMIT UPDATED");
          tgs_upper_limit = g_limit;
          delay(2000);
          char buffer[256];
          doc["status"] = "gas_limit_success";
          size_t n = serializeJson(doc, buffer);
          client.publish(device_response,buffer, n);  
        }
        else{
          Serial.println("INVALID GAS LIMIT");
          char buffer[256];
          doc["status"] = "gas_limit_failed";
          size_t n = serializeJson(doc, buffer);
          client.publish(device_response,buffer, n);
          delay(1000);
        }
  if( t_limit >=5 && t_limit <= 60){ //sleep time within 2 hours
          Serial.println("TEMPARETURE LIMIT UPDATED");
          temp_upper_limit = t_limit;
          delay(2000);
          char buffer[256];
          doc["status"] = "temp_limit_success";
          size_t n = serializeJson(doc, buffer);
          client.publish(device_response,buffer, n);  
        }
        else{
          Serial.println("INVALID TEMPARETURE LIMIT");
          char buffer[256];
          doc["status"] = "temp_limit_failed";
          size_t n = serializeJson(doc, buffer);
          client.publish(device_response,buffer, n);
          delay(1000);
        }  
  }

else{
      //do nothing
  }
  
}//strcomp ends
}
//  else if( did == "GP000" ){ //MAGIC CODE
//      if(cmd == 1){
//        Serial.println("RESET");
//
//        char buffer[256];
//        doc["status"] = "success";
//        size_t n = serializeJson(doc, buffer);
//        client.publish(device_response,buffer, n);
//        delay(2000);
//        ESP.reset();
//      }
//      else{
//        Serial.println("INVALID RESET COMMAND");
//
//        char buffer[256];
//        doc["status"] = "failed";
//        size_t n = serializeJson(doc, buffer);
//        client.publish(device_response,buffer, n);
//        delay(2000);
//      }
//  }
//  else{
//      //do nothing
//  }
//  
//}//strcomp ends

//---------------------gas upper limit-------------------//
//
// if(strcmp(topic, "gphouse/dbox/gas_limit") == 0){
//    
//    StaticJsonDocument<256> doc;
//    deserializeJson(doc, payload, length);
//  
//    String devId = doc["did"];
//    int g_limit = doc["gas_limit"];
//    Serial.println(devId);
//    Serial.println(g_limit);
//    Serial.println("GAS MSG RECEIVED");
//   
//      
//    if( devId == did){
//        if( g_limit >=5 && g_limit <= 1000){ //sleep time within 2 hours
//          Serial.println("GAS LIMIT UPDATED");
//          tgs_upper_limit = g_limit;
//          delay(2000);
//          char buffer[256];
//          doc["status"] = "success";
//          size_t n = serializeJson(doc, buffer);
//          client.publish(device_response,buffer, n);  
//        }
//        else{
//          Serial.println("INVALID GAS LIMIT");
//          char buffer[256];
//          doc["status"] = "failed";
//          size_t n = serializeJson(doc, buffer);
//          client.publish(device_response,buffer, n);
//          delay(1000);
//        }
//    }else if( devId == "GP000" ){
//        if(5 < g_limit < 1000){ //sleep time within 2 hours
//          Serial.println("GAS LIMIT FOR ALL DeVICE UPDATED");
//          tgs_upper_limit = g_limit;
//          delay(2000);
//          char buffer[256];
//          doc["status"] = "success";
//          size_t n = serializeJson(doc, buffer);
//          client.publish(device_response,buffer, n);
//        }
//        else{
//          Serial.println("INVALID GAS LIMIT");
//          char buffer[256];
//          doc["status"] = "failed";
//          size_t n = serializeJson(doc, buffer);
//          client.publish(device_response,buffer, n);
//          delay(1000);
//        }
//    }else{
//         //do nothing
//    }
// }//strcomp ends
//
////----------------------tempareture-----------------//
//
// if(strcmp(topic, "gphouse/dbox/temp_limit") == 0){
//    
//    StaticJsonDocument<256> doc;
//    deserializeJson(doc, payload, length);
//  
//    String devId = doc["did"];
//    int t_limit = doc["temp_limit"];
//    Serial.println(devId);
//    Serial.println(t_limit);
//    Serial.println("TEMP MSG RECEIVED");
//   
//      
//    if( devId == did){
//        if( t_limit >=5 && t_limit <= 50){ //sleep time within 2 hours
//          Serial.println("TEMPARETURE LIMIT UPDATED");
//          temp_upper_limit = t_limit;
//          delay(2000);
//          char buffer[256];
//          doc["status"] = "success";
//          size_t n = serializeJson(doc, buffer);
//          client.publish(device_response,buffer, n);  
//        }
//        else{
//          Serial.println("INVALID TEMPARETURE LIMIT");
//          char buffer[256];
//          doc["status"] = "failed";
//          size_t n = serializeJson(doc, buffer);
//          client.publish(device_response,buffer, n);
//          delay(1000);
//        }
//    }else if( devId == "GP000" ){
//        if(t_limit >=5 && t_limit <= 50){ //sleep time within 2 hours
//          Serial.println("TEMPARETURE LIMIT FOR ALL DeVICE UPDATED");
//          temp_upper_limit = t_limit;
//          delay(2000);
//          char buffer[256];
//          doc["status"] = "success";
//          size_t n = serializeJson(doc, buffer);
//          client.publish(device_response,buffer, n);
//        }
//        else{
//          Serial.println("INVALID TEMPARETURE LIMIT");
//          char buffer[256];
//          doc["status"] = "failed";
//          size_t n = serializeJson(doc, buffer);
//          client.publish(device_response,buffer, n);
//          delay(1000);
//        }
//    }else{
//         //do nothing
//    }
// }
//
// 
//}//Callback ends

