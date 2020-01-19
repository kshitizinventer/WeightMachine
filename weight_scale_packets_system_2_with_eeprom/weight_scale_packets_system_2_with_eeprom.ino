#include "HX711.h"
#include "EEPROM.h"
HX711 scale(7,8);
//HX711 scale(10,11);
long int list[20];
long int avgzero=0;
long int avgeighty=0;;
int stage=0;
int i=0;
int eightyiscalibrated=0;
long int packet[4];
long int avgofcurrentpacket=0;
long int avgofpreviouspacket=-50;
int avgofcalibrationweight=148;

int tempflag1=0;
int k=0;
int flag=0;
//stage 0 is averaging the zero weight mark
//stage 1 is averaging the the 80gm weight mark

void setup() {
  pinMode(13,INPUT);
  pinMode(3,OUTPUT);
  pinMode(12,OUTPUT);
  Serial.begin(9600);
//  Serial.println("you are in setup");
   avgzero=EEPROM.read(80);
   avgeighty=EEPROM.read(180); 
}

void loop() {
 
 if(digitalRead(13)==HIGH)
 {
  calibrate();
 }
 else
  normalcode();

//  Serial.println("This is good man");
}

void calibrate()
{
  Serial.println("yo man you are in calibrate function");
  flag=0;
  stage=0;
  avgzero=0;
  avgeighty=0;
  eightyiscalibrated=0;
 while(stage<2)
 { 
    //if(stage<1) 
 // Serial.println(scale.read())
 
// this if condition is to check if 80 gm weight is detected
// if so then eightyiscalibrated is given true which takes the calibration code to its final stage
 if(stage==1 and !eightyiscalibrated)
 {
  if(abs(scale.read()-(avgzero+6000)) > 0)
   { i=0;
    eightyiscalibrated=1;
    Serial.println("80 gm weight detected");
   }else
   {
    Serial.println("stage 1 reached");
    Serial.println(scale.read());
   }
 }
 if(i<20)
 {
   list[i]=scale.read();
   i++;
 }
 
 if(i==20)
 {
   if(stage==0)
   {
    for(int r=0;r<20;r++)
    avgzero=avgzero+list[r];

    avgzero=avgzero/20;
    Serial.println(avgzero);
    Serial.println("OK now put on weight a 100gm weight otherwise the program will not continue");
    stage=1;// the zero is calibrated
   }
   else if(stage==1 and eightyiscalibrated)
   { 
     for(int r=5;r<20;r++)
      avgeighty=avgeighty+list[r];

      avgeighty=avgeighty/15;
      Serial.println(avgeighty);
      Serial.println("Calibration is completed");
      stage=2;

      EEPROMWritelong(80,avgzero);
      EEPROMWritelong(180,avgeighty);
      Serial.println(EEPROMReadlong(80));
      Serial.println(EEPROMReadlong(180));          
      
   }
   if(stage==2)
   {
    Serial.println("Program wont continue if calibration switch isnt turned off");
    while(digitalRead(13)== HIGH);
   }  
  }// end of i=10
 }// end of while loop
}// end of calibration 
void normalcode()
{
  if(flag==0)
  {
    avgzero = EEPROMReadlong(80);
    avgeighty = EEPROMReadlong(180);
    flag=1;
  }
  
  //Serial.println("normal code running");
 // stage 2 will start when when the calibration is completed
 stage=2; 
  tempflag1=0;
  boolean isplanar;
  while(k<4)
   {
    long int weight = ((scale.read()-avgzero)*avgofcalibrationweight)/(avgeighty-avgzero);
    packet[k]=weight;
    if((abs(packet[k]-packet[k-1])>20) and tempflag1==0 and k>0)
    {
     
     Serial.println("Weight has been changed");
     //Serial.println(tempflag1);
     tempflag1=1;
  
    }
    k++;
    //Serial.println(avgzero); 
    if(weight>500000 or weight<-100000)
    {
     Serial.println("something is up looks like you need to recalibrate");
     Serial.println(weight);
    }
   }
   
   if( abs(packet[1]-packet[0])<=10 and abs(packet[2]-packet[0])<=10 and abs(packet[3]-packet[0])<=10)
     isplanar=true;
   else
    isplanar=false;

  
    if(isplanar){
      avgofcurrentpacket = (packet[0]+ packet[1]+ packet[2]+packet[3])/4;
      if( abs(avgofcurrentpacket-avgofpreviouspacket)>10 and avgofpreviouspacket!=-50)
      {
        if((avgofcurrentpacket-avgofpreviouspacket)>40)
        {
         //Serial.println("looks like a weight is added");
         Serial.println(avgofcurrentpacket-avgofpreviouspacket);
         //Serial.println(avgofcurrentpacket);
         digitalWrite(12,HIGH);
         delay(500);
         digitalWrite(12,LOW);
        }
        else if((avgofcurrentpacket-avgofpreviouspacket)<-40)
        {
        // Serial.println("looks like weight is sutbtracted");
         Serial.println(avgofcurrentpacket-avgofpreviouspacket);
         //Serial.println(avgofcurrentpacket);
         digitalWrite(12,HIGH);
         delay(500);
         digitalWrite(12,LOW);
         
        }
      }
        avgofpreviouspacket = avgofcurrentpacket;
    }
    k=0; 
    packet[0]=0;
    packet[1]=0;
 
  
}// end of normalcode

void EEPROMWritelong(int address, long value)
      {
      //Decomposition from a long to 4 bytes by using bitshift.
      //One = Most significant -> Four = Least significant byte
      byte four = (value & 0xFF);
      byte three = ((value >> 8) & 0xFF);
      byte two = ((value >> 16) & 0xFF);
      byte one = ((value >> 24) & 0xFF);

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(address, four);
      EEPROM.write(address + 1, three);
      EEPROM.write(address + 2, two);
      EEPROM.write(address + 3, one);
      }
long EEPROMReadlong(long address)
      {
      //Read the 4 bytes from the eeprom memory.
      long four = EEPROM.read(address);
      long three = EEPROM.read(address + 1);
      long two = EEPROM.read(address + 2);
      long one = EEPROM.read(address + 3);

      //Return the recomposed long by using bitshift.
      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
      }

 // 208 lines of code      
