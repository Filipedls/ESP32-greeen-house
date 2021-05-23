#include <HTTPClient.h>



void logTempHumidToGS() {
    HTTPClient http;
    String URL = String(google_sheets_url);

    String temperature = readDHTTemperature();
    String humidity = readDHTHumidity();
    
    URL += "?temp=";
    URL += temperature;
    URL += "&humid=";
    URL += humidity;

    Serial.print("[HTTP] begin...");
    //Serial.println(URL);
    // access to your Google Sheets
    // configure target server and url
    http.begin(URL, root_ca);

    Serial.print(" GET...");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if(httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.print(" code: ");
        Serial.println(httpCode);

//        // file found at server
//        if(httpCode == HTTP_CODE_OK) {
//            String payload = http.getString();
//            Serial.println(payload);
//        }
    } else {
        Serial.print(" failed, error: ");
        Serial.println(http.errorToString(httpCode).c_str());
    }
}
