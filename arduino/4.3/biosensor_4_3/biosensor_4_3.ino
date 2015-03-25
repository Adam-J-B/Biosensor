




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
    Serial.begin(9600);
    // initialize Timer1
    cli();          // disable global interrupts
    TCCR1A = 0;     // set entire TCCR1A register to 0
    TCCR1B = 0;
    TCNT1 = 65275;  // preload timer (2^16 = 65536)-16MHz/1024/60Hz
    // enable Timer1 overflow interrupt:
    TIMSK1 = (1 << TOIE1);
    // Set CS10 bit so timer runs at clock speed:
    TCCR1B |= (1 << CS12) | (1 << CS10);
    // enable global interrupts:
    sei();
}

loop()
{

}

ISR(TIMER1_OVF_vect)
{
    TCNT1 = 65275;
    Serial.print("Time: ");
    time = micros();
    //prints time since program started
    Serial.println(time);
}

void readADC()
{
    vol = (ads.readADC_Differential_0_1()) * MULTIPLIER; // Voltage reading
    current = ((vol) / (RESISTANCE)); // ohm's law
    anodePotential = ((ads.readADC_Differential_2_3()) * MULTIPLIER) / 1000;
    cell_vol = ((ads.readADC_SingleEnded(1)) * MULTIPLIER) / 1000;
}
