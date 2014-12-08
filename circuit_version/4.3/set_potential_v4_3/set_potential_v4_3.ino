/*
 set_potential_v4_3.ino
 
 Measures current while holding the anode potential close to a specified value
 
 =============================================================
 ***** ONLY WORKS WITH CIRCUIT VERSION 4.3 **********
 =============================================================
 
 30 minutes in open circui mode . 
 During off mode, log all of the same data as the other part of the sketch.
 
 Changes:
 - Using only one digital pot now - removed #2 (Analog 2 & 3)
 - Added ADS1115 library + code
 - Changed digipot cs pin to D7
 - Fixed issue with digitpot CS pin 
 - Removed code for 2nd digipot
 
 Last modified: 12/8/14 - Adam Burns - burns7@illinois.edu
 */
#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;
#include <SPI.h>
int power1state = LOW;
boolean offState = true;
const int Power1 = 6; // power control for MCP4161 #1 to D6
unsigned long offDuration = 300000; // off mode for 30 mins (1,800,000ms)

int csPin1 = 7; //Chip select Digital Pin 7 for digital pot #1
int trans_sig = 0; // Value sent to digipot
int cnt = 0;
double current=0; 
double vol_nor=0;
double cell_vol=0;
int numOfDigits = 5;

#define DEBUG 0 // set to 1 to print debug data to serial monitor

void setup()
{
  Serial.begin(9600);
  SPI.begin();
  ads.begin();
  pinMode(csPin1, OUTPUT);
  pinMode(Power1, OUTPUT);
  digitalWrite(Power1, LOW);

  delay(200);

  // Reset digipot to 0
  digitalWrite(Power1, HIGH);
  delay(200);
  digitalWrite(csPin1, HIGH);
  delay(200);
  digitalWrite(csPin1, LOW);
  SPI.transfer(0);
  SPI.transfer(0);
  digitalWrite(csPin1,HIGH);
  delay(200);

  // Setting digipot to 0 again in case 1st attempt failed
  digitalWrite(csPin1, LOW);
  SPI.transfer(0);
  SPI.transfer(0);
  digitalWrite(csPin1, HIGH);
  delay(200);
  digitalWrite(Power1, LOW);
  delay(200);

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  //
  //ADC Range: +/- 6.144V (1 bit = 0.1875mV)
  //
  //analogReference(INTERNAL);
}

void setPot(int pin, int value)//function currently unused
{
  digitalWrite(pin, LOW);
  SPI.transfer(0);
  SPI.transfer(value);
  digitalWrite(pin,HIGH);
}


void loop()
{
  cnt ++;

  if(offState==true){
    unsigned long currentMillis = millis();
    if((currentMillis>offDuration) && (offState==true))
    {
      power1state=HIGH;
      offState=false;
      digitalWrite(Power1,power1state);
      delay(150); // delay for stabilization
    }
  }

  float multiplier = 0.125F; // ADS1115  1x gain   +/- 4.096V (16-bit results) 0.125mV
  double vol_nor;
  double current;
  double cell_vol;



  /*-------- changed to using ads1115 --------------------
   read1 = analogRead(A0);
   delay(7);
   read2 = analogRead(A1);
   delay(7);
   read5 = analogRead(A4);
   double vol = (read1 - read2) * 1.0 /1023 * 1.1;
   double current = vol / 97 * 1000; //Ohm's Law to calculate current based on drop across 97 ohm resistor
   double vol_nor = (read1 - read5) * 1.0 / 1023 * 1.1; // calculate anode potential (A0) vs reference electrode (A4)
   double cell_vol = read1 * 1.0 / 1023 * 1.1;
   */
  if(offState==false){
    if (vol_nor < -0.352) trans_sig ++;
    else if (vol_nor > -0.342 && trans_sig >0) trans_sig --;
  }

  digitalWrite(csPin1, LOW);
  SPI.transfer(0);
  SPI.transfer(trans_sig);
  digitalWrite(csPin1, HIGH);
  delay(200); // let the state change stabilize



  double vol = ads.readADC_Differential_0_1();
  vol=vol*multiplier; //reading in mV
  current = ((vol)/(98.2)); // ohm law

  vol_nor = ads.readADC_Differential_2_3();
  vol_nor= (vol_nor * multiplier)/1000;

  cell_vol = ads.readADC_SingleEnded(1);
  cell_vol=(cell_vol * multiplier)/1000; 



#if DEBUG
  Serial.println();
  Serial.print("trans_sig: ");
  Serial.println(trans_sig);
  Serial.print("current: ");
  Serial.print(current, numOfDigits);
  Serial.print(",  annode potential:");
  Serial.print(vol_nor, numOfDigits);
  Serial.print(",  Cell vol:");
  Serial.print(cell_vol, numOfDigits);
  Serial.print(",  vol_drop:");
  Serial.print(vol, numOfDigits);
  Serial.println();
#endif


  /*--------------- only using one digital pot -----------------
   //setPot(csPin2, trans_sig2); // set pot #2
   delay(200); // let the state change stabilize
   read3 = analogRead(A2);
   delay(7);
   read4 = analogRead(A3);
   delay(7);
   read5 = analogRead(A4);
   double vol_nor2 = (read3 - read5) * 1.0 / 1023 * 1.1;
   double vol2 = (read3 - read4) * 1.0 /1023 * 1.1;
   double current2 = vol2 / 97 * 1000;
   double cell_vol2 = read3 * 1.0 / 1023 * 1.1;
   if (vol_nor2 < -0.352) trans_sig2 ++;
   else if (vol_nor2 > -0.342 && trans_sig2 >0) trans_sig2 --;
   
   digitalWrite(csPin2, LOW);
   SPI.transfer(0);
   SPI.transfer(trans_sig2);
   digitalWrite(csPin2, HIGH);  //-- moved to setPot function - 11/5/14 - AJB
   
   #if DEBUG
   Serial.println();
   Serial.print("trans_sig2: ");
   Serial.println(trans_sig2);
   #endif
   -----------------------------------------------------*/

  if (cnt == 6){
    //Printing
    cnt = 0;
    Serial.print(current, numOfDigits);
    Serial.print("  ");
    //Serial.print(current2, numOfDigits);
    //Serial.print("  ");
    Serial.print(vol_nor, numOfDigits);
    Serial.print("  ");
    //Serial.print(vol_nor2, numOfDigits);
    // Serial.print("  ");
    Serial.println(cell_vol, numOfDigits);
    //Serial.print("  ");
    //Serial.println(cell_vol2, numOfDigits);
  }

  delay(10000);

}















