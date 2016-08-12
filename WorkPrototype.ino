#include<pt.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#define button 2
#define buzz 10
#define tem A5
#define sou A4
#include "DHT.h"
#define DHTIN 2
#define DHTOUT 3
#define DHTTYPE DHT11
#define light 12
#define PT_DELAY(pt, ms, ts) \
  ts = millis(); \
  PT_WAIT_WHILE(pt, millis()-ts < (ms));
int timemin=0;
bool push=0;
bool i=0;
int x,y,z;
int x2,y2,z2;
int xd=0,yd=0,zd=0;
float hum;
int pick=0;
int count=0;
bool once=0;
String up;
DHT dht(DHTIN,DHTOUT, DHTTYPE);
Adafruit_BMP085 bmp;
struct pt pt_taskbutton;
struct pt pt_taskgyro;
struct pt pt_tasktem;
struct pt pt_taskpr;
struct pt pt_tasksou;
struct pt pt_taskbuzz;
struct pt pt_taskhu;
void setup() 
{
  Serial.println("Prototype TEST");
  pinMode(button, INPUT);
  pinMode(tem,INPUT);
  pinMode(sou,INPUT);
  pinMode(buzz,OUTPUT);
  pinMode(light,OUTPUT);
  PT_INIT(&pt_taskgyro);
  PT_INIT(&pt_taskbutton);
  PT_INIT(&pt_tasktem);
  PT_INIT(&pt_taskpr);
  PT_INIT(&pt_tasksou);
  PT_INIT(&pt_taskbuzz);
  Serial.begin(9600);
  Serial1.begin(115200);
  dht.begin();
  //if (!bmp.begin()) {
  //Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  //while (1) {}
  //}

}

PT_THREAD(taskgyro(struct pt* pt))
{
  if(push==1)
  {
    static uint32_t ts;
  PT_BEGIN(pt);
  if(once ==0)
  {
    xd=x;
    yd=y;
    zd=z;
    once=1;
  }
  if(i==0)
  {
    x = analogRead(A0);  // อ่านค่าจากขาอะนาล็อก A0
    y = analogRead(A1);  // อ่านค่าจากขาอะนาล็อก A1
    z = analogRead(A2); // อ่านค่าจากขาอะนาล็อก A2
    i=1;
  }
  else
  {
    x2 = analogRead(A0);  // อ่านค่าจากขาอะนาล็อก A0
    y2 = analogRead(A1);  // อ่านค่าจากขาอะนาล็อก A1
    z2 = analogRead(A2); // อ่านค่าจากขาอะนาล็อก A2
    i=0;
  }
  if(abs(x-xd)>25 || abs(y-yd)>25 || abs(z-zd)>25)
  {
    pick++;
    Serial1.print("5,");
    Serial1.print(pick);
    xd=x;
    yd=y;
    zd=z;
    count=0;
  }
  if(count>=4)
  {
    xd=x;
    yd=y;
    zd=z;
    count=0;
  }
  else if(abs(x-x2)<10 || abs(y-y2)<10 || abs(z-z2)<10)
  {
    count++;
  }
   Serial.print(x);
   Serial.print(" ");
   Serial.print(y);
   Serial.print(" ");
   Serial.print(z);
   Serial.print(" ");
  Serial.println(pick);
  delay(1000); // หน่วงเวลา 100 มิลลิวินาท  
  PT_END(pt);
  }
}
PT_THREAD(taskbutton(struct pt* pt))
{
    static uint32_t ts;
  PT_BEGIN(pt);
  
  if(digitalRead(button)==0)
  { push=1;
    Serial.println("start");
    for(;;)
    {
      PT_DELAY(pt,1000,ts);
      timemin++;
      if(digitalRead(button)==0)
      {push=0;
       Serial.println("end");
       Serial.println(timemin);
       Serial1.print("1,");
       Serial1.println(timemin);
       timemin=0;
       pick=0;
       PT_DELAY(pt,500,ts);
       break;
       
      }
    }
  }
   PT_END(pt);
}
PT_THREAD(tasktem(struct pt* pt))
{
  static uint32_t ts;
  PT_BEGIN(pt);
  Serial.print("temp= ");
  Serial.println((25*analogRead(tem)-2050)/100);
  Serial1.print("2,");
  Serial1.println((25*analogRead(tem)-2050)/100);
  PT_DELAY(pt,60000,ts);
  PT_END(pt);
}
PT_THREAD(taskpr(struct pt* pt))
{
    static uint32_t ts;
    PT_BEGIN(pt);
    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");
    Serial1.print("3,");
    Serial1.println(bmp.readPressure());
    PT_DELAY(pt,60000,ts);
    PT_END(pt);
}
PT_THREAD(tasksou(struct pt* pt))
{
    static uint32_t ts;
    PT_BEGIN(pt);
    //Serial.print("volume= ");
    //Serial.println(analogRead(sou));
    Serial1.print("4,");
    Serial1.println(analogRead(sou));
    PT_DELAY(pt,3000,ts);
    PT_END(pt);
}
PT_THREAD(taskbuzz(struct pt*pt))
{
  static uint32_t ts;
  PT_BEGIN(pt);
  if(Serial1.available()) {
    up = Serial1.readStringUntil('\r');
    Serial.print(up);
    Serial1.flush();
  }
  if(up=="Wake")
  {
    digitalWrite(light,HIGH);
    for(;;)
    {
      analogWrite(buzz,250);
      PT_DELAY(pt,500,ts);
      analogWrite(buzz, 0);
      PT_DELAY(pt,500,ts);
      if(Serial1.available()) {
        up = Serial1.readStringUntil('\r');
        Serial.print(up);
        Serial1.flush();
      }
      if(up=="1" || up == "Cancel")
      {
        digitalWrite(light,LOW);  
        break;
      }
    }
  }
  PT_END(pt);
}
PT_THREAD(taskhu(struct pt*pt))
{
  static uint32_t ts;
  PT_BEGIN(pt);
  PT_DELAY(pt,10000,ts); 
  hum=dht.readHumidity();

  //Serial.print("Humidity: ");
  //Serial.println(hum);
  Serial1.print("6,");
  Serial1.println(hum);
  PT_END(pt);
}

void loop()
{
  taskbutton(&pt_taskbutton);
  taskgyro(&pt_taskgyro);
  tasktem(&pt_tasktem);
  //taskpr(&pt_taskpr);
  tasksou(&pt_tasksou);
  taskbuzz(&pt_taskbuzz);
  taskhu(&pt_taskhu);
}
