//MAX6675 Setup
#include <max6675.h>
//The usual MAX6675 pinouts
int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

//Power control pin (relay)
int pwr=8;

// 'F' or 'f' for fafhrenheit temp settings and readings.  Anything else for Celsius.
char tempType = 'F';

//startPeriod should usually be left at 0
//warmupTemp is warm-up temperature to begin from.  Careful not to set higher than you can get to in one period.
//warmupSoakPeriod The amount of periods to soak the warmup.  (The .34 default, at the defaut period length, is 20 mins)   This is useful if you have rocks that need to dry a little bit before proceeding, or that are sensitive to thermal shock.
//upRampTemp is the temperature increment that will be added each period till reaching soakTemp.  Careful not to set higher than you can achieve in a single period.
//soakTemp is the maximum temperature you want to achieve
//soakPeriod is the amount of periods you want to hold at the maximum temp (soakTemp)
//downRampTemp is the temperature decrement per period for cooldown.  For instant cool down, rather than a phased (periodic) cool down, then set to a number greater than the difference between soakTemp and warmupTemp.
int startPeriod=0;
int warmupTemp=100;
double warmupSoakPeriod=.34;
int upRampTemp=25;
int soakTemp=390;
double soakPeriod=5;
int downRampTemp=20;

//Create period length (3600000UL = 1hr)
long pCount=3600000UL;
//long pCount=10000UL;
//long pCount=1000UL;

//Set the first temp
int tSet=warmupTemp;

//Calculate start holding period
long startHoldSec = pCount*warmupSoakPeriod;

void setup() {
  Serial.begin(9600);
  Serial.println("Chuck Roast 3000 Kiln Temp Controller");

  //Configure the power PIN and turn it off to start
  pinMode(pwr,OUTPUT);
  digitalWrite(pwr,LOW);

  //stabilize for a sec
  delay(1000);
    
  //Warm it up to starting temp
  Serial.print("Warming up to ");
  Serial.println(warmupTemp);
  int currTemp = round(readAvgTemp(tempType));
  while(currTemp < warmupTemp){
    digitalWrite(pwr,HIGH);
    int currTemp = round(readAvgTemp(tempType));
    printData(startPeriod, tSet, digitalRead(pwr), currTemp);
  }

  //Check to see if a warm-up soak time is set
  if(startHoldSec > 0){
    Serial.print("Warm-up soaking for ");
    Serial.print(startHoldSec/1000/60);
    Serial.print(" minutes (");
    Serial.print(startHoldSec/1000);
    Serial.println(" seconds)");
    
    //Hold the warm-up soak temp for the assigned period
    while(startHoldSec > 0){
      if(currTemp < warmupTemp){
        digitalWrite(pwr,HIGH);
      }else{
        digitalWrite(pwr,LOW);
      }
      startHoldSec-=10000;
      printData(startHoldSec/1000, tSet, digitalRead(pwr), currTemp);
      int currTemp = round(readAvgTemp(tempType));
    }
  }
}

void loop() {
   //Read the current temp
   //int currTemp=thermocouple.readFahrenheit();
   //Calculate moving average temp for 5 seconds
   //max6675 has a large natural variance an average temp calculation smooths it out
   int currTemp = round(readAvgTemp(tempType));
   
   //Determine the current period
   unsigned long currPeriod=millis()/pCount;

   //Check if we're in a new period and adjust settings
   //We'll stop adjusting the tempSet when we drop below the original warmupTemp 
   if(currPeriod+1 > startPeriod and tSet>=warmupTemp){
     startPeriod++;
     //If we've reached targeted hold period, remove a hold period from the count
     //Else If there are no more hold periods decrease the temperature set by dRampTemp
     //Else increase the temperature set by uRampTemp
     if(tSet==soakTemp and soakPeriod>0){
       tSet=soakTemp;
       soakPeriod--;
     }else if(soakPeriod==0){
       tSet=tSet-downRampTemp;
     }else{
       tSet=tSet+upRampTemp;
       //If we've reached the target hold temp, this is the first period and well remove a soakPeriod count
       if(tSet==soakTemp){
         soakPeriod--;
       }
     }      
   }

   //safety catch for if the user has mismatched upRampTemp increments and soakTemp maximum temp, as our example settings do (25 upRampTemp will overshoot soakTemp)
   if(tSet>=soakTemp){
     tSet=soakTemp;
   }

   //Check the temp and compare to the current setting - turn power ((relay) on or off accordingly
   if(currTemp<tSet){
      digitalWrite(pwr,HIGH);
   }else{
      digitalWrite(pwr,LOW); 
   }

   printData(startPeriod, tSet, digitalRead(pwr), currTemp);
      
}

void printData(long period, int tempSet, int power, int temp){
  Serial.print("period:");
  Serial.print(period);
  Serial.print("|tSet:");
  Serial.print(tempSet);
  Serial.print("|power:");
  Serial.print(power);
  Serial.print("|");
  Serial.print(tempType);
  Serial.print(":");
  Serial.println(temp);
}

float readAvgTemp(char tempType){
  //Explanation of temp "flutter" and moving avg algorithm courtesy Robert's Smorgasbord Youtube Channel
  //https://www.youtube.com/watch?v=qub3yzqEwek
  //There are many unexpected details about the MAX6675 explained that are worth knowing
  //including some grounding issues with most thermocouples - generating unexpected readings.
  
  static float mov_avg = -100;

  //sample the temp 24 times, once every 250 ms, to develop a good moving average/stable temp reading
  for(int i=0; i<=24; i++){
    const float mov_avg_alpha = 0.1;
    double value;
    //minimum wait time to read a MAX6675 is 220ms, but most recommend 250ms minimum wait.
    delay(250);
    if(tempType == 'F' or tempType == 'f'){
      value=thermocouple.readFahrenheit();
    }else{
      value=thermocouple.readCelsius();
    }

    if(mov_avg == -100) mov_avg=value;
    
    mov_avg = mov_avg_alpha * value + (1 - mov_avg_alpha) * mov_avg;
  }
  
  return mov_avg;
}
