#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHT10.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//const char* ssid = "SSID";
//const char* password = "password";
const char* ssid = "SSID";
const char* password = "password";

//Node-Red Network Location
const char* serverName = "https://NODE-RED.IP:1880/Greenhouse";
//const char* serverName = "https://NODE-RED.IP:1880/Greenhouse";

Adafruit_AHT10 aht;
Adafruit_BMP280 bmp; // I2C

//Probe Setup
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
uint8_t sensor1[8] = { 0x28, 0x56, 0xDF, 0x76, 0xE0, 0xFF, 0x3C, 0x49 };
uint8_t sensor2[8] = { 0x28, 0xFF, 0x6C, 0xB5, 0x58, 0x16, 0x04, 0x11 };


float Temp1;
float Pres;
float Alt;
float pressure;
String Temp2;
String Hum;

void setup() {
  Serial.begin(115200);

  //Wifi Setup
    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while(WiFi.status() != WL_CONNECTED) {
      delay(1500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println(F("BMP280 test"));

  //if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
  if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    while (1) delay(10);
    
  }
  if (! aht.begin()) {
    Serial.println("Could not find AHT10? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 found");
  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void loop() {
  if(WiFi.status()== WL_CONNECTED){

    //Http stuff before Posting
    HTTPClient http;
    http.begin(serverName);

    //Probe Check
    sensors.requestTemperatures();
    delay(500);
    float probeA = sensors.getTempC(sensor1);
    float probeB = sensors.getTempC(sensor2);
    

    //Serial Print data from AHT10
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
        
    //Put data in Variables
    Temp1 = bmp.readTemperature();
    pressure = bmp.readPressure();
    Alt = bmp.readAltitude(1013.25);
    Temp2 = String(temp.temperature);
    Hum = String(humidity.relative_humidity);

    //Convert to Sea level Pressure. Adjust to elevation of station
    //Pres = (((pressure)/pow((1-((float)(*Elevation*))/44330), 5.255))/100.0)
    Pres = (((pressure)/pow((1-((float)(18))/44330), 5.255))/100.0);
    
    //Prep Json Data
    String json;
    DynamicJsonDocument jdata(1024);

      jdata["api_key"] = "tPmAT5Ab3j7F9";
      jdata["sensor_name"]   = "Greenhouse";
      jdata["Temperature"] = String(Temp1);
      jdata["Pressure"] = String(Pres);
      jdata["Altitude"] = String(Alt);
      jdata["Temperature2"] = String(Temp2);
      jdata["humidity"] = String(Hum);
      jdata["OutsideProbe"] = String(probeA);
      jdata["AirProbe"] = String(probeB);


      serializeJson(jdata, json);
      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST(json);
      http.end();

      //Serial Print data for BMP280
      Serial.print(F("Temperature = "));
      Serial.print(bmp.readTemperature());
      Serial.println(" *C");

      Serial.print(F("Pressure = "));
      Serial.print(bmp.readPressure());
      Serial.println(" Pa");

      Serial.print(F("Approx altitude = "));
      Serial.print(bmp.readAltitude(1013.25)); /* Adjusted to local forecast! */
      Serial.println(" m");
      Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
      Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");

      Serial.print(probeA);
      Serial.print(" C | ");
      Serial.print(probeB);
      Serial.print(" C");
      Serial.print("");
      Serial.print("Going to Sleep");
      esp_sleep_enable_timer_wakeup(60 * 1000000);
      esp_deep_sleep_start();
   }
    else {
      Serial.println("WiFi Disconnected, Reconnecting in:");
      Serial.println("3");
      delay(1000);
      Serial.println("2");
      delay(1000);
      Serial.println("1");
      delay(1000);
      WiFi.begin();
    }
}
