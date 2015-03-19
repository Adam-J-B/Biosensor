




#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <SPI.h>

#define MULTIPLIER 0.125F   // ADS1115  1x gain   +/- 4.096V (16-bit results) 0.125mV
#define RESISTANCE 98.2     // @TODO


Adafruit_ADS1115 ads;



double anodePotential;
double current;
double cell_vol;
double vol;

setup()
{
    
}

loop()
{
    
}

void readADC()
{
    vol = (ads.readADC_Differential_0_1())*MULTIPLIER;  // Voltage reading
    current = ((vol)/(RESISTANCE));  // ohm's law
    anodePotential = ((ads.readADC_Differential_2_3())*MULTIPLIER)/1000;
    cell_vol = ((ads.readADC_SingleEnded(1))*MULTIPLIER)/1000;
}
