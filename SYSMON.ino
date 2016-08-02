////////////SYSMON version 1////////////////

#include <SoftwareSerial.h>
#include <stdlib.h>
#define SSID "Nayatel"    // SSID
#define PASS "owais123"  // Password
//#define PASS "[owais123]";
float wapda_status;
int mos_status;
int power_pv;
int wapda_charge;
int solar_charge;
int battery_percent;

char api_key1[30] = "WI7HGUGRR5ZKTMPX";
char api_key2[30] = "M7TMD93M8723KF42";
// replace with your channel's thingspeak API key
char api_key[30] = "0IPCKMA0138QSV3A";
//Energy Meter//
float load_current,battery_max,full,battery_temp,charging,charge_current;
// connect 10 to TX of Serial USB
// connect 11 to RX of serial USB


SoftwareSerial ser(8,9); // RX, TX
/// Enerygy meter///
SoftwareSerial E_meter(10,11); // RX, TX
uint8_t start[] = { 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xEB, 0x90, 0xEB, 0x90, 0xEB, 0x90 }; uint8_t id = 0x16; uint8_t cmd[] = { 0xA0, 0x00, 0xB1, 0xA7, 0x7F };
uint8_t buff[128];

//Energy Meter//








int v2; 
#define mos_Pin 4

int second=0; //TIMER
int minute=0; //TIMER
////////////CURRENT SECTION/////////////////
const int currentPin = A7;  // current
const unsigned long sampleTime = 100000UL;     // current                      // sample over 100ms, it is an exact number of cycles for both 50Hz and 60Hz mains
const unsigned long numSamples = 250UL;       // current                        // choose the number of samples to divide sampleTime exactly, but low enough for the ADC to keep up
const unsigned long sampleInterval = sampleTime/numSamples;  //current        // the sampling interval, must be longer than then ADC conversion time
const int adc_zero = 512;          //current                                           // relative digital zero of the arudino input from ACS712 (could make this a variable and auto-adjust it)
float rms=0.00;
 int i;

 //////////VOLTAGE SECTION/////////////
int voltagePin = A4;     //voltage
float vout = 0.0;        //voltage
float vin = 0.0;         //voltage
float R1 = 100000.0+56000.0; //voltage    // resistance of R1 (156K) -see text!
float R2 = 22000.0; //Voltage     // resistance of R2 (22K) - see text!
int value = 0;      //voltage
//////// POWER SECTION//////////////////
int power;


void setup()
{
//////////PINS INITIALIZE///////////
  pinMode(mos_Pin, OUTPUT);  //mosfet 
 
 pinMode(voltagePin, INPUT);

  
  //////// SERIAL PORTS DEFINE///////////
  Serial.begin(115200);
 ser.begin(115200);
 
mosfet();   
///////// CHECKING ESP STATUS///////////
 Serial.println("SYSMON INITIALIZING");
 ser.println("AT");
   delay(200);
  
 
  // reset ESP8266
  connectWiFi();
  response();
 delay(2000);
//////// WAIT TO CONNECT ESP////////////
}
//Energy Meter
float to_float(uint8_t* buffer, int offset){ unsigned short full = buffer[offset+1] << 8 | buff[offset];

return full / 100.0; }

//Energy Meter
void loop()
{
 mosfet();  /// MOSFET
 Energy_meter(); //Energy Meter
Voltage_sensor();  // VOLTAGE

Current_sensor(); //CURRENT
 
 power=vin*rms; // POWER
 
 Serial.print("power=");
 Serial.print(abs(power));
 Serial.println("W");
 ser.println("AT");
   delay(200);
response();
////////// TIMER START ////////////
 
second++;
Serial.print("Minute=");
Serial.print(minute);
Serial.print("         ");
Serial.print("second=");
Serial.println(second);
delay(1000);

if(second>=60)
{
minute++;
second=0;
Serial.print("Minute=");
Serial.println(minute);


if(minute>=1)
{
  Serial.println("Updating the website");
  send_net(rms,vin,power,api_key);
  delay(1000);
  send_net(rms,vin,power,api_key);
  delay(1000);
 send_net(wapda_status,power_pv,mos_status,api_key1);
 delay(1000);
 send_net(wapda_status,power_pv,mos_status,api_key1);
 delay(1000);
send_net(wapda_charge,solar_charge,battery_percent,api_key2);
 delay(1000);
send_net(wapda_charge,solar_charge,v2,api_key2);
 delay(1000);
  minute=0;
  second=0;
  ser.println("AT+RST");
  }
  else
  {
 ser.println("AT+RST");
 response();
  }
  }
}






/////////// TIMER END/////////////

void Energy_meter()
{
  E_meter.begin(9600);

Serial.println("Reading from Tracer");

E_meter.write(start, sizeof(start)); E_meter.write(id); E_meter.write(cmd, sizeof(cmd));

int read = 0;

for (int i = 0; i < 255; i++){ if (E_meter.available()) { buff[read] = E_meter.read(); read++; } }

Serial.print("Read "); Serial.print(read); Serial.println(" bytes");

for (int i = 0; i < read; i++){ Serial.print(buff[i], HEX); Serial.print(" "); }

Serial.println();

float battery = to_float(buff, 9); float pv = to_float(buff, 11); //13-14 reserved float 
float load_current = to_float(buff, 15); float over_discharge = to_float(buff, 17); float battery_max = to_float(buff, 19); // 21 load on/off // 22 overload yes/no // 23 load short yes/no // 24 reserved // 25 battery overload // 26 over discharge yes/no 
uint8_t full = buff[27]; uint8_t charging = buff[28]; int8_t battery_temp = buff[29] - 30;
float charge_current = to_float(buff, 30);

Serial.print("Load is "); Serial.println(buff[21] ? "on" : "off");

Serial.print("Load current: "); Serial.println(load_current);

Serial.print("Battery level: "); Serial.print(battery); Serial.print("/"); Serial.println(battery_max+1);
battery_percent=(battery/(battery_max+0.5))*100;
Serial.print("Battery Percentage: ");Serial.print((battery/(battery_max+0.5))*100);

Serial.print("Battery full: "); Serial.println(full ? "yes " : "no" );

Serial.print("Battery temperature: "); Serial.println(battery_temp);

Serial.print("PV voltage: "); Serial.println(pv);

Serial.print("Charging: "); Serial.println(charging ? "yes" : "no" );
if(charging==1)
{
  solar_charge=1;
}
if(charging==0)
{
solar_charge=0;
}

Serial.print("Charge current: "); Serial.println(charge_current);
power_pv=pv*charge_current;
Serial.print("PV Power Produced:");Serial.println(power_pv);
float t_current=rms+charge_current;

E_meter.end();
delay(200);

}

void send_net(float a_a,float b_b,float c_c,char apiKey[])
{
 
  ser.begin(115200);
   
   char buf[30];
  String current = dtostrf(a_a, 4, 1, buf);
  String voltage= dtostrf(b_b, 4, 1, buf);
   String powerr= dtostrf(c_c, 4, 1, buf);

  //Serial.println(current);
  
  // TCP connection

 String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
  cmd += "\",80";
  ser.println(cmd);
  delay(400);
   response();
 
   cmd="";
  if(ser.find("Error")){
    Serial.println("AT+CIPSTART error");
    ser.println("AT+RST");
    return;
  }
  
  // prepare GET string
   String getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr +="&field1=";
  getStr += String(current);
  //getStr += "\r\n\r\n";
getStr +="&field2=";
  getStr += String(voltage);
  getStr +="&field3=";
  getStr += String(powerr);
  getStr += "\r\n\r\n";
  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ser.println(cmd);

  if(ser.find(">")){
    ser.print(getStr);
    Serial.print(getStr);
  }
  else{
    ser.println("AT+CIPCLOSE");
    ser.println("AT+RST");
    // alert user
    Serial.println("AT+CIPCLOSE");
  }
  ser.end();
  delay(5000);


}




int Voltage_sensor()
{
  for(int i = 0; i < 60; i++)
  {
 value = analogRead(voltagePin);
 //Serial.println(value);
   vout = (value * 4.90) / 1024.00; // see text
   vin += vout / (R2/(R1+R2)); 

  }
  vin=(vin/60);
   Serial.print("voltage=");
 Serial.print(vin,1);
 
 Serial.println("V");


 v2= map(vin, 22.5, 29.2, 15, 100);
  Serial.print("percentage of battery=");
Serial.print(v2);
 Serial.println("%");
 
if(vin>=29)
{

Serial.println("fully charged");
}
else if (vin<=20)
{
Serial.println("fully discharge");
}
}
//if(vin>=BAT_max)
//{
//Serial.println("Fully Charged");
//}






int Current_sensor()
{
  int adc_raw=0;
   unsigned long currentAcc = 0;
 unsigned int count = 0;
 unsigned long prevMicros = micros() - sampleInterval ;

if(analogRead(currentPin)>=520)
{
Serial.println("battery in discharging condition");
Serial.println("WAPDA OFF");
wapda_charge=0;
wapda_status=0;
for (int i=0;i<=150;i++)
{  
     adc_raw +=(analogRead(currentPin))-adc_zero ;
     delay(2);
}    
   adc_raw=adc_raw/150;
 
rms = abs((adc_raw) * (75.7576 / 1024.0));

Serial.print("Current=");
Serial.print(abs(rms));
Serial.println("Amps");
delay(100);
}

if(analogRead(currentPin)<520)
{

//Serial.println(analogRead(currentPin));
Serial.println("battery in charging condition");
wapda_status=1;
wapda_charge=1;
Serial.println("WAPDA ON");
while (count < numSamples)
 {
   if (micros() - prevMicros >= sampleInterval)
   {
     adc_raw =analogRead(currentPin)-adc_zero ;
     currentAcc += (unsigned long)(adc_raw * adc_raw);
     ++count;
     prevMicros += sampleInterval;
     delay(2);
   }
 }
 
rms = abs(sqrt((float)currentAcc/(float)numSamples) * (75.7576 / 1024.0));

Serial.print("Current=");
Serial.print(abs(rms));
Serial.println("Amps");
delay(100);




}

}




//
int mosfet()

{
  digitalWrite(mos_Pin,HIGH);

   mos_status=digitalRead(mos_Pin);
  Serial.println(mos_status);

  
  }
//Serial.println("ON STATE");
//if(vin>=29||vin<=20)
//{
//  Serial.println("OFF STATE");
//  delay(100);
//digitalWrite(mos_Pin,0);
//delay(100);
//
//}
//else
//{
//
//digitalWrite(mos_Pin,HIGH);
// Serial.println("ON STATE");
//}





boolean connectWiFi() {
  String cmd="";
  ser.println("AT+CWMODE=1");
    delay(200);
  response();
 
  //

  
  cmd = "AT+CWJAP=\"";
  cmd += SSID;
  cmd += "\",\"";
  cmd += PASS;
  cmd += "\"";
  ser.print(cmd);
  delay(600);
   response();
   ser.println("AT+CIFSR=?");
  response();
  Serial.print(cmd);
  delay(500);
  cmd="";
}
void response(){
while(ser.available())
  {
    String msg="";
   //Serial.println("data available");
    while(ser.available()){ 
char   b=ser.read();
   
    msg+=b;
    }
    Serial.println(msg);
    delay(100);
    msg="";
  }}

