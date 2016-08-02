#include <SoftwareSerial.h>
#include <stdlib.h>
#define SSID "internet"    // SSID
#define USERNAME "zonginternet"
#define PASS ""  // Password
#define code "80"
String full_details="";
int battery;
int imp=51;
int minute=0;
int bat_percent;
int sec=0;
int power_pv;
int solar_charge;
int Battery_charging;
int voltagePin = A6;     //voltage
float vout = 0.0;        //voltage
float vin = 0.0;         //voltage
float R1 = 98000.0; //voltage    // resistance of R1 (156K) -see text!
float R2 = 9000.0; //Voltage     // resistance of R2 (22K)
int value = 0;      //voltage
int voltage;
int led=13;
int Battery_percent;
const int analogIn = A7;
int load=1;
int mos_pin=5;
// Set your scale factor
int mVperAmp = 20; // See Scale Factors Below
float curr;


// Set you Offset
int ACSoffset = 2500; // See offsets below

/* Offsets
If bi-directional = 2500
If uni- directional = 600
*/
int power;
int RawValue= 0;
float Voltage = 0;
float Amps = 0;
float current=0;

//GSM////
SoftwareSerial gsm(10,11);
char api_key1[30] = "WI7HGUGRR5ZKTMPX";
char api_key2[30] = "M7TMD93M8723KF42";
//  channel's thingspeak API key
char api_key[30] = "3K7HKUE6ETW4T0J0";
//

SoftwareSerial E_meter(8,7); // RX, TX
uint8_t start[] = { 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xEB, 0x90, 0xEB, 0x90, 0xEB, 0x90 }; uint8_t id = 0x16; uint8_t cmd[] = { 0xA0, 0x00, 0xB1, 0xA7, 0x7F };
uint8_t buff[128];
float to_float(uint8_t* buffer, int offset){ unsigned short full = buffer[offset+1] << 8 | buff[offset];
int solar_charge=0;
return full / 100.0; }

//Energy Meter


void setup(){ 
 Serial.begin(9600);
 gsm.begin(9600);
 pinMode(voltagePin,INPUT);
 pinMode(analogIn,INPUT);
 pinMode(mos_pin,OUTPUT);

 analogWrite(mos_pin,255);
 connectgsm();
 gsm.end();
}

void loop(){
  full_details="";
 sec++;
  Energy_meter();
 current_sensor();
 Voltage_sensor();
 power=abs(battery*curr);
Serial.print("load connected=");
Serial.print(abs(power));
Serial.println(" Watts");
Serial.print("Battery percentage=");
Serial.print(Battery_percent);
 Serial.println("%");
  Serial.println("-------------------------------------------------------------");
Serial.print("Minte=");
Serial.print(minute);
Serial.print("      Second=");
Serial.println(sec);

if(sec==12){
  minute++;

sec=0;
}


if(minute==4)
{

gsm.begin(9600);
gsm_reset();
 //connectgsm();
 
delay(2000);
delay(2000);  
  gsm.println("AT+CIFSR");
response();
delay(2000);
send_net(abs(curr),voltage,power, Battery_percent,power_pv,solar_charge,load,Battery_charging,api_key);
delay(4000);

gsm.println("AT+CIPCLOSE");
response();
delay(2000);


gsm.end();
minute=0;
sec=0;

}


 
 
 
digitalWrite(led,HIGH);
 delay(2500);
digitalWrite(led,LOW);
 delay(2500);
 digitalWrite(led,HIGH);
 delay(2500);
 digitalWrite(led,LOW);
 delay(2500); 
 
}
int current_sensor()
{

for(int i=0;i<=100;i++)
 {
 RawValue = analogRead(analogIn);
 Voltage = (RawValue / 1023.0) * 5000; // Gets you mV
 Amps += ((Voltage - ACSoffset) / mVperAmp);
 }
 current=Amps/100;
 Amps=0;

Serial.print("Current = "); // shows the voltage measured 
 curr=current+0.500;
 Serial.print(curr,3); // the '3' after voltage allows you to display 3 digits after decimal point
Serial.println(" amps");
if(curr>=-1.00)
 {
Serial.print("Battery Status=");
Serial.println("Charging Battery");
Battery_charging=1;
Serial.print("Wapda Status=");
Serial.println("Wapda is ON");
if(load==1){Serial.println("load connected");} else {Serial.println("load disconnected");}
 }
 else
 {
  Battery_charging=0;
Serial.print("Battery Status=");
Serial.println("Discharging Battery");
Serial.print("Wapda Status=");
Serial.println("Wapda is OFF");
if(load==0){Serial.println("load disconnected");} else {Serial.println("load connected");}
 }

}

int Voltage_sensor()
{
  for(int i = 0; i <100; i++)
  {
 value = analogRead(voltagePin);
 //Serial.println(value);
   vout = (value * 4.60) / 1024.00; // see text
   vin += vout / (R2/(R1+R2)); 

  }
 voltage=(vin/100);
 
 Serial.print("voltage=");
  Serial.print(battery);
  Serial.println(" Volts");
   Battery_percent= map(voltage, 22.5, 29.9, 10, 100);
 
vin=0.0;

}



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

battery = to_float(buff, 9); float pv = to_float(buff, 11); //13-14 reserved float 
float load_current = to_float(buff, 15); float over_discharge = to_float(buff, 17); float battery_max = to_float(buff, 19); // 21 load on/off // 22 overload yes/no // 23 load short yes/no // 24 reserved // 25 battery overload // 26 over discharge yes/no 
uint8_t full = buff[27]; uint8_t charging = buff[28]; int8_t battery_temp = buff[29] - 30;
float charge_current = to_float(buff, 30);

Serial.print("Load is "); Serial.println(buff[21] ? "on" : "off");

Serial.print("Load current: "); Serial.println(load_current);

Serial.print("Battery level: "); Serial.print(battery); Serial.print("/"); Serial.println(battery_max+1);
bat_percent=(battery/(battery_max+0.5))*100;
Serial.print("Battery Percentage: ");Serial.print(bat_percent);
Serial.print("Battery full: "); Serial.println(full ? "yes " : "no" );

Serial.print("Battery temperature: "); Serial.println(battery_temp);

Serial.print("PV voltage: "); Serial.println(pv);

Serial.print("Charging: "); Serial.println(charging ? "yes" : "no" );

Serial.print("Charge current: "); Serial.println(charge_current);

if(charging==1){solar_charge=1;}
if(charging==0){solar_charge=0;}
power_pv=pv*charge_current;

E_meter.end();
delay(200);
}

boolean connectgsm() {
  String cmd="";
  gsm.println("AT+CPIN?");
    response();
    delay(300);
    gsm.println("AT+CNMI=3,3,0,0");
response();
delay(300);
   gsm.println("AT+CGATT?");
    response();
    delay(1000);
gsm.println("AT+CMGF=1");
response();
delay(300);
      gsm.println("AT+CIPMUX=0");
response();
delay(300);
cmd = "AT+CSTT=\"";
  cmd += SSID;
  cmd += "\",\"";
  cmd += USERNAME;
  cmd += "\",\"";
  cmd += PASS;
  cmd += "\"";
  gsm.println(cmd);

 

   response();
 
     delay(1000);
   response();


   gsm.println("AT+CIICR");
   response();
   delay(1000);
   
  response();
  //Serial.print(cmd);
  delay(600);
  cmd="";
}
void response(){
while(gsm.available())
  {
    String msg="";
 
    while(gsm.available()){ 
char   b=gsm.read();
   
    msg+=b;
    }
    Serial.println(msg);
    delay(100);
    msg="";
  }}



  void send_net(int a_a,int b_b,int c_c,int d_d,int e_e,int f_f,int g_g,int h_h,char apiKey[])
{
 
 gsm.begin(9600);

   char buf[30];
  String one = dtostrf(a_a,2, 1, buf);
   delay(100);
  String two= dtostrf(b_b,2, 1, buf);
    delay(100);
   String three= dtostrf(c_c,2, 1, buf);
   delay(100);
  String four= dtostrf(d_d,2, 1, buf);
  delay(100);
  String five= dtostrf(e_e,2, 1, buf);
 delay(100);
String six= dtostrf(f_f,2, 1, buf);
 delay(100);
String seven= dtostrf(g_g,2, 1, buf);
 delay(100);
String eight= dtostrf(h_h,2, 1, buf);
  delay(100);
  
  // TCP connection

 String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
 cmd += "\",\"";
  cmd += code;
  cmd += "\"";
 gsm.println(cmd);
  response();
delay(500);
 delay(2000);

 cmd="";
 
  
  // prepare GET string
   String getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr +="&field1=";
  getStr +=String(one);
  delay(100);
  //getStr += "\r\n\r\n";
getStr +="&field2=";
  getStr +=String(two);
 delay(100);
  getStr +="&field3=";
  getStr +=String(three);
    delay(100);
    getStr +="&field4=";
  getStr +=String(four);
  delay(100);
  getStr +="&field5=";
  getStr +=String(five);
delay(100);
    getStr +="&field6=";
  getStr +=String(six);
delay(100);
//  getStr +="&field7=";
//    getStr +=String(seven);
//  delay(100);
    getStr +="&field8=";
    getStr +=String(eight);
  delay(100);
  getStr +="\r\n\r\n";
  
  // send data length
  
  gsm.println("AT+CIFSR");
response();
delay(2000);
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  
 gsm.println(cmd);
delay(50);
//response();
 if(gsm.find(">")){
    gsm.println(getStr);
//Serial.println(getStr);
  delay(2000);
   delay(2000);
    delay(2000);
  }
  else{
  gsm.println("AT+CIPCLOSE");
   
    // alert user
 Serial.println("AT+CIPCLOSE");
  gsm.println("AT+CMGS=\"+923135845995\"");
delay(100); 
 gsm.print(getStr);
    gsm.write((char)26);                       // End AT command with a ^Z, ASCII code 26
  delay(2000); 
  
 

gsm.println("AT+CMGD=1,4");
response();
delay(150);

  }
  

delay(2000);
response();
delay(1000);
response();
delay(2000);
response();

gsm.end();

}

 void gsm_reset()
 {

gsm.println("AT+IPR=9600");
   response();
delay(200);
gsm.println("AT+CIPSHUT");
    delay(4000);
    response();
     gsm.println("AT+CFUN=0");
    delay(4000);
      response();
    delay(4000);
      response();
    delay(4000);
      response();
    delay(4000);
      response();
    delay(4000);
      response();
    delay(4000);
      response();
    delay(4000);
      response();
    delay(4000);
      response();
    delay(4000);
      response();
    delay(4000);
    gsm.println("AT+CFUN=1");
    delay(2000);
      response();
    delay(2000);
      response();
    delay(2000);
      response();
    delay(2000);
      response();
    delay(2000);
      response();
    delay(2000);
      response();
    delay(2000);
      response();
    delay(2000);
    delay(2000);
      response();
    gsm.println("AT");
    response();
    delay(200);
    
    gsm.println("AT+IPR=9600");
    response();
    delay(200);
    gsm.println("AT+CMGD=1,4");
   delay(150);
connectgsm();

delay(3000);



 }
 void all_details()
{
delay(100);


 

 Serial.println("sending sms");
 gsm.println("AT+CMGS=\"+923135845995\"");
 delay(100); 
 gsm.print(full_details);
    gsm.write((char)26);                       // End AT command with a ^Z, ASCII code 26
  delay(2000); 
  
  Serial.println("done");
 

  gsm.println("AT+CMGD=1,4");
  response();
delay(150);
full_details="";

}
 
