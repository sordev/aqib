#include <Wire.h>
#include <SPI.h>
// Only if you're using LCD 16x2
#include <LiquidCrystal.h>
// Creates an LC object. Parameters: (rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd(10,9,5,6,7,8);

// Include DHT LIbrary
// DHT 11 Library from https://github.com/adafruit/DHT-sensor-library required https://github.com/adafruit/Adafruit_Sensor
#include <DHT.h>
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
#include <DHT_U.h>

// PPD42NS data Pin 3, 4
#define DUST_SENSOR_DIGITAL_PIN_PM10  3
#define DUST_SENSOR_DIGITAL_PIN_PM25  4

unsigned long SLEEP_TIME = 30*1000; // Sleep time between reads (in milliseconds)
//VARIABLES
int val = 0;           // variable to store the value coming from the sensor
float valDUSTPM25 =0.0;
float lastDUSTPM25 =0.0;
float valDUSTPM10 =0.0;
float lastDUSTPM10 =0.0;
unsigned long duration;
unsigned long starttime;
unsigned long endtime;
unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
long concentrationPM25 = 0;
long concentrationPM10 = 0;
// If you don't have DHT 11 uncomment below
// int temp=20; //external temperature, if you can replace this with a DHT11 or better
long ppmv;

int mq136;

void setup() {
  // Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display }
  lcd.begin(16, 2);
  dht.begin();
  // Start Pin
  pinMode(DUST_SENSOR_DIGITAL_PIN_PM10,INPUT);
  pinMode(DUST_SENSOR_DIGITAL_PIN_PM25,INPUT);
  Serial.begin(9600);
}

void loop()
{
  lcd.clear(); // Clears the display
  float humidity = dht.readHumidity();
  float temp = dht.readTemperature();
  lcd.print( "Humi: " + (String)humidity + "%" );
  lcd.setCursor(0, 2);
  lcd.print( "Temp: " + (String)temp + "C");

  //get PM 2.5 density of particles over 2.5 μm.
  concentrationPM25=getPM(DUST_SENSOR_DIGITAL_PIN_PM25);
  // Clears the display
  lcd.clear();
  //ppmv=mg/m3 * (0.08205*Tmp)/Molecular_mass
  //0.08205   = Universal gas constant in atm·m3/(kmol·K)
  ppmv=(concentrationPM25*0.0283168/100/1000) *  (0.08205*temp)/0.01;
  if ((ceil(concentrationPM25) != lastDUSTPM25)&&((long)concentrationPM25>0)) {
    lastDUSTPM25 = ceil(concentrationPM25);
  }
  lcd.print( "PM25: " + (String)concentrationPM25 );
  lcd.setCursor(0, 2);
  lcd.print( "PPMV: " + (String)ppmv );

  //get PM 1.0 - density of particles over 1 μm.
  concentrationPM10=getPM(DUST_SENSOR_DIGITAL_PIN_PM10);
  // Clears the display
  lcd.clear();
  //ppmv=mg/m3 * (0.08205*Tmp)/Molecular_mass
  //0.08205   = Universal gas constant in atm·m3/(kmol·K)
  ppmv=(concentrationPM10*0.0283168/100/1000) *  (0.08205*temp)/0.01;
  if ((ceil(concentrationPM10) != lastDUSTPM10)&&((long)concentrationPM10>0)) {
    // gw.send(dustMsgPM10.set((long)ppmv));
    lastDUSTPM10 = ceil(concentrationPM10);
  }
  lcd.print( "PM10: " + (String)concentrationPM10 );
  lcd.setCursor(0, 2);
  lcd.print( "PPMV: " + (String)ppmv );
  delay(4000);

  // Read mq136 sensor on Analogpin 0
  lcd.clear();
  mq136 = analogRead(0);
  lcd.print( "MQ136: " + (String)mq136 );
  delay(4000);
}


long getPM(int DUST_SENSOR_DIGITAL_PIN) {
  starttime = millis();
  while (1) {
    duration = pulseIn(DUST_SENSOR_DIGITAL_PIN, LOW);
    lowpulseoccupancy += duration;
    endtime = millis();
    if ((endtime-starttime) > sampletime_ms)
    {
      ratio = (lowpulseoccupancy-endtime+starttime)/(sampletime_ms*10.0);  // Integer percentage 0=>100
      long concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve

      lowpulseoccupancy = 0;
      return(concentration);
    }
  }
}
