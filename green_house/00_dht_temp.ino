#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTPIN 25     // Digital pin connected to the DHT sensor
#define NTRIES 3      // number of tries in case of a failed measure
   
// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

String readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  for(int i = 0; i < NTRIES; i++){
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Check if any reads failed and exit early (to try again).
    if (!isnan(t)) {
      return String(t);
    }
    delay(5);
  }     
  Serial.println("Failed to read temp from DHT sensor!");
  return "--";
}

String readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  for(int i = 0; i < NTRIES; i++){
    float h = dht.readHumidity();
    if (!isnan(h)) {
      return String(h);
    }
    delay(5);
  }
  Serial.println("Failed to read humidity from DHT sensor!");
  return "--";
}

void setupDHT(){
  dht.begin(); 
}
