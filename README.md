# Chuck Roast 3000
Arduino code for reading kiln temperature and controlling the kilns power for a steady temperature ramp up and cool down process.

Simple arduino control program to control my kiln power for a controlled heat ramp up, hold at max temp, and then ramp down in a controlled manner.  I use it to heat treat various cherts, agates, and jaspers to make them more workable for flint knapping and need to be able to adjust to various "heat treat" or cooking temperatures,  See Pudget Sound Knappers for information and guidance on heat treating rocks for knapping.
http://www.pugetsoundknappers.com/how_to/Heat%20Treating%20Guide%20with%20Table.html

My setup is a simple MAX6675 thermocouple reader and a zero crossing solid state relay to turn on the kiln power.

There is no reason this shouldn't work for knappers out there using turkey roasters as well.

Demonstration video:  https://youtu.be/Y11y6WhBYUg

kiln_ramp_2:
A seperate version "kiln_ramp_2" represents a more versatile (albeit more verbose code) state management.  The original still retains some value as a small and simple version, so it has been left in that form.  Additionally in "kiln_ramp_2" an OLED SH1106 display functionality has been added and can easiy be enabled/disabled (Default:Disabled).  A reconfiguration of the Serial monitor output allows it to be enabled/disabled (Default:Enabled).  The OLED GFX library is especially "hefty" and consumes a lot of memory.  Enabling both the Serial and OLED will consume excessive amounts of memory and some output (both Serial and OLED) information seems to suffer.



Changelog:
----------

20220905
- Added second version "kiln_ramp_2" (see description above)
- _2 added OLED support
- _2 Enable/Disable Serial Monitor
- _2 Enable/Disable OLED dispaly0
- _2 Added more versatile state management scheme

20220808
- Added parameter (tempType) for Celsius or Fahrenheit temperature and readings
- Added function (readAvgTemp) to remove "flutter" in MAX6675 readings.  Temp moving average calculated by 24 readings over 5 seconds

20220731
- Changed parameter names to more closely match common kiln nomenclature
- Added safety catch for mismatch upRampTemp to final soakTemp

20220730
- Added parameters for soaking (hold) of the warm-up cycle
- Changed soaking (hold) parameters to double for portioning of a period.
