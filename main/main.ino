#include <SoftwareSerial.h>
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
// If you don't have DHT 11 or DHT 22 uncomment below
// int temp=20; //external temperature, if you can replace this with a DHT11 or better
long ppmv;

int so2;
int co;
int nh3;
int no2;

// WIFI credentials rx tx
SoftwareSerial ESP8266(12, 11);
String SSID = "devsor";
String PASSWORD = "97070767";
boolean FAIL_8266 = false;

void setup() {
  // Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display }
  lcd.begin(16, 2);
  dht.begin();
  // Start Pin
  pinMode(DUST_SENSOR_DIGITAL_PIN_PM10,INPUT);
  pinMode(DUST_SENSOR_DIGITAL_PIN_PM25,INPUT);
  // Serial.begin(9600);
  // ESP8266.begin(9600);
  do{
    Serial.begin(9600);
    ESP8266.begin(9600);

    //Wait Serial Monitor to start
    while(!Serial);
    Serial.println("--- Start ---");

    ESP8266.println("AT+RST");
    delay(1000);
    if(ESP8266.find("ready"))
    {
      Serial.println("Module is ready");
      ESP8266.println("AT+CWMODE=1");
      delay(2000);

      //Quit existing AP, for demo
      Serial.println("Quit AP");
      ESP8266.println("AT+CWQAP");
      delay(1000);

      clearESP8266SerialBuffer();
      if(cwJoinAP())
      {
        Serial.println("CWJAP Success");
        FAIL_8266 = false;

        delay(3000);
        clearESP8266SerialBuffer();
        //Get and display my IP
        sendESP8266Cmdln("AT+CIFSR", 1000);
        //Set multi connections
        sendESP8266Cmdln("AT+CIPMUX=1", 1000);
        //Setup web server on port 80
        sendESP8266Cmdln("AT+CIPSERVER=1,80",1000);

        Serial.println("Server setup finish");
      }else{
        Serial.println("CWJAP Fail");
        delay(500);
        FAIL_8266 = true;
      }
    }else{
      Serial.println("Module have no response.");
      delay(500);
      FAIL_8266 = true;
    }
  }while(FAIL_8266);
}

void loop()
{
  // Read mq136 sensor on Analogpin 0
  // Read 6814 sensor on Analogpin 7,6,5
  lcd.clear();
  so2 = analogRead(0);
  co = analogRead(7);
  nh3 = analogRead(6);
  no2 = analogRead(5);

  lcd.print( "SO2: " + (String)so2 );
  lcd.setCursor(0, 2);
  lcd.print( "CO: " + (String)co );
  delay(4000);
  lcd.clear();
  lcd.print( "NH3: " + (String)nh3 );
  lcd.setCursor(0, 2);
  lcd.print( "NO2: " + (String)no2 );
  delay(4000);

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

  if(ESP8266.available())
    {
      Serial.println("Something received");
      delay(1000);
      if(ESP8266.find("+IPD,"))
      {
        String action;

        Serial.println("+IPD, found");
        int connectionId = ESP8266.read()-48;
        Serial.println("connectionId: " + String(connectionId));

        ESP8266.find("led=");
        char s = ESP8266.read();
        if(s=='0'){
          action = "led=0";
          // digitalWrite(LED, LOW);
        }else if(s=='1'){
          action = "led=1";
          // digitalWrite(LED, HIGH);
        }else{
          action = "led=?";
        }

        Serial.println(action);
        sendHTTPResponse(connectionId, action);

        //Close TCP/UDP
        String cmdCIPCLOSE = "AT+CIPCLOSE=";
        cmdCIPCLOSE += connectionId;
        sendESP8266Cmdln(cmdCIPCLOSE, 1000);
      }
    }

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

void sendHTTPResponse(int id, String content)
{
  String response;
  response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: text/html; charset=UTF-8\r\n";
  response += "Content-Length: ";
  response += content.length();
  response += "\r\n";
  response +="Connection: close\r\n\r\n";
  response += content;

  String cmd = "AT+CIPSEND=";
  cmd += id;
  cmd += ",";
  cmd += response.length();

  Serial.println("--- AT+CIPSEND ---");
  sendESP8266Cmdln(cmd, 1000);

  Serial.println("--- data ---");
  sendESP8266Data(response, 1000);
}

boolean waitOKfromESP8266(int timeout)
{
  do{
    Serial.println("wait OK...");
    delay(1000);
    if(ESP8266.find("OK"))
    {
      return true;
    }

  }while((timeout--)>0);
  return false;
}

boolean cwJoinAP()
{
  String cmd="AT+CWJAP=\"" + SSID + "\",\"" + PASSWORD + "\"";
  ESP8266.println(cmd);
  return waitOKfromESP8266(10);
}

//Send command to ESP8266, assume OK, no error check
//wait some time and display respond
void sendESP8266Cmdln(String cmd, int waitTime)
{
  ESP8266.println(cmd);
  delay(waitTime);
  clearESP8266SerialBuffer();
}

//Basically same as sendESP8266Cmdln()
//But call ESP8266.print() instead of call ESP8266.println()
void sendESP8266Data(String data, int waitTime)
{
  ESP8266.print(data);
  delay(waitTime);
  clearESP8266SerialBuffer();
}

//Clear and display Serial Buffer for ESP8266
void clearESP8266SerialBuffer()
{
  Serial.println("= clearESP8266SerialBuffer() =");
  while (ESP8266.available() > 0) {
    char a = ESP8266.read();
    Serial.write(a);
  }
  Serial.println("==============================");
}
