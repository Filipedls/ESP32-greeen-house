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

void setupWiFi(){
  // Connect to Wi-Fi
  Serial.print("WiFi: ");
  do {
    Serial.print("starting");
    if(!WiFi.config(local_IP, gateway, subnet)) {
      Serial.println("STA Failed to configure");
    }
    WiFi.begin(ssid, password);
    Serial.print(", connecting.");
    for(int i = 0; i < 5; i++){   
      if (WiFi.status() == WL_CONNECTED) {
        break;
      }
      Serial.print(".");
      delay(1000);
    }
    //Serial.println("Could not connect to WiFi! :( Restarting WiFi!");
  } while (WiFi.status() != WL_CONNECTED);

  Serial.print("  sucessful :) IP: ");  
  Serial.println(WiFi.localIP()); 
}
