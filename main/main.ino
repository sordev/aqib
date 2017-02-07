#include <Wire.h>
#include <SPI.h>
// Only if you're using LCD 16x2
#include <LiquidCrystal.h>
// Creates an LC object. Parameters: (rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd(10,9,5,6,7,8);

// Include DHT LIbrary
// DHT Library from https://github.com/adafruit/DHT-sensor-library required https://github.com/adafruit/Adafruit_Sensor
#include <DHT.h>
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
#include <DHT_U.h>

//VARIABLES
byte nano[8] = {B00000,B00000,B00000,B10010,B10010,B10010,B11110,B10000};
byte pow3[8] = {B11000,B00100,B11000,B00100,B11000,B00000,B00000,B00000};

// #include <avr/wdt.h>

#define PM25 0
#define PM10 1
int pin[] = {4,3};
unsigned long starttime;
unsigned long sampletime_ms = 30000;
unsigned long triggerOn[2];
unsigned long triggerOff[2];
unsigned long lowpulseoccupancy[] = {0,0};
float ratio[] = {0,0};
float concentration[] = {0,0};
boolean value[] = {HIGH,HIGH};
boolean trigger[] = {false, false};

float humidity = 0;
float temp = 0;
// If you don't have DHT 11 or DHT 22 uncomment below
// int temp=20; //external temperature, if you can replace this with a DHT11 or better

int so2;
int co;
int nh3;
int no2;

void setup() {
  // Make weird characters
  lcd.createChar(0,nano);
  lcd.createChar(1,pow3);
  // Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display }
  lcd.begin(16, 2);
  // DHT Temp humidity sensor begin
  dht.begin();
  // Output to Serial at 9600 baud
  Serial.begin(9600);
  // Start Pin
  pinMode(pin[PM25],INPUT); //Listen at the designated PIN
  pinMode(pin[PM10],INPUT); //Listen at the designated PIN
  // wdt_enable(WDTO_8S);
}

void loop()
{
    lcd.clear();
    // Read mq136 sensor on Analogpin 0
    so2 = analogRead(0);
    // Read 6814 sensor on Analogpin 7,6,5
    co = analogRead(7);
    nh3 = analogRead(6);
    no2 = analogRead(5);

    lcd.print( "SO2: " + (String)so2 );
    lcd.setCursor(9, 0);
    lcd.print( "CO: " + (String)co );
    lcd.setCursor(0, 2);
    lcd.print( "NH3: " + (String)nh3 );
    lcd.setCursor(9, 2);
    lcd.print( "NO2: " + (String)no2 );
    delay(3000);

    lcd.clear(); // Clears the display
    humidity = dht.readHumidity();
    temp = dht.readTemperature();
    lcd.print( "Humi: " + (String)humidity + "%" );
    lcd.setCursor(0, 2);
    lcd.print( "Temp: " + (String)temp + "C");
    delay(3000);

    lcd.clear(); // Clears the display
    lcd.print( "Checking PM");
    lcd.setCursor(0, 2);
    lcd.print( "Wait for 30 sec");
    getPM();
    lcd.clear(); // Clears the display
    lcd.print( "PM10: " + (String)concentration[PM10] );
    lcd.setCursor(0, 2);
    lcd.print( "PM25: " + (String)concentration[PM25] );
    delay(5000);

}

void getPM(){
  // This part is from
  // https://www.shadowandy.net/2015/05/arduino-dust-sensor.htm
  starttime = millis(); //Fetching the current time
  while (1) {
    value[PM25] = digitalRead(pin[PM25]);
    value[PM10] = digitalRead(pin[PM10]);

    if(value[PM25] == LOW && trigger[PM25] == false) {
      trigger[PM25] = true;
      triggerOn[PM25] = micros();
    }
    if(value[PM25] == HIGH && trigger[PM25] == true) {
      triggerOff[PM25] = micros();
      lowpulseoccupancy[PM25] += (triggerOff[PM25] - triggerOn[PM25]);
      trigger[PM25] = false;
    }
    if(value[PM10] == LOW && trigger[PM10] == false) {
      trigger[PM10] = true;
      triggerOn[PM10] = micros();
    }
    if(value[PM10] == HIGH && trigger[PM10] == true) {
      triggerOff[PM10] = micros();
      lowpulseoccupancy[PM10] += (triggerOff[PM10] - triggerOn[PM10]);
      trigger[PM10] = false;
    }

    if ((millis()-starttime) > sampletime_ms)//Checking if it is time to sample
    {
      // main code from https://github.com/empierre/arduino/blob/master/DustSensor_Shinyei_PPD42NS.ino
      // ratio = (lowpulseoccupancy-endtime+starttime)/(sampletime_ms*10.0);  // Integer percentage 0=>100
      // long concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
      ratio[PM25] = lowpulseoccupancy[PM25]/(sampletime_ms*10.0);
      concentration[PM25] = 1.1*pow(ratio[PM25],3)-3.8*pow(ratio[PM25],2)+520*ratio[PM25]+0.62;
      ratio[PM10] = lowpulseoccupancy[PM10]/(sampletime_ms*10.0);
      concentration[PM10] = 1.1*pow(ratio[PM10],3)-3.8*pow(ratio[PM10],2)+520*ratio[PM10]+0.62;
      concentration[PM25] -= concentration[PM10];

      // Begin mass ppmv calculation
      temp = dht.readTemperature();
      float ppmv[] = {0,0};
      //ppmv=mg/m3 * (0.08205*Tmp)/Molecular_mass
      //0.08205   = Universal gas constant in atm·m3/(kmol·K)
      ppmv[PM10]=(concentration[PM10]*0.0283168/100/1000)*(0.08205*temp)/0.01;
      ppmv[PM25]=(concentration[PM25]*0.0283168/100/1000)*(0.08205*temp)/0.01;

      // Begin printing to Serial
      Serial.print("PM10 ppmv  : ");
      Serial.print(ppmv[PM10]);
      Serial.println(" ug/m3");
      Serial.print("PM10       : ");
      Serial.print(concentration[PM10]);
      Serial.println(" pt/cf");
      Serial.print("PM2.5 ppmv : ");
      Serial.print(ppmv[PM25]);
      Serial.println(" ug/m3");
      Serial.print("PM2.5      : ");
      Serial.print(concentration[PM25]);
      Serial.println(" pt/cf");
      Serial.println("");

      // Resetting for next sampling
      lowpulseoccupancy[PM25] = 0;
      lowpulseoccupancy[PM10] = 0;
      // starttime = millis();
      return;
    }
  }
}
