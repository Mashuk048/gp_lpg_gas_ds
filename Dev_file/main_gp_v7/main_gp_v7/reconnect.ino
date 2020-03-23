//------------------------------------------------While client not conncected---------------------------------//

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) 
  { 
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if your MQTT broker has clientID,username and password
    //please change following line to if (client.connect(clientId,userName,passWord))

    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass))
    {
      P_R_I_N_T("connected ");
//      delay(1500);
     //once connected to MQTT broker, subscribe command if any
     //----------------------Subscribing to required topics-----------------------//
      client.subscribe(device_config);
      delay(500);
      client.subscribe(device_response);
      delay(1000);
                   
    }else {     
      Serial.print("failed, rc=");
      Serial.print(client.state());
      P_R_I_N_T(" try again");
      mqttTryCounter++;
      delay(500);
      P_R_I_N_T(mqttTryCounter);    
    }


    if(mqttTryCounter==4){
      
      P_R_I_N_T("Failed MQTT connection...");
//      client.setServer(public_mqtt_server,mqttPort);//Connecting to broker
      P_R_I_N_T(mqttTryCounter);
      mqttTryCounter=0;
      if (!client.connected()){
        return loop();
        }    
     }
     
  }
  
}
