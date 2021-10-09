#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <SPI.h>

const char* ssid = "SSID";
const char* password = "Password";

//DHT Config stuff
#define DHTPIN 27  // Digital pin connected to the DHT sensor
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value

// TFT module info
TFT_eSPI tft = TFT_eSPI(); 


//Node-Red Network Location
const char* serverName = "https://NodeRED.IP:1880/aptout";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 20000;

void setup() {
  dht.begin();    
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
  //TFT
  tft.begin();
  tft.init();
  tft.fillScreen(0xF81F);
  tft.setRotation(1); 
  

  //Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop() {
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      http.begin(serverName);
      hum = dht.readHumidity();
      temp= dht.readTemperature();
      DynamicJsonDocument jdata(1024);

      jdata["api_key"] = "tPmAT5Ab3j7F9";
      jdata["sensor_name"]   = "AptOutside";
      jdata["temperature"] = String(temp);
      jdata["humidity"] = String(hum);
      
      //serializeJson(jdata, Serial);
      String json;
      String Stemp = String("T: " + String(temp, 1));
      String Shum = String("H: " + String(hum, 1));
      serializeJson(jdata, json);
      //TFT
      tft.fillScreen(TFT_BLACK);
      tft.drawFastHLine(10, 10, tft.width() - 20, TFT_RED);
      tft.setCursor(20, 20);
      tft.setTextSize(5);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.println(Stemp);
      tft.drawFastHLine(10, 65, tft.width() - 20, TFT_RED);
      tft.setCursor(20, 75);
      tft.setTextColor(TFT_CYAN, TFT_BLACK);
      tft.println(Shum);
      tft.drawFastHLine(10, 125, tft.width() - 20, TFT_RED);

      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST(json);
  
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
      delay(2000);
      reboot();
    }
    lastTime = millis();
  }
}
