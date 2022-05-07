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

String convertToUrlStringParams(String keys[], String vals[], size_t nElems){
  String kv_string = "";
  for(int i=0; i<nElems; i++){
    kv_string += keys[i];
    kv_string += "=";
    kv_string += vals[i];
    if(i<nElems-1)
      kv_string += "&";
  }
  return kv_string;
}

void logToGS(String params_string) {
  
  Serial.print("[HTTPS] start...");

  WiFiClientSecure client;

  client.setTimeout(20000);
  client.setInsecure();
  
  String url = String(google_sheets_url) + "?" + params_string;

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


float sum_temp = 0.0;
float sum_humid = 0.0;
int avg_i = 0;
void updateTempHumidAvg(float temp, float humid) {
  if(!isnan(temp) && !isnan(humid)){
    sum_temp += temp;
    sum_humid += humid;
    avg_i++;
    //Serial.println("Updated temp/humid avg - "+String(sum_temp)+" / "+String(sum_humid)+"  "+String(avg_i));
  }
}

void getResetTemperatureHumidityAvg(float * rt, float * rh) {
  if(avg_i != 0){
    *rt = sum_temp/avg_i;
    *rh = sum_humid/avg_i;
    avg_i = 0;
    sum_temp = 0.0;
    sum_humid = 0.0;
  } else {
    *rt = 0.0;
    *rh = 0.0;
  }
}

///// SQL
//
//#include <MySQL_Connection.h>
//#include <MySQL_Cursor.h>
////#include <WiFi.h>
////
////const char* ssid = "your wifi name";
////const char* password = "your wifi password";
//
//char userSQL[] = "root";//"root"; // MySQL user login username
//char passwordSQL[] = ""; // MySQL user login password
//
//
//// CREATE TABLE `ghouse`.`sensorvals` ( `datetime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP , `temp` FLOAT NOT NULL , `humd` FLOAT NOT NULL , `fanspeed` SMALLINT NOT NULL );
//
//// INSERT INTO `id17187452_esp32ghs`.`sensorvals` (`datetime`, `temp`, `humd`, `fanspeed`) VALUES (current_timestamp(), '25.6', '84.3', '125');
//
//// INSERT INTO `ghouse`.`sensorvals` (`datetime`, `temp`, `humd`, `fanspeed`) VALUES ('2013-08-05 18:19:03', '25.6', '84.3', '125');
//
//char INSERT_SQL[] = "INSERT INTO `ghouse`.`sensorvals` (`datetime`, `temp`, `humd`, `fanspeed`) VALUES (current_timestamp(), '25.6', '84.3', '125');";
//
//IPAddress sql_server_addr(34,65,16,41);// your MySQL ip like (8, 8, 8, 8); // IP of the MySQL server here
//
//
////WiFiServer  server(80);
////void setup() {
////  //Código de configuração aqui
////  Serial.begin(115200);
////
////  connectToNetwork();
////  mySqlLoop();
////}
////
////void loop() {
////}
////void connectToNetwork() {
////  WiFi.begin(ssid, password);
//// 
////  while (WiFi.status() != WL_CONNECTED) {
////    delay(1000);
////    Serial.println("Establishing connection to WiFi..");
////  }
//// 
////  Serial.println("Connected to network");
//// 
////}
//void mySqlLoop(){
//  //WiFiClient client = getWiFiClient();
//  WiFiClient client;
//  MySQL_Connection conn((Client *)&client);
//  if (conn.connect(sql_server_addr, 3306, userSQL, passwordSQL)) {
//    Serial.println("Database connected.");
//  }
//  else{
//    Serial.println("DB Connection failed!");
//    //return;
//  }
//  // Initiate the query class instance
//  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
//  // Execute the query
//  cur_mem->execute(INSERT_SQL);
//  // Note: since there are no results, we do not need to read any data
//  // Deleting the cursor also frees up memory used
//  delete cur_mem;
//  Serial.println("closing connection\n");
//  //client.stop();
//}

// POST PHP

#include <WiFi.h>
#include <HTTPClient.h>

const char* serverName = "http://esp32gh.000webhostapp.com/post-esp-data.php";

String apiKeyValue = "Z2E58eFfzfBb";

void postdataphp(String params_string) {
  WiFiClient wifi_client;
  HTTPClient http;

  wifi_client.setTimeout(20000);
  
  // Your Domain name with URL path or IP address with path
  http.begin(wifi_client, serverName);
  
  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  // Prepare your HTTP POST request data
  String datetime_str = getDateTime(true);
  String httpRequestData = "api_key=" + apiKeyValue + "&datetime=" + datetime_str + "&"+ params_string;//"&temp=" + String(99.9) + "&humd=" + String(69.69) + "&fanspeed=" + String(233) + "";
//  Serial.print("httpRequestData: ");
//  Serial.println(httpRequestData);

  // Send HTTP POST request
  int httpResponseCode = http.POST(httpRequestData);
   
  // If you need an HTTP request with a content type: text/plain
  //http.addHeader("Content-Type", "text/plain");
  //int httpResponseCode = http.POST("Hello, World!");
  
  // If you need an HTTP request with a content type: application/json, use the following:
  //http.addHeader("Content-Type", "application/json");
  //int httpResponseCode = http.POST("{\"value1\":\"19\",\"value2\":\"67\",\"value3\":\"78\"}");
      
  if(httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
//    String payload = http.getString();
//    Serial.println(payload);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}


void logTempHumidToGS(bool get_avg, int fan_speed, int avg_light_power) {
  
  float temperature = NAN;
  float humidity = NAN;
  if(get_avg){
    getResetTemperatureHumidityAvg(&temperature, &humidity);
  } else {
    readDHTTemperatureHumidity(&temperature, &humidity);
  }
  
//  String names[2]  =  {"temp", "humid"};
//  String vals[2] = {String(temperature), String(humidity)};
//
//    
//  String params_string = convertToUrlStringParams(names, vals, 2);

  //logToGS(params_string);

  String names[4]  =  {"temp", "humid", "fanspeed", "avglight"};
  String vals[4] = {String(temperature), String(humidity), String(fan_speed), String(avg_light_power)};

  String params_string = convertToUrlStringParams(names, vals, 4);
  postdataphp(params_string);
}
