// Nokia 5110 LCD Libraries
// https://github.com/baghayi/Nokia_5110
#include "Nokia_5110.h"

#define RST 13
#define CE 12
#define DC 14
#define DIN 15
#define CLK 2
Nokia_5110 lcd = Nokia_5110(RST, CE, DC, DIN, CLK);

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

int PM01Value = 0;        //define PM1.0 value of the air detector module
int PM2_5Value = 0;       //define PM2.5 value of the air detector module
int PM10Value = 0;       //define PM10 value of the air detector module


#include <ESP8266WiFi.h>
const char ssid[] = "devsor";
const char pass[] = "97070767";
int tries = 0;

unsigned long starttime;

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
  ESP.wdtDisable(); // Disable Watchdog
  ESP.wdtEnable(WDTO_8S); // Enabling Watchdog
  connectWiFi();
  starttime = millis(); //Fetching the current time

  //  lcd.setContrast(90);
  lcd.print("Please Wait ...");
  delay(1000);
  lcd.clear();

  lcd.print("Hi there");
  lcd.println(":D");

  lcd.setCursor(0, 5);
  lcd.println("1 2 3 ...");

  dht.begin();
  Serial.begin(9600);
  Serial.setTimeout(1500);    //set the Timeout to 1500ms, longer than the data transmission periodic time of the sensor
}

void loop(void)
{
  humidity = dht.readHumidity();
  temp = dht.readTemperature();
  Serial.println("Humi: " + (String)humidity + "%");
  Serial.println("Temp: " + (String)temp + "C");
  lcd.clear();
  lcd.println("Humi: " + (String)humidity + "%");
  lcd.println("Temp: " + (String)temp + "C");
  delay(3000);

  if (Serial.find(0x42)) {  //start to read when detect 0x42
    Serial.readBytes(buf, LENG);

    if (buf[0] == 0x4d) {
      if (checkValue(buf, LENG)) {
        PM01Value = transmitPM01(buf); //count PM1.0 value of the air detector module
        PM2_5Value = transmitPM2_5(buf); //count PM2.5 value of the air detector module
        PM10Value = transmitPM10(buf); //count PM10 value of the air detector module
      }
    }
  }

  static unsigned long OledTimer = millis();
  if (millis() - OledTimer >= 1000)
  {
    OledTimer = millis();

    Serial.print("PM1.0: ");
    Serial.print(PM01Value);
    Serial.println("  ug/m3");

    Serial.print("PM2.5: ");
    Serial.print(PM2_5Value);
    Serial.println("  ug/m3");

    Serial.print("PM1 0: ");
    Serial.print(PM10Value);
    Serial.println("  ug/m3");
    Serial.println();
  }
  lcd.clear();
  lcd.println("PM in ug/m3");
  lcd.println(" ");
  lcd.println("PM 1.0: " + (String)PM01Value);
  lcd.println("PM 2.5: " + (String)PM2_5Value);
  lcd.println("PM 10: " + (String)PM10Value);
  delay(3000);

  ESP.wdtFeed(); // Reset the WatchDog
}

void connectWiFi() {
  lcd.clear();
  lcd.println("Connecting WiFi:");
  lcd.println((String)ssid);
  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.println("Connected WiFi:");
    lcd.println((String)ssid);
    return;
  }
  tries++;
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.clear();
    lcd.println("Connection failed:");
    lcd.println("Trying:" + tries);
  }
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

int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val = ((thebuf[3] << 8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}

//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val = ((thebuf[5] << 8) + thebuf[6]); //count PM2.5 value of the air detector module
  return PM2_5Val;
}

//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val = ((thebuf[7] << 8) + thebuf[8]); //count PM10 value of the air detector module
  return PM10Val;
}
