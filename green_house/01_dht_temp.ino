#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTPIN 25     // Digital pin connected to the DHT sensor
#define NTRIES 10      // number of tries in case of a failed measure
   
// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);
int main_fan_goal_temp;
float readDHTTemperature() {
  return main_fan_goal_temp;//26.5;
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float t;
  for(int i = 0; i < NTRIES; i++){
    // Read temperature as Celsius (the default)
    t = dht.readTemperature();
    // Check if any reads failed and exit early (to try again).
    if (!isnan(t)) {
      return t;
    }
    delay(5);
  }     
  Serial.println("Failed to read temp from DHT sensor!");
  return t;
}

float readDHTHumidity() {
  return 46.5;
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h;
  for(int i = 0; i < NTRIES; i++){
    h = dht.readHumidity();
    if (!isnan(h)) {
      return h;
    }
    delay(5);
  }
  Serial.println("Failed to read humidity from DHT sensor!");
  return h;
}


void readDHTTemperatureHumidity(float * rt, float * rh) {
  *rt = readDHTTemperature();
  *rh = readDHTHumidity();
  return;
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float t = NAN;
  float h = NAN;
  for(int i = 0; i < NTRIES; i++){
    if (isnan(h))
      h = dht.readHumidity();
      
    if (isnan(t))
      t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)){
      //Serial.println("TH "+String(t)+","+String(h)+" i"+String(i));
      *rt = t;
      *rh = h;
      return;
    }
    delay(5);
  }
  Serial.println("Failed to read humidity from DHT sensor!");
  return;
}

void setupDHT(){
  dht.begin(); 
}
