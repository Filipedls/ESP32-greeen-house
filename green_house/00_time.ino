#include "time.h"

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";

// Variable to save current epoch time
unsigned long epochTime; 

// Function that gets current epoch time
struct tm getTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return timeinfo;
  }
  //time(&now);
  //Serial.println(&timeinfo, "It's %A, %B %d %Y %H:%M:%S");
  return timeinfo;
}

int getHour() {
  struct tm timeinfo;
  timeinfo = getTime();
   //GET DATE
  //Get full weekday name
//  char weekDay[10];
//  strftime(weekDay, sizeof(weekDay), "%a", &timeinfo);
//  //Get day of month
//  char dayMonth[4];
//  strftime(dayMonth, sizeof(dayMonth), "%d", &timeinfo);
//  //Get abbreviated month name
//  char monthName[5];
//  strftime(monthName, sizeof(monthName), "%b", &timeinfo);
//  //Get year
//  char year[6];
//  strftime(year, sizeof(year), "%Y", &timeinfo);
//  //Get hour (24 hour format)
//  char hour[4];
//  strftime(hour, sizeof(hour), "%H", &timeinfo);
  //Get minute
//  char minute[4];
//  strftime(minute, sizeof(minute), "%M", &timeinfo);  
  return (int) timeinfo.tm_hour;//String(hour).toInt();
}

String getDateTime(){
  struct tm timeinfo;
  timeinfo = getTime();
   //GET DATE
  char dateTime[35];
  strftime(dateTime, sizeof(dateTime), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  return String(dateTime);
}

void setupTime() {
  //before  do: initWiFi();
  configTime(0, 0, ntpServer);
  // Berlin timeZone
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
}
