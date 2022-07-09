# kiln_ramp
Arduino code for reading kiln temperature and controlling the kilns power for a steady temperature ramp up and cool down process.

Simple arduino control program to control my kiln power for a controlled heat ramp up, hold at max temp, and then ramp down in a controlled manner.  I use it to heat treat various cherts, agates, and jaspers to make them more workable for flint knapping and need to be able to adjust to various "heat treat" or cooking temperatures,  See Pudget Sound Knappers for information and guidance on heat treating rocks for knapping.
http://www.pugetsoundknappers.com/how_to/Heat%20Treating%20Guide%20with%20Table.html

My setup is a simple MAX6675 thermocouple reader and a zero crossing solid state relay to turn on the kiln power.
