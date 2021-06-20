// Import required libraries
#include <WiFi.h>
//#include <ETH.h>

//void connectToWiFi(const char * ssid, const char * pwd);
//void WiFiEvent(WiFiEvent_t event) ;
void WiFiStationDisconnected( WiFiEvent_t event, WiFiEventInfo_t info );
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);

// Replace with your network credentials
const char* ssid = "The Internet";
const char* password = "ledigsilva1";

// Set your Static IP address
IPAddress local_IP(192, 168, 0, 232);
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(1, 1, 1, 1);

void connectWiFi(){
  // Connect to Wi-Fi
  Serial.print("starting..."); 

  if(!WiFi.config(local_IP, gateway, subnet, dns)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.begin(ssid, password);
}

//
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.print(" IP: ");
  Serial.println(WiFi.localIP());
}

void WiFiStationDisconnected( WiFiEvent_t event, WiFiEventInfo_t info ){
  Serial.print("conn lost: ");
  Serial.print(info.disconnected.reason);
  Serial.print(" _ ");

  //WiFi.disconnect();
//  delay(1000);
//  connectWiFi();
//  while (WiFi.status() != WL_CONNECTED){
//    Serial.print(".");
//    delay(1000);
//  }
  
  //Then we will call WiFi.disconnect() function with true as a parameter inside it. 
  // This will delete all the previous network credentials stored in the board.
  // delete old config
  //WiFi.disconnect(true);

  //delay(1000);
  
  WiFi.disconnect();
  WiFi.reconnect();

  // WiFi.begin(ssid, password); // without disconnect(true)
}



void setupWiFi(){
  // Connect to Wi-Fi
  Serial.print("WiFi setup... ");
  
  // delete old config
  //WiFi.disconnect(true);

  //delay(1000);

  //WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent(WiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
  
  connectWiFi();
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(1000);
  }
  delay(1000);
}
