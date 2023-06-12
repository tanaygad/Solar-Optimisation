// PINS USED
// 5V input from ESP
// bmp -> 22(SCL), 21(SDA)
// voltage -> 32
// current 1 -> 33
// current 2 -> 34 (unused)
// dht -> 13
// voltage_2 -> 34

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include "Adafruit_Sensor.h"
#include <Servo.h>
#include "WiFi.h"
#include "ThingSpeak.h"
#include <HTTPClient.h>
#include <DHT.h>
#include <DHT_U.h>
#include "DHT.h"

int master_count = 0, debug_count = 0;
Adafruit_BMP280 bmp; // use I2C interface
int cur1_pin = 33, cur2_pin = 71, dht_dpin = 13;
float cur1_avg = 0.0, cur2_avg = 0.0;
float sum_t = 0, sum_p = 0, sum_a = 0, mean_t = 0, mean_p = 0, mean_a = 0;
const int voltage_analog_pin= 32, voltage_2_analog_pin = 34;
int voltage_adc = 0, voltage_count = 0;
float voltage_avg = 0.0;
int thingspeakcount=0;
float humidity = 0.0, humidity_avg = 0.0;
int voltage_2_adc = 0, voltage_2_count = 0;
float voltage_2_avg = 0.0;
HTTPClient http;
const char* ntpServer = "pool.ntp.org";
unsigned long epochTime; 
long randNumber;
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}
#define Channel_ID 2163205
#define Channel_API_Key "3NFJ999UA29ABUYQ"
char* SSID="Galaxy M53 5GC4C1";
char* pass="kkqu3690";
WiFiClient client;
#define CSE_IP      "192.168.197.184"
#define CSE_PORT    5089
#define HTTPS     false
#define OM2M_ORGIN    "admin:admin"
#define OM2M_MN     "/~/in-cse/in-name/"
#define OM2M_AE     "SOLAR-OPTIMISATION"
#define OM2M_DATA_CONT  "Node-1/Data"
#define DHTTYPE DHT11

DHT dht(dht_dpin, DHTTYPE);

Servo servohori;
int servoh = 90;
int servohLimitHigh = 160;
int servohLimitLow = 20;

Servo servoverti; 
int servov = 90; 
int servovLimitHigh = 160;
int servovLimitLow = 20;
//Assigning LDRs
int ldrtopl = 34; //top left LDR green
int ldrtopr = 33; //top right LDR yellow
int ldrbotl = 35; // bottom left LDR blue
int ldrbotr = 32; // bottom right LDR orange
void setup() {
  // put your setup code here, to run once:
  // bmp.begin(0x76);
  // Serial.begin(9600);
  pinMode(cur1_pin,INPUT);
  // pinMode(cur2_pin,INPUT);
  // delay(500);
  dht.begin();
  configTime(0, 0, ntpServer);
  bmp.begin(0x76);
  Serial.begin(9600);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(SSID,pass);
  delay(2000);
  if(WiFi.status()==WL_CONNECTED){
    Serial.println("WiFi connected successfully!");
  }
  Serial.println("Local IP: ");
  Serial.print(WiFi.localIP());
  ThingSpeak.begin(client);
  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:
  master_count++;
  // debug_count++;
  thingspeakcount++;
  int occupancy=0;
  Serial.println("bhaisaab");
  if(master_count == 10)
  {
    mean_t = sum_t / 10.0;
    Serial.print("Temperature: ");
    Serial.println(mean_t);
    sum_t = 0.0;
    
    mean_p = sum_p / 10.0;
    Serial.print("Pressure: ");
    Serial.println(mean_p/100);
    sum_p = 0.0;

    mean_a = sum_a / 10.0;
    Serial.print("Altitude: ");
    Serial.println(mean_a);
    sum_a = 0.0;
    
    cur1_avg = cur1_avg / 10.0;
    Serial.print("Current 1: ");
    Serial.println(cur1_avg);
    cur1_avg = 0.0;

    cur2_avg = cur2_avg / 10.0;
    // Serial.print("Current 2: ");
    // Serial.println(cur2_avg);
    cur2_avg = 0.0;

    humidity_avg = humidity_avg / 10.0;
    Serial.print("Humidity: ");
    Serial.print(humidity_avg);
    Serial.println("%");
    humidity_avg = 0.0;

    voltage_avg /= 10;
    Serial.print("Voltage: ");
    Serial.println(voltage_avg);
    voltage_count = 0;
    voltage_avg = 0.0;

    voltage_2_avg /= 10;
    Serial.print("Voltage 2: ");
    Serial.println(voltage_2_avg);
    voltage_2_count = 0;
    voltage_2_avg = 0.0;

    master_count = 0;
    Serial.println("");
    Serial.println("");
    Serial.println("");
  }

  // if(debug_count == 10)
  // {
  //   // voltage_avg /= 10;
  //   // Serial.print("Voltage: ");
  //   // Serial.println(voltage_avg);
  //   // debug_count = 0;
  // }

  float temp = bmp.readTemperature();
  sum_t = sum_t + temp;

  float pressure = bmp.readPressure();
  sum_p = sum_p + pressure;

  float altitude = bmp.readAltitude();
  sum_a = sum_a + altitude;

  float humidity_reading = dht.readHumidity();
  humidity = humidity_reading;
  if(isnan(humidity_reading))
  {
    humidity = 0.0;
  }
  humidity_avg = humidity_avg + humidity;

  int cur1_adc=analogRead(cur1_pin);
  float cur1_current = (cur1_adc * 0.0002566) - 0.9261;             // Updated 9/6/2023
  cur1_avg += cur1_current;

  int cur2_adc=analogRead(cur2_pin);
  // Debugging
  // Serial.println(cur2_adc);
  float cur2_current = (cur2_adc * 0.003842) - 4.491;
  cur2_avg += cur2_current;

  voltage_adc = analogRead(voltage_analog_pin);
  Serial.println(voltage_adc);
  // Debugging
  // Serial.println(voltage_adc);
  float v=((voltage_adc * 0.004574) +  0.08769);
  if(v < 0.8)
  {
    v = 0.00;
  }
  voltage_avg += v;
  voltage_count++;

  float voltage_2_adc_reading = analogRead(voltage_2_analog_pin);
  Serial.println(voltage_2_adc_reading);
  if(!isnan(voltage_2_adc_reading))
  {
    voltage_2_adc = voltage_2_adc_reading;
  }
  // Debugging
  // Serial.println(voltage_adc);
  float voltage_2=((voltage_2_adc * 0.004574) +  0.08769);

  if(voltage_2 < 0.8)
  {
    voltage_2 = 0.00;
  }
  voltage_2_avg += voltage_2;
  voltage_2_count++;


  if(thingspeakcount==8){
  ThingSpeak.setField(1,temp);
  ThingSpeak.setField(2,pressure);
  ThingSpeak.setField(3,altitude);
  ThingSpeak.setField(4,cur1_current);
  ThingSpeak.setField(5,v);
  // ThingSpeak.setField(8,voltage_2);
  ThingSpeak.writeFields(Channel_ID,Channel_API_Key);
    thingspeakcount=0;
  }

  static int i=0;
  String data="[" + String(epochTime) + ", " + String(occupancy) + " , " + String(temp)+"]";

String server="http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String()+OM2M_MN;

// // Serial.println(data);
http.begin(server + String() +OM2M_AE + "/" + "Temp/Data" + "/");

http.addHeader("X-M2M-Origin", OM2M_ORGIN);
http.addHeader("Content-Type", "application/json;ty=4");
http.addHeader("Content-Length", "100");

String label = "Temp";

String req_data = String() + "{\"m2m:cin\": {"

  + "\"con\": \"" + data + "\","

  + "\"rn\": \"" + "cin_"+String(i) + "\","

  + "\"lbl\": \"" + label + "\","

  + "\"cnf\": \"text\""

  + "}}";
int code = http.POST(req_data);
http.end();
//-------------------------------------------------------------------------------------------
// Serial.println(code);
data="[" + String(epochTime) + ", " + String(occupancy) + " , " + String(humidity)+"]";

server="http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String()+OM2M_MN;

// // Serial.println(data);
http.begin(server + String() +OM2M_AE + "/" + "Humidity/Data" + "/");

http.addHeader("X-M2M-Origin", OM2M_ORGIN);
http.addHeader("Content-Type", "application/json;ty=4");
http.addHeader("Content-Length", "100");

label = "Humidity";
req_data = String() + "{\"m2m:cin\": {"

  + "\"con\": \"" + data + "\","

  + "\"rn\": \"" + "cin_"+String(i) + "\","

  + "\"lbl\": \"" + label + "\","

  + "\"cnf\": \"text\""

  + "}}";
code = http.POST(req_data);
http.end();
//-------------------------------------------------------------------------------
// Serial.println(code);
data="[" + String(epochTime) + ", " + String(occupancy) + " , " + String(pressure)+"]";

server="http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String()+OM2M_MN;

// // Serial.println(data);
http.begin(server + String() +OM2M_AE + "/" + "Pressure/Data" + "/");

http.addHeader("X-M2M-Origin", OM2M_ORGIN);
http.addHeader("Content-Type", "application/json;ty=4");
http.addHeader("Content-Length", "100");

label = "Pressure";
req_data = String() + "{\"m2m:cin\": {"

  + "\"con\": \"" + data + "\","

  + "\"rn\": \"" + "cin_"+String(i) + "\","

  + "\"lbl\": \"" + label + "\","

  + "\"cnf\": \"text\""

  + "}}";
code = http.POST(req_data);
http.end();
// Serial.println(code);
data="[" + String(epochTime) + ", " + String(occupancy) + " , " + String(altitude)+"]";

server="http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String()+OM2M_MN;

// Serial.println(data);
http.begin(server + String() +OM2M_AE + "/" + "Altitude/Data" + "/");

http.addHeader("X-M2M-Origin", OM2M_ORGIN);
http.addHeader("Content-Type", "application/json;ty=4");
http.addHeader("Content-Length", "100");

label = "Altitude";

req_data = String() + "{\"m2m:cin\": {"

  + "\"con\": \"" + data + "\","

  + "\"rn\": \"" + "cin_"+String(i) + "\","

  + "\"lbl\": \"" + label + "\","

  + "\"cnf\": \"text\""

  + "}}";
code = http.POST(req_data);
http.end();
Serial.println(code);
data="[" + String(epochTime) + ", " + String(occupancy) + " , " + String(cur1_current)+"]";

server="http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String()+OM2M_MN;

// Serial.println(data);
http.begin(server + String() +OM2M_AE + "/" + "Current-1/Data" + "/");

http.addHeader("X-M2M-Origin", OM2M_ORGIN);
http.addHeader("Content-Type", "application/json;ty=4");
http.addHeader("Content-Length", "100");

label = "Current-1";
req_data = String() + "{\"m2m:cin\": {"

  + "\"con\": \"" + data + "\","

  + "\"rn\": \"" + "cin_"+String(i) + "\","

  + "\"lbl\": \"" + label + "\","

  + "\"cnf\": \"text\""

  + "}}";
code = http.POST(req_data);
http.end();
//--------------------------------------------------------------------------------------

// Serial.println(code);
data="[" + String(epochTime) + ", " + String(occupancy) + " , " + String(v)+"]";

server="http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String()+OM2M_MN;

// Serial.println(data);
http.begin(server + String() +OM2M_AE + "/" + "Voltage-1/Data" + "/");

http.addHeader("X-M2M-Origin", OM2M_ORGIN);
http.addHeader("Content-Type", "application/json;ty=4");
http.addHeader("Content-Length", "100");
label = "Voltage-1";

req_data = String() + "{\"m2m:cin\": {"

  + "\"con\": \"" + data + "\","

  + "\"rn\": \"" + "cin_"+String(i) + "\","

  + "\"lbl\": \"" + label + "\","

  + "\"cnf\": \"text\""

  + "}}";
code = http.POST(req_data);

data="[" + String(epochTime) + ", " + String(occupancy) + " , " + String(voltage_2)+"]";

server="http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String()+OM2M_MN;
http.begin(server + String() +OM2M_AE + "/" + "Voltage-2/Data" + "/");

http.addHeader("X-M2M-Origin", OM2M_ORGIN);
http.addHeader("Content-Type", "application/json;ty=4");
http.addHeader("Content-Length", "100");
label = "Voltage-2";

req_data = String() + "{\"m2m:cin\": {"

  + "\"con\": \"" + data + "\","

  + "\"rn\": \"" + "cin_"+String(i++) + "\","

  + "\"lbl\": \"" + label + "\","

  + "\"cnf\": \"text\""

  + "}}";
code = http.POST(req_data);
http.end();
Serial.println(code);
  delay(50);
}
