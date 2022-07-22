//MAX6675 Setup
#include <max6675.h>
//The usual MAX6675 pinouts
int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

//Power control pin (relay)
int pwr=8;

//startPeriod should usually be left at 0
//startTemp is warm-up temperature to begin from.  Careful not to set higher than you can get to in one period.
//uRampTemp is the temperature increment that will be added each period till reaching holdTemp.  Careful not to set higher than you can achieve in a single period.
//holdTemp is the maximum temperature you want to achieve
//holdPeriod is the amount of periods you want to hold at the maximum temp (holdTemp)
//dRampTemp is the temperature decrement per period for cooldown.  For instant cool down, rather than a phased (periodic) cool down, then set to a number greater than the difference between holdTemp and startTemp.
int startPeriod=0;
int startTemp=100;
int uRampTemp=25;
int holdTemp=400;
int holdPeriod=5;
int dRampTemp=25;

//Create period length (3600000UL = 1hr)
long pCount=3600000UL;
//long pCount=10000UL;
//long pCount=1000UL;

//Set the first temp
int tSet=startTemp;

void setup() {
  Serial.begin(9600);
  Serial.println("Kiln Temp Controller - 400x4hrs 21+hr runtime");

  //Configure the power PIN and turn it off to start
  pinMode(pwr,OUTPUT);
  digitalWrite(pwr,LOW);

  //stabilize for a sec
  delay(1000);

  //Warm it up to starting temp
  while(thermocouple.readFahrenheit() < startTemp){
    digitalWrite(pwr,HIGH);
    delay(10000);
    printData(startPeriod, tSet, digitalRead(pwr), thermocouple.readFahrenheit());
  }
}

void loop() {
   //Read the current temp
   int currTemp=thermocouple.readFahrenheit();
   //Determine the current period
   unsigned long currPeriod=millis()/pCount;

   //Check if we're in a new period and adjust settings
   //We'll stop adjusting the tempSet when we drop below the original startTemp 
   if(currPeriod+1 > startPeriod and tSet>=startTemp){
     startPeriod++;
     //If we've reached targeted hold period, remove a hold period from the count
     //Else If there are no more hold periods decrease the temperature set by dRampTemp
     //Else increase the temperature set by uRampTemp
     if(tSet==holdTemp and holdPeriod>0){
       tSet=holdTemp;
       holdPeriod--;
     }else if(holdPeriod==0){
       tSet=tSet-dRampTemp;
     }else{
       tSet=tSet+uRampTemp;
       //If we've reached the target hold temp, this is the first period and well remove a holdPeriod count
       if(tSet==holdTemp){
         holdPeriod--;
       }
     }      
   }

   //Check the temp and compare to the current setting - turn power ((relay) on or off accordingly
   if(currTemp<tSet){
      digitalWrite(pwr,HIGH);
   }else{
      digitalWrite(pwr,LOW); 
   }

   printData(startPeriod, tSet, digitalRead(pwr), currTemp);
      
   // For the MAX6675 to update, you must delay AT LEAST 250ms between reads!
   //We'll do 10 seconds to keep from bouncing too much when temps are borderline.
   delay(10000);
}

void printData(int period, int tempSet, int power, int temp){
  Serial.print("period:");
  Serial.print(period);
  Serial.print("|tSet:");
  Serial.print(tempSet);
  Serial.print("|power:");
  Serial.print(power);
  Serial.print("|F:");
  Serial.println(temp);
}