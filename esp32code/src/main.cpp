#include <Arduino.h>
/*********
  Complete project details at https://randomnerdtutorials.com
  
  This is an example for our Monochrome OLEDs based on SSD1306 drivers. Pick one up today in the adafruit shop! ------> http://www.adafruit.com/category/63_98
  This example is for a 128x32 pixel display using I2C to communicate 3 pins are required to interface (two I2C and one reset).
  Adafruit invests time and resources providing this open source code, please support Adafruit and open-source hardware by purchasing products from Adafruit!
  Written by Limor Fried/Ladyada for Adafruit Industries, with contributions from the open source community. BSD license, check license.txt for more information All text above, and the splash screen below must be included in any redistribution. 
*********/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "evan";
const char* password = "12345678";

//Your Domain name with URL path or IP address with path
String serverName = "https://testing123-pa1y.onrender.com/events";

#define TIME_DELAY 100
unsigned long lastTime = 0;

#include <ArduinoJson.h>
JsonDocument doc;

#include <ESP32Time.h>
#include "time.h"
ESP32Time rtc;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -4*3600;
const int   daylightOffset_sec = 0;

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.println(timeinfo.tm_hour);
  rtc.setTimeStruct(timeinfo);
  Serial.println(rtc.getTimeDate());
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  //get time from server
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(1000); // Pause for 2 seconds

  
}

void loop() {
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > TIME_DELAY) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED) {
      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      http.begin(serverName.c_str());
      
      // If you need Node-RED/server authentication, insert user and password below
      //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
        deserializeJson(doc, payload);
        
        bool nothingPlanned = true;
        uint16_t currentTotal = rtc.getSecond() + rtc.getMinute()*60 + rtc.getHour(true)*3600;
        for (JsonObject item : doc.as<JsonArray>()) {
          String summary = item["summary"];
          String startTime = item["start_time"];
          String endTime = item["end_time"];

          int startYear, startMonth, startDay, startHour, startMinute, startSecond;
          int endYear, endMonth, endDay, endHour, endMinute, endSecond;
          // Parse the string
          sscanf(startTime.c_str(), "%d-%d-%d %d:%d:%d", &startYear, &startMonth, &startDay, &startHour, &startMinute, &startSecond);
          sscanf(endTime.c_str(), "%d-%d-%d %d:%d:%d", &endYear, &endMonth, &endDay, &endHour, &endMinute, &endSecond);

          uint16_t startTotal = startSecond + startMinute*60 + startHour*3600;
          uint16_t endTotal = endSecond + endMinute*60 + endHour*3600;

          if(currentTotal > startTotal && currentTotal < endTotal) {
            nothingPlanned = false;
            display.clearDisplay();
            display.setTextSize(1);             // Normal 1:1 pixel scale
            display.setTextColor(WHITE);        // Draw white text
            display.setCursor(0,0);             // Start at top-left corner
            display.println(summary);
            display.setCursor(0,10);            
            display.println(startTime);
            display.setCursor(0, 20);
            display.println(rtc.getTime());
            display.display();
            break;
          }         
        }
        if(nothingPlanned) {
          display.clearDisplay();
          display.setTextSize(1);             // Normal 1:1 pixel scale
          display.setTextColor(WHITE);        // Draw white text
          display.setCursor(0,0);             // Start at top-left corner
          display.println("Nothing Planned Right Now");
          display.setCursor(0, 20);
          display.println(rtc.getTime());
          display.display();
        }
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}