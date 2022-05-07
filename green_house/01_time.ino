#include "time.h"

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";

// Variable to save current epoch time
unsigned long epochTime; 

// Function that gets current epoch time
// todo: time failed mode?
const int max_gtime_tries = 3;
struct tm getTime() {
  struct tm timeinfo;
  int n_tries = 0;
  while (n_tries > max_gtime_tries-1 || !getLocalTime(&timeinfo)) {
    n_tries++;
    //Serial.println("Obtaining time try n " + String(n_tries));
    delay(500);
    //return timeinfo;
  }
  if(n_tries == max_gtime_tries) Serial.println("Failed to obtain time!");
  //time(&now);
  //Serial.println(&timeinfo, "It's %A, %B %d %Y %H:%M:%S");
  return timeinfo;
}

void getHourMin(int *hour, int* mins) {
  struct tm timeinfo;
  timeinfo = getTime();
  *hour = (int) timeinfo.tm_hour;
  *mins = (int) timeinfo.tm_min;
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

String getDateTime(bool format_for_sql){
  struct tm timeinfo;
  timeinfo = getTime();
   //GET DATE
  char dateTime[35];
  String date_format;
  if(format_for_sql){
    date_format = String("%Y-%m-%d %H:%M:%S");
  } else {
    date_format = String("%A, %B %d %Y %H:%M");
  }
  strftime(dateTime, sizeof(dateTime), date_format.c_str(), &timeinfo);
  return String(dateTime);
}

void setupTime() {
  //before  do: initWiFi();
  configTime(0, 0, ntpServer);
  // Berlin timeZone
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
  //Serial.println("time configed");
}
