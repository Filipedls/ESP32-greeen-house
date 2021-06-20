//#include <HTTPClient.h>
#include <WiFiClientSecure.h>

//
////  google sheets macro URL
//const char* google_sheets_url = "https://script.google.com/macros/s/AKfycbxBkJ0qJlmY2SeqlwSqxfXrToO65Y4RTOIG3It-T8s_fPFxz1zygQgOa4kw85bzvm3t/exec";

// openssl s_client -showcerts -connect script.google.com:443
// Certificate chain 1
//const char* root_ca= \
//"-----BEGIN CERTIFICATE-----\n" \
//"MIIES...1yg==\n" \
//"-----END CERTIFICATE-----";
//
//void logToGS() {
//    HTTPClient http;
//    String URL = String(google_sheets_url);
//
//    Serial.print("[HTTP] begin...");
//    //Serial.println(URL);
//    // access to your Google Sheets
//    // configure target server and url
//    http.begin(URL, root_ca);
//
//    Serial.print(" GET...");
//    // start connection and send HTTP header
//    int httpCode = http.GET();
//
//    // httpCode will be negative on error
//    if(httpCode > 0) {
//        // HTTP header has been send and Server response header has been handled
//        Serial.print(" code: ");
//        Serial.println(httpCode);
//
////        // file found at server
////        if(httpCode == HTTP_CODE_OK) {
////            String payload = http.getString();
////            Serial.println(payload);
////        }
//    } else {
//        Serial.print(" failed, error: ");
//        Serial.println(http.errorToString(httpCode).c_str());
//    }
//    http.end();
//}
//


const char* host = "script.google.com";
const char* google_sheets_url = "/macros/s/AKfycbxBkJ0qJlmY2SeqlwSqxfXrToO65Y4RTOIG3It-T8s_fPFxz1zygQgOa4kw85bzvm3t/exec";

const uint16_t httpsPort = 443;

//WiFiClientSecure client;
//// needs wifi and time
//void setupLogs() {
//  // Load root certificate in DER format into WiFiClientSecure object
//  //client.setCACert(root_ca);
//
//  client.setTimeout(20000);
//  client.setInsecure();
//}

void logToGS(String params[], float vals[], size_t nElems) {
  
  Serial.print("[HTTPS] start...");

  WiFiClientSecure client;

  client.setTimeout(20000);
  client.setInsecure();
  
  String url = String(google_sheets_url);
  url += "?";
  for(int i=0; i<nElems; i++){
    url += params[i];
    url += "=";
    url += vals[i];
    if(i<nElems-1)
      url += "&";
  }

  // Connect to remote server
  Serial.print("connecting to ");
  Serial.print(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println(" connection failed!");
    return;
  }
  
  //Serial.println(url);

  client.print(String("GET https://") + host + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP32\r\n" +
               "Connection: close\r\n\r\n"); // keep-alive, close

  Serial.print(" sent...");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.print(" received");
      break;
    }
  }
  // if there are incoming bytes available
  // from the server, read them and print them:
//  while (client.available()) {
//    char c = client.read();
//    Serial.write(c);
//  }
  client.stop();
  Serial.println(" done");
}

void logTempHumidToGS() {
  float temperature = readDHTTemperature();
  float humidity = readDHTHumidity();

  String names[2]  =  {"temp", "humid"};
  float vals[2] = {temperature, humidity};

  logToGS(names, vals, 2);
}
