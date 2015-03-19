/*
 set_potential_v4_3.ino
 
 Version: 1.0.3
 
 Measures current while holding the anode potential close to a specified value
 
 
 =============================================================
 ***** ONLY WORKS WITH CIRCUIT VERSION 4.3 **********
 =============================================================
 
 30 minutes in open circui mode . 
 During off mode, log all of the same data as the other part of the sketch.
 
 Last modified: March 17, 2015
 
 
 1.0.0 Changes:
 - Using only one digital pot now - removed #2 (Analog 2 & 3)
 - Added ADS1115 library + code
 - Changed digipot cs pin to D7
 - Fixed issue with digitpot CS pin 
 - Removed code for 2nd digipot
 
  1.0.1 Changes:
  - Refactored anodePotential to anodePotential

  1.0.2 Changes:
  - Added comments 

  1.0.3 Changes:
  - Refactored trans_sig to digiPotValue

 Adam Burns - burns7@illinois.edu
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
int digiPotValue = 0; // Value sent to digipot
int cnt = 0;
double current=0; 
double anodePotential=0;
double cell_vol=0;
int numOfDigits = 5;

#define DEBUG 0 // set to 1 to print debug data to serial monitor


/*#############################################################################
##############################  Setup Function  ###############################
#############################################################################*/
void setup()
{
  Serial.begin(9600);
  SPI.begin();
  
// ==================== ADC Initilization =========================
  ads.begin();
  ads.setGain(GAIN_ONE); // 1x gain,  +/- 4.096V, 1 bit = 0.125mV
  
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


  


}
/*######################### end setup function ##############################*/

void setPot(int pin, int value)//function currently unused
{
  digitalWrite(pin, LOW);
  SPI.transfer(0);
  SPI.transfer(value);
  digitalWrite(pin,HIGH);
}


/*#############################################################################
############################ Loop Function ####################################
#############################################################################*/
void loop()
{
  
  
  float multiplier = 0.125F; // ADS1115  1x gain   +/- 4.096V (16-bit results) 0.125mV
  double anodePotential;
  double current;
  double cell_vol;
  double vol;
  
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


  if(offState==false){
    if (anodePotential < -0.352) digiPotValue ++;
    else if (anodePotential > -0.342 && digiPotValue >0) digiPotValue --;
  }

  digitalWrite(csPin1, LOW);
  SPI.transfer(0);
  SPI.transfer(digiPotValue);
  digitalWrite(csPin1, HIGH);
  delay(200); // let the state change stabilize




  /* =====================================================
  ============== Take ADC Readings =======================
  ======================================================*/
vol = (ads.readADC_Differential_0_1())*multiplier;  // Voltage reading
current = ((vol)/(98.2));  // ohm's law
anodePotential = ((ads.readADC_Differential_2_3())*multiplier)/1000;
cell_vol = ((ads.readADC_SingleEnded(1))*multiplier)/1000;


/* =====================================================
===================== Diagnostic =======================
======================================================*/
#if DEBUG
  Serial.println();
  Serial.print("digiPotValue: ");
  Serial.println(digiPotValue);
  Serial.print("current: ");
  Serial.print(current, numOfDigits);
  Serial.print(",  annode potential:");
  Serial.print(anodePotential, numOfDigits);
  Serial.print(",  Cell vol:");
  Serial.print(cell_vol, numOfDigits);
  Serial.print(",  vol_drop:");
  Serial.print(vol, numOfDigits);
  Serial.println();
#endif


  if (cnt == 6){
    // Print ADC readings to serial momnitor
    cnt = 0;
    Serial.print(current, numOfDigits);
    Serial.print("  ");
    Serial.print(anodePotential, numOfDigits);
    Serial.print("  ");
    Serial.println(cell_vol, numOfDigits);

  }

  delay(10000);

}
