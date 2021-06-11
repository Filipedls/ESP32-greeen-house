//#include <HTTPClient.h>
#include <WiFiClientSecure.h>

//
////  google sheets macro URL
//const char* google_sheets_url = "https://script.google.com/macros/s/AKfycbxBkJ0qJlmY2SeqlwSqxfXrToO65Y4RTOIG3It-T8s_fPFxz1zygQgOa4kw85bzvm3t/exec";

// openssl s_client -showcerts -connect script.google.com:443
// Certificate chain 1
//const char* root_ca= \
//"-----BEGIN CERTIFICATE-----\n" \
//"MIIESjCCAzKgAwIBAgINAeO0mqGNiqmBJWlQuDANBgkqhkiG9w0BAQsFADBMMSAw\n" \
//"HgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEGA1UEChMKR2xvYmFs\n" \
//"U2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjAeFw0xNzA2MTUwMDAwNDJaFw0yMTEy\n" \
//"MTUwMDAwNDJaMEIxCzAJBgNVBAYTAlVTMR4wHAYDVQQKExVHb29nbGUgVHJ1c3Qg\n" \
//"U2VydmljZXMxEzARBgNVBAMTCkdUUyBDQSAxTzEwggEiMA0GCSqGSIb3DQEBAQUA\n" \
//"A4IBDwAwggEKAoIBAQDQGM9F1IvN05zkQO9+tN1pIRvJzzyOTHW5DzEZhD2ePCnv\n" \
//"UA0Qk28FgICfKqC9EksC4T2fWBYk/jCfC3R3VZMdS/dN4ZKCEPZRrAzDsiKUDzRr\n" \
//"mBBJ5wudgzndIMYcLe/RGGFl5yODIKgjEv/SJH/UL+dEaltN11BmsK+eQmMF++Ac\n" \
//"xGNhr59qM/9il71I2dN8FGfcddwuaej4bXhp0LcQBbjxMcI7JP0aM3T4I+DsaxmK\n" \
//"FsbjzaTNC9uzpFlgOIg7rR25xoynUxv8vNmkq7zdPGHXkxWY7oG9j+JkRyBABk7X\n" \
//"rJfoucBZEqFJJSPk7XA0LKW0Y3z5oz2D0c1tJKwHAgMBAAGjggEzMIIBLzAOBgNV\n" \
//"HQ8BAf8EBAMCAYYwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMBIGA1Ud\n" \
//"EwEB/wQIMAYBAf8CAQAwHQYDVR0OBBYEFJjR+G4Q68+b7GCfGJAboOt9Cf0rMB8G\n" \
//"A1UdIwQYMBaAFJviB1dnHB7AagbeWbSaLd/cGYYuMDUGCCsGAQUFBwEBBCkwJzAl\n" \
//"BggrBgEFBQcwAYYZaHR0cDovL29jc3AucGtpLmdvb2cvZ3NyMjAyBgNVHR8EKzAp\n" \
//"MCegJaAjhiFodHRwOi8vY3JsLnBraS5nb29nL2dzcjIvZ3NyMi5jcmwwPwYDVR0g\n" \
//"BDgwNjA0BgZngQwBAgIwKjAoBggrBgEFBQcCARYcaHR0cHM6Ly9wa2kuZ29vZy9y\n" \
//"ZXBvc2l0b3J5LzANBgkqhkiG9w0BAQsFAAOCAQEAGoA+Nnn78y6pRjd9XlQWNa7H\n" \
//"TgiZ/r3RNGkmUmYHPQq6Scti9PEajvwRT2iWTHQr02fesqOqBY2ETUwgZQ+lltoN\n" \
//"FvhsO9tvBCOIazpswWC9aJ9xju4tWDQH8NVU6YZZ/XteDSGU9YzJqPjY8q3MDxrz\n" \
//"mqepBCf5o8mw/wJ4a2G6xzUr6Fb6T8McDO22PLRL6u3M4Tzs3A2M1j6bykJYi8wW\n" \
//"IRdAvKLWZu/axBVbzYmqmwkm5zLSDW5nIAJbELCQCZwMH56t2Dvqofxs6BBcCFIZ\n" \
//"USpxu6x6td0V7SvJCCosirSmIatj/9dSSVDQibet8q/7UK4v4ZUN80atnZz1yg==\n" \
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
    Serial.println("connection failed");
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
