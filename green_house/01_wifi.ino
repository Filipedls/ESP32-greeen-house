// Import required libraries
#include <WiFi.h>

void WiFiStationDisconnected( WiFiEvent_t event, WiFiEventInfo_t info );
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);

// Replace with your network credentials
String ssid;
String password;

// Set your Static IP address
String IP_addr, gateway_str;


void connectWiFi(){
  IPAddress local_IP;
  IPAddress gateway;
  
  if (local_IP.fromString(IP_addr) & gateway.fromString(gateway_str)) { // try to parse into the IPAddress
    //Serial.println(local_IP); // print the parsed IPAddress 
  } else {
    Serial.println("Unparsable IP/gateway! IP: "+IP_addr+" Gateway: "+gateway);
  }

  // Set your Gateway IP address
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(1, 1, 1, 1);
  // Connect to Wi-Fi
  Serial.print("starting... SSID: " + ssid + "  P: " + password); 

  if(!WiFi.config(local_IP, gateway, subnet, dns)) {
    Serial.println("/!\ STA Failed to configure!");
  }
  WiFi.begin(ssid.c_str(), password.c_str());
}

//
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.print(" IP: ");
  Serial.println(WiFi.localIP());
}

void WiFiStationDisconnected( WiFiEvent_t event, WiFiEventInfo_t info ){
  Serial.print("conn lost: ");
  Serial.print(info.wifi_sta_disconnected.reason);
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

// signal strenght
// WiFi.RSSI();

//WiFiClient getWiFiClient(){
//  WiFiClient client;
//  return client;
//}


void setupWiFi(){
  // Connect to Wi-Fi
  Serial.print("WiFi setup... ");

  ssid = getWiFiSSID(); 
  password = getWiFiPass();
  IP_addr = getWiFiIP();
  gateway_str = getWiFigateway();

  // delete old config
  //WiFi.disconnect(true);

  //delay(1000);

  //WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
  //WiFi.onEvent(WiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);
  //WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);

  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  // WiFiEventId_t eventID = WiFi.onEvent(
  //   [](WiFiEvent_t event, WiFiEventInfo_t info){
  //       Serial.print("WiFi lost connection. Reason: ");
  //       Serial.println(info.wifi_sta_disconnected.reason);
  //       WiFi.disconnect();
  //       WiFi.reconnect();
  //   }, 
  //   WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED
  // );
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  
  connectWiFi();
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(800);
  }
  delay(500);
}
