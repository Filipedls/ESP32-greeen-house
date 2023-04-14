// TODO log buffer, for when it fails

#include <WiFi.h>
#include <HTTPClient.h>
//#include <WiFiClientSecure.h>


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
//// CREATE TABLE `ghouse`.`sensorvals` ( `datetime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP , `temp` FLOAT NOT NULL , `humd` FLOAT NOT NULL , `fanspeed` SMALLINT NOT NULL );
//
//// INSERT INTO `id17187452_esp32ghs`.`sensorvals` (`datetime`, `temp`, `humd`, `fanspeed`) VALUES (current_timestamp(), '25.6', '84.3', '125');
//
//// INSERT INTO `ghouse`.`sensorvals` (`datetime`, `temp`, `humd`, `fanspeed`) VALUES ('2013-08-05 18:19:03', '25.6', '84.3', '125');
//

// POST PHP

// const char* serverName; // = "http://esp32gh.000webhostapp.com/post-esp-data.php";

String serverName;
String apiKeyValue; // = "Z2E58eFfzfBb";


void postdataphp(String params_string) {
  WiFiClient wifi_client;
  HTTPClient http;

  wifi_client.setTimeout(50000);
  
  // Your Domain name with URL path or IP address with path
  http.begin(wifi_client, serverName.c_str() );
  
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


int logToCloudFlag;
void setLogCloudFlag(int val){
  logToCloudFlag = val;
  setMemLogCloudFlag(val);
}

void logTempHumidToGS(bool get_avg, int fan_speed, int avg_light_power) {

  float temperature = NAN;
  float humidity = NAN;
  if(get_avg){
    getResetTemperatureHumidityAvg(&temperature, &humidity);
  } else {
    readDHTTemperatureHumidity(&temperature, &humidity);
  }

  if(logToCloudFlag == 0){
    Serial.println("Not logging to cloud. Avg temp/humid avg - "+String(temperature)+" / "+String(humidity));
    return;
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


void setupLogs() {
  logToCloudFlag = getLogCloudFlag();
  serverName = getLogServerURL();
  apiKeyValue = getLogServerAPIkey();
}
