// Import required libraries
#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "The Internet";
const char* password = "ledigsilva1";

// Set your Static IP address
IPAddress local_IP(192, 168, 0, 232);
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(1, 1, 1, 1);

void setupWiFi(){
  // Connect to Wi-Fi
  Serial.print("WiFi: starting...");
  do {
    if(!WiFi.config(local_IP, gateway, subnet, dns)) {
      Serial.println("STA Failed to configure");
    }
    WiFi.begin(ssid, password);
    Serial.print(" ");
    for(int i = 0; i < 4; i++){   
      if (WiFi.status() == WL_CONNECTED) {
        break;
      }
      Serial.print(".");
      delay(700);
    }
  } while (WiFi.status() != WL_CONNECTED);
  Serial.print(" IP ");  
  Serial.println(WiFi.localIP()); 
}
