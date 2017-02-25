// Nokia 5110 LCD Libraries
// https://github.com/baghayi/Nokia_5110
#include "Nokia_5110.h"

#define RST 13
#define CE 12
#define DC 14
#define DIN 15
#define CLK 2
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
#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

int PM1Value = 0;        //define PM1.0 value of the air detector module
int PM25Value = 0;       //define PM2.5 value of the air detector module
int PM10Value = 0;       //define PM10 value of the air detector module

#include <ESP8266WiFi.h>
const char ssid[] = "devsor";
const char pass[] = "97070767";
const char api_url[] = "api.utaa.mn";
const char api_key[] = "3aa18dbe7df6844974e199e327c5c30e";
const char unwiredlabs_key[] = "918092cf2502c3";
int tries = 0;

// Include DHT LIbrary
// DHT Library from https://github.com/adafruit/DHT-sensor-library required https://github.com/adafruit/Adafruit_Sensor
#include <DHT.h>
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
#include <DHT_U.h>
float humidity = 0;
float temp = 0;

void setup(void)
{
  Serial.begin('9600');
  Serial.setTimeout(1500);    //set the Timeout to 1500ms, longer than the data transmission periodic time of the sensor
  setupSensor();
  ESP.wdtDisable(); // Disable Watchdog
  ESP.wdtEnable(WDTO_8S); // Enabling Watchdog
}

void loop(void)
{
  humidity = dht.readHumidity();
  temp = dht.readTemperature();
  lcd.clear();
  lcd.println("Humi: " + (String)humidity + "%");
  lcd.println("Temp: " + (String)temp + "C");
  delay(3000);

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
  delay(3000);

  ESP.wdtFeed(); // Reset the WatchDog
}

boolean connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.println("Connected WiFi:");
    lcd.println((String)ssid);
    delay(3000);
    return true;
  } else {
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED || tries  < 4) {
      tries++;
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
  // updateThingSpeak("1=" + String(concentration[PM10], DEC) + "&2=" + String(count[PM10], DEC) + "&3=" + String(concentration[PM25], DEC) + "&4=" + String(count[PM25], DEC));
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
  if (!client.connect(api_url, 80)) {
    return;
  }
  client.print(F("POST /api/update?key="));
  client.print(api_key);
  client.print(F("&"));
  client.print(data);
  client.print(F(" HTTP/1.1\r\nHost: \r\n\r\n"));
  client.println();
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
