#include <SPI.h>

// Nokia 5110 LCD Libraries
// https://github.com/baghayi/Nokia_5110
#include "Nokia_5110.h"
#define RST 10 //s3 gpio10
#define CE 5 //d1 gpio5
#define DC 4 //d2 gpio4
#define DIN 0 //d3 gpio0
#define CLK 2 //d3 gpio2
Nokia_5110 lcd = Nokia_5110(RST, CE, DC, DIN, CLK);

// https://www.dfrobot.com/wiki/index.php/PM2.5_laser_dust_sensor_SKU:SEN0177
//******************************
//*Abstract: Read value of PM1,PM2.5 and PM10 of air quality
//
//*Version：V3.1
//*Author：Zuyang @ HUST
//*Modified by Cain for Arduino Hardware Serial port compatibility
//*Date：March.25.2016
//******************************
#include <Arduino.h>
#define LENG 31 //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

int PM1Value = 0; //define PM1.0 value of the air detector module
int PM25Value = 0; //define PM2.5 value of the air detector module
int PM10Value = 0; //define PM10 value of the air detector module

#include <ESP8266WiFi.h>
const char ssid[] = "devsor";
const char pass[] = "97070767";
const char api_url[] = "api.utaa.mn";
const char api_key[] = "3aa18dbe7df6844974e199e327c5c30e";
const char unwiredlabs_key[] = "918092cf2502c3";
const int interval = 60000; // 60000 = 1min
unsigned long startTime;
int tries = 0;
byte mac[6];

// Include DHT LIbrary
// DHT Library from https://github.com/adafruit/DHT-sensor-library required https://github.com/adafruit/Adafruit_Sensor
#include <DHT.h>
#define DHTPIN 14 //d5 gpio14
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
#include <DHT_U.h>
float humidity = 0;
float temp = 0;

// Include mcp3008 library ADC
// https://github.com/nodesign/MCP3008
#include <MCP3008.h>
#define CLOCK_PIN 16 //d0 gpio16
#define MISO_PIN 12 //d6 (mcp3008-Dout) gpio12
#define MOSI_PIN 13 //d7 (mcp3008-Din) gpio13
#define CS_PIN 15 //d8 (mcp3008-CS) gpio15
MCP3008 adc(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN); //Setup mcp3008

// Analog reads
int co;
int nh3;
int no2;
int so2;
int o3; 

void setup(void)
{
  Serial.begin(9600);
  Serial.setTimeout(1500);    //set the Timeout to 1500ms, longer than the data transmission periodic time of the sensor
  setupSensor();
  ESP.wdtDisable(); // Disable Watchdog
  ESP.wdtEnable(WDTO_8S); // Enabling Watchdog
}

void loop(void)
{
  co = adc.readADC(0);
  nh3 = adc.readADC(1);
  no2 = adc.readADC(2);
  so2 = adc.readADC(3);
  lcd.clear();
  lcd.println("CO: " + (String)co);
  lcd.println("NH3: " + (String)nh3);
  lcd.println("NO2: " + (String)no2);
  lcd.println("SO2: " + (String)so2);
  delay(2000);

  humidity = dht.readHumidity();
  temp = dht.readTemperature();
  lcd.clear();
  lcd.println("Humi: " + (String)humidity + "%");
  lcd.println("Temp: " + (String)temp + "C");
  delay(2000);

  if (Serial.find(0x42)) {  //start to read when detect 0x42
    Serial.readBytes(buf, LENG);
    if (buf[0] == 0x4d) {
      if (checkValue(buf, LENG)) {
        PM1Value = transmitPM1(buf); //count PM1.0 value of the air detector module
        PM25Value = transmitPM25(buf); //count PM2.5 value of the air detector module
        PM10Value = transmitPM10(buf); //count PM10 value of the air detector module
      }
    }
  }

  lcd.clear();
  lcd.println("PM in ug/m3");
  lcd.println(" ");
  lcd.println("PM 1.0: " + (String)PM1Value);
  lcd.println("PM 2.5: " + (String)PM25Value);
  lcd.println("PM 10: " + (String)PM10Value);
  delay(2000);

  if (WiFi.status() == WL_CONNECTED) {
    //only send data after interval passed
    if (millis()-startTime > interval) {
      updateApi(
      "pm1=" + (String)PM1Value + 
      "&pm25=" + (String)PM25Value + 
      "&pm10=" + (String)PM10Value +       
      "&co=" + (String)co + 
      "&nh3=" + (String)nh3 + 
      "&no2=" + (String)no2 + 
      "&so2=" + (String)so2 + 
      "&humidity=" + (String)humidity + 
      "&temperature=" + (String)temp
      );
      // Reset Timer
      startTime = millis();
      delay(3000);
    }
  }

  ESP.wdtFeed(); // Reset the WatchDog
}

boolean connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.println("Connected WiFi:");
    lcd.println((String)ssid);
    startTime = millis(); // Start counting only when WiFi connected;
    delay(3000);
    return true;
  } else {
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED || tries  < 4) {
      tries++;
      lcd.clear();
      lcd.println("Connecting:");
      lcd.println((String)ssid);
      lcd.println("Tries:" + String(tries));
      delay(1000);
    }
    lcd.println("Can't connect:");
    lcd.println((String)ssid);
    delay(2000);
    return false;
  }
}

void setupSensor() {
  lcd.print("Booting Up ...");
  delay(3000);
  connectWiFi();
  if (WiFi.status() == WL_CONNECTED) {
    updateApi("mac=asdasdas&lat=47.921067&lon=106.977267&ip=192.168.1.10");
  }
}

void getIP() {
  // https://api.ipify.org?format=json
}

void getLocation() {
  // https://unwiredlabs.com/api#documentation
  // https://ap1.unwiredlabs.com/v2/process.php
}

void updateApi(String data) {
  WiFiClient client;
  if (client.connect(api_url, 80)) {
    lcd.clear();
    lcd.println("Sending Data");
    data = "api_key=" + (String)api_key + "&" + data;
    client.println("POST /api/update HTTP/1.1");
    client.println("Host: api.utaa.mn");
    //    client.print(api_url);
    client.println("Cache-Control: no-cache");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.println(data);
  } else {
    lcd.clear();
    lcd.println("Can't Send Data");
    client.stop();
  }
  return;
}

char checkValue(unsigned char *thebuf, char leng)
{
  char receiveflag = 0;
  int receiveSum = 0;
  for (int i = 0; i < (leng - 2); i++) {
    receiveSum = receiveSum + thebuf[i];
  }
  receiveSum = receiveSum + 0x42;
  if (receiveSum == ((thebuf[leng - 2] << 8) + thebuf[leng - 1])) //check the serial data
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

int transmitPM1(unsigned char *thebuf)
{
  int PM1Val;
  PM1Val = ((thebuf[3] << 8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM1Val;
}

//transmit PM Value to PC
int transmitPM25(unsigned char *thebuf)
{
  int PM25Val;
  PM25Val = ((thebuf[5] << 8) + thebuf[6]); //count PM2.5 value of the air detector module
  return PM25Val;
}

//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val = ((thebuf[7] << 8) + thebuf[8]); //count PM10 value of the air detector module
  return PM10Val;
}
