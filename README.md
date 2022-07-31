# Chuck Roast 3000
Arduino code for reading kiln temperature and controlling the kilns power for a steady temperature ramp up and cool down process.

Simple arduino control program to control my kiln power for a controlled heat ramp up, hold at max temp, and then ramp down in a controlled manner.  I use it to heat treat various cherts, agates, and jaspers to make them more workable for flint knapping and need to be able to adjust to various "heat treat" or cooking temperatures,  See Pudget Sound Knappers for information and guidance on heat treating rocks for knapping.
http://www.pugetsoundknappers.com/how_to/Heat%20Treating%20Guide%20with%20Table.html

My setup is a simple MAX6675 thermocouple reader and a zero crossing solid state relay to turn on the kiln power.

There is no reason this shouldn't work for knappers out there using turkey roasters as well.

Demonstration video:  https://youtu.be/Y11y6WhBYUg



Changelog:
----------

20220731
- Changed parameter names to more closely match common kiln nomenclature
- Added safety catch for mismatch upRampTemp to final soakTemp

20220730
- Added parameters for soaking (hold) of the warm-up cycle
- Changed soaking (hold) parameters to double for portioning of a period.
