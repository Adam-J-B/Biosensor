/*
 set_potential.ino
 
 Measures current while holding the anode potential close to a specified value
 
 =============================================================
 ***** ONLY WORKS WITH CIRCUIT VERSION 4.3 **********
 =============================================================
  
 30 minutes in off mode for both anodes. 
 During off mode, log all of the same data as the other part of the sketch.
 
 Changes:
 - Using only one digital pot now - removed #2 (Analog 2 & 3)
 - Added ADS1115 library + code
 - Changed digipot cs pin to D7
 
 Last modified: 12/3/14 - Adam Burns - burns7@illinois.edu
 */
#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;
#include <SPI.h>
int power1state = LOW;
int power2state = LOW;
boolean offState = true;
const int Power1 = 6; // power control for MCP4161 #1 to D6
const int Power2 = 5; // power control for MCP4161 #2 to D5
unsigned long offDuration = 30000; // off mode for 30 mins (1,800,000ms)

int csPin1 = 7; //Chip select Digital Pin 7 for digital pot #1
int csPin2 = 3; //Chip select D3 for digital pot #2
int read1;
int read2;
int read3;
int read4;
int read5;
int trans_sig1 = 0;
int trans_sig2 = 0;
int cnt = 0;
double current2=0; 
double vol_nor2=0;
double cell_vol2=0;
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
  pinMode(csPin2, OUTPUT);
  pinMode(Power1, OUTPUT);
  pinMode(Power2, OUTPUT);
  digitalWrite(Power1, LOW);
  digitalWrite(Power2, LOW);
  
  
  // Reset digipot to 0
  digitalWrite(Power1, HIGH);
  delay(200);
  digitalWrite(csPin1, LOW);
  SPI.transfer(0);
  SPI.transfer(0);
  digitalWrite(csPin1,HIGH);
  delay(200);
  digitalWrite(Power1, LOW);
  
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

void setPot(int csPin1, int value)//function currently unused
{
  digitalWrite(csPin1, LOW);
  SPI.transfer(0);
  SPI.transfer(value);
  digitalWrite(csPin1,HIGH);
}


void loop()
{
  cnt ++;

  if(offState==true){
    unsigned long currentMillis = millis();
    if((currentMillis>offDuration) && (offState==true))
    {
      power1state=HIGH;
      power2state=HIGH;
      offState=false;
      digitalWrite(Power1,power1state);
      digitalWrite(Power2,power2state);
      delay(150); //allow current to stabilize
    }
  }

  float multiplier = 0.125F; // ADS1115  1x gain   +/- 4.096V (16-bit results) 0.125mV
  //int16_t vol;

  double vol = ads.readADC_Differential_0_1();
  vol=vol*multiplier; //reading in mV
  double current = ((vol)/(98.2));

  double vol_nor = ads.readADC_Differential_2_3();
  vol_nor= (vol_nor * multiplier)/1000;

  double cell_vol = ads.readADC_SingleEnded(1);
  cell_vol=(cell_vol * multiplier)/1000; 

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
    if (vol_nor < -0.352) trans_sig1 ++;
    else if (vol_nor > -0.342 && trans_sig1 >0) trans_sig1 --;
  }

  digitalWrite(csPin1, LOW);
  SPI.transfer(0);
  SPI.transfer(trans_sig1);
  digitalWrite(csPin1, HIGH);
  delay(200); // let the state change stabilize

#if DEBUG
  Serial.println();
  Serial.print("trans_sig1: ");
  Serial.println(trans_sig1);
  Serial.print("current: ");
  Serial.print(current, numOfDigits);
  Serial.print(",  annode potential:");
  Serial.print(vol_nor, numOfDigits);
  Serial.print(",  Cell vol:");
  Serial.println(cell_vol, numOfDigits);
  Serial.print(",  vol_drop:");
  Serial.println(vol, numOfDigits);
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
  //  digitalWrite(power1LED,LOW);
  //  digitalWrite(power2LED,LOW);
}














