//MAX6675 Setup
#include <max6675.h>
//The usual MAX6675 pinouts
#define thermoDO 4
#define thermoCS 5
#define thermoCLK 6
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

//Power control pin (relay)
#define pwr 8

// 'F' or 'f' for fafhrenheit temp settings and readings.  Anything else for Celsius.
#define tempType 'F'

//warmupTemp is warm-up temperature to begin from.  Careful not to set higher than you can get to in one period.
#define warmupTemp 100
//warmupSoakPeriod The amount of periods to soak at the warmup temp.  Useful for drying rocks before making steam or for rocks sensitive to thermal shock.
#define warmupSoakPeriod .34
//upRampTemp is the temperature increment that will be added each period till reaching soakTemp.  This allows you to carefully (or dangerously) creep up to final temp.
#define upRampTemp 25
//soakTemp is the maximum temperature you want to achieve.
#define soakTemp 290
//soakPeriod is the amount of periods you want to hold at the maximum temp (soakTemp)
#define soakPeriod 5
//downRampTemp is the temperature decrement per period for cooldown.  Some kilns/ovens cool faster than others and this can control a steady and slow cool down.
#define downRampTemp 20
// Temp Tolerance range as a percent (5 = 5%) Measuring moving average temp causes necessary lag in the reading.  This means temperature overshot of tSet regularly happens.
// This setting can better ease the device into the final temperature.  Set to 0 if its not an issue for you.
#define tempTolerance 2

//Create period length (3600000UL = 1hr)
#define pCount 3600000UL
//#define pCount 10000UL
//#define pCount 1000UL

//Turn debug (aka serial monitor output) on or off.  This "#define alias" approach has the advantage of saving room in the compiled sketch when turned off, while only writing one version of the code.
//To enable/disable change: DEBUG 0 <-> DEBUG 1
#define DEBUG 1
#if DEBUG == 1
#define debugbegin Serial.begin(9600)
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debugbegin
#define debug(x)
#define debugln(x)
#endif

//Turn on/off the OLED display. If you have one and want to use it, adjust your SCREEN_WIDTH and SCREEN_HEIGHT values
//Note that a display is extremely useful here, but the GFX library is a resource hog, likely to fill ~50% of your arduino memory.
//Also note that this setup is for the SH1106 OLED.
//You'll need to connect to the proper i2c interface and perhaps change the address of your OLED i2c
//My pro mini pinouts: scl -> a4  |  sda -> a5
//Reference: https://www.electroniclinic.com/arduino-oled-i2c-display-128x64-with-examples-wiring-and-libraries-issues-solved/
//To enable/disable change: OLED 0 <-> OLED 1
#define OLED 0
#if OLED == 1
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1
#define Adafruit_SH1106_display(x) Adafruit_SH1106 display(x)
#define displaybegin(x,y) display.begin(x, y)
#define displayclear display.clearDisplay()
#define displaysettextcolor(x) display.setTextColor(x)
#define displaysettextsize(x) display.setTextSize(x)
#define displaysetcursor(x,y) display.setCursor(x,y)
#define displayprint(x) display.print(x)
#define displaydisplay display.display()
#else
#define displaybegin(x,y)
#define displayclear
#define displaysettextcolor(x)
#define displaysettextsize(x)
#define displaysetcursor(x,y)
#define displayprint(x)
#define displaydisplay
#define Adafruit_SH1106_display(x)
#endif
Adafruit_SH1106_display(OLED_RESET);

void setup() {

  debugbegin;
  debugln("Chuck Roast 3000 Kiln Temp Controller");
  
  //Configure the power PIN and turn it off to start
  pinMode(pwr,OUTPUT);
  digitalWrite(pwr,LOW);

  displaybegin(SH1106_SWITCHCAPVCC, 0x3C);
  displayclear;
  displaysettextcolor(WHITE);
    
  //hold for just a second so everything gets powered up.  The thermocouple needs some time (at least 250ms) to power on and read.  Other breakouts may need just a sec as well.
  delay(1000);

  debugln("Setup Complete");
}

void loop() {
  int currTemp = readAvgTemp();

  stateCheck(currTemp);
}

int readAvgTemp(){
  //Explanation of temp "flutter" and moving avg algorithm courtesy Robert's Smorgasbord Youtube Channel
  //https://www.youtube.com/watch?v=qub3yzqEwek
  //There are many unexpected details about the MAX6675 explained that are worth knowing
  //including some grounding issues with most thermocouples - generating unexpected readings.
  
  unsigned long currTime = millis();
  static double mov_avg = 0;
  static int return_avg = 0;
  static unsigned long readMillis = currTime;
  static unsigned long returnMillis = currTime;

    if (currTime - readMillis >= 250) {
        const double mov_avg_alpha = 0.1;
        double value;
        if(tempType == 'F' or tempType == 'f'){
          value=thermocouple.readFahrenheit();
        }else{
          value=thermocouple.readCelsius();
        }
    
        if(mov_avg == 0) mov_avg=value;
        
        mov_avg = mov_avg_alpha * value + (1 - mov_avg_alpha) * mov_avg;
        readMillis = currTime;
    }
  
  if (currTime - returnMillis >= 5000) {
     return_avg = round(mov_avg);
  }
  
  return return_avg;
}

void stateCheck(int currTemp){
  enum class cookState : uint8_t {
        WARMUP = 0,
        WARMSOAK = 1,
        UPRAMP = 2,
        SOAK = 3,
        DOWNRAMP = 4,
        DONE = 5,
        ERRORSTATE= 6,
   };

   unsigned long currTime = millis();
   static unsigned long startTime = currTime;
   static unsigned long currStateStart = currTime;
   static cookState currState = cookState::WARMUP;
   static int tSet = warmupTemp;
   static int period = 0;
   static int startPeriod = 0;
   static bool changePeriod = false;
   //static String currPhase = "WarmUp";
   static int tempStatus = 0;
   static bool changeStatus = false;
   static bool powerStatus = false;
   
   switch(currState){
      case cookState::WARMUP:
        if(currTemp >= warmupTemp){
          currState = cookState::WARMSOAK;
          currStateStart = currTime;
          startPeriod = 1;
          startTime = currTime;
          period = 1;
          changeStatus = true;
        }
        break;

      case cookState::WARMSOAK:
        if(currTime - currStateStart >= (warmupSoakPeriod*pCount)){
          currState = cookState::UPRAMP;
          currStateStart = currTime;
          startPeriod = period;
          changeStatus = true;
        }
        break;

      case cookState::UPRAMP:
        if(changePeriod == true and (tSet + upRampTemp) < soakTemp){
          tSet += upRampTemp;
          changeStatus = true;
        }else if(changePeriod == true and (tSet + upRampTemp) >= soakTemp){
          currState = cookState::SOAK;
          currStateStart = currTime;
          startPeriod = period;
          changeStatus = true;
        }
        break;

      case cookState::SOAK:
        tSet = soakTemp;
        if(changePeriod == true and (period - startPeriod) > soakPeriod){
          currState = cookState::DOWNRAMP;
          currStateStart = currTime;
          startPeriod = period;
          changeStatus = true;
        }
        break;

      case cookState::DOWNRAMP:
        if(changePeriod == true and (tSet - upRampTemp) > warmupTemp){
          tSet -= downRampTemp;
          changeStatus = true;
        }else if(changePeriod == true and (tSet - downRampTemp) <= warmupTemp){
          currState = cookState::DONE;
          currStateStart = currTime;
          startPeriod = period;
          changeStatus = true;
        }
        break;

      case cookState::DONE:
        tSet = 0;
        changeStatus = false;
        break;

      default:
        currState = cookState::ERRORSTATE;
        tSet = 0;
        changeStatus = false;
        debugln("'Default' Switch Case reached - Error");        
   }

   //Turn on or off the kiln power based on the current temp vs the set temp
   if(currTemp<round(tSet-tSet*tempTolerance/100)){
      digitalWrite(pwr,HIGH);
   }else{
      digitalWrite(pwr,LOW); 
   }
   
   if(currTemp != tempStatus){
     tempStatus = currTemp;
     changeStatus = true;     
   }   
   
   if(digitalRead(pwr) != powerStatus){
     powerStatus = !powerStatus;
     changeStatus = true;     
   }

   //log/screen print if anything has changed
   if(changeStatus){
     writeData(int(currState), period, tSet, powerStatus, tempStatus, currTime);
     changeStatus = false;
   }
   
  if(floor((currTime - startTime)/pCount) >= period and currState>cookState::WARMUP){
     period += 1;
     changePeriod = true;
     changeStatus = true;
   }else{
     changePeriod = false;
   }
}

void printData(char currPhase[], long period, int tSet, char power[], int temp, unsigned long currTime){
  debug("_time:");
  debug(currTime);
  debug("|phase:");
  debug(currPhase);
  debug("|period:");
  debug(period);
  debug("|tempSet:");
  debug(tSet);
  debug("|power:");
  debug(power);
  debug("|temp:");
  debugln(temp);  
} 

void printScreenData(char currPhase[], long period, int tSet, char power[], int temp){
   //clear display
  displayclear;
  displaysettextsize(1);
  
  displaysetcursor(0,0);
  displayprint("Period: ");
  displaysetcursor(50,0);
  displayprint(period);
  
  displaysetcursor(75,0);
  displayprint(currPhase);
   
  displaysetcursor(0,15);
  displayprint("TempSet: ");
  displaysetcursor(50,15);
  displayprint(tSet);
 
  displaysetcursor(0,30);
  displayprint("Power: ");
  displaysetcursor(50,30);
  displayprint(power);

  displaysetcursor(0,45);
  displayprint("Temp: ");
  displaysetcursor(50,45);
  displayprint(temp);
  
  displaysetcursor(0,57);
  displayprint(" Chuck Roast 3K ");
  displaydisplay;
}

void writeData(int currPhase, long period, int tSet, bool power, int temp, unsigned long currTime){
    char* powerStatus[] = {"Off", "On"};
    
    char* phaseStatus[] = {"WarmUp", "WarmSoak", "UpRamp", "Soak", "DownRamp", "Done", "Error"};
        
    printScreenData(phaseStatus[currPhase], period, tSet, powerStatus[power], temp);
    printData(phaseStatus[currPhase], period, tSet, powerStatus[power], temp, currTime);
}
