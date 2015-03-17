/*
 cv_v4_3.ino
 
 Version: 1.0.2
 
 Measures current while cycling the anode potential at a set rate
   - Start with both off for 30 minutes, then run CV using initial potential
 as starting & ending point (per cycle)
 
 
 ==================================================
 ***ONLY WORKS ON CIRCUIT V4.3********
 ==================================================
 
 Last modified: March 17, 2015
  
 1.0.1 Changes:
  - Refactored vol_nor to anodePotential
  
 1.0.2 Changes:
  - Added comments
 

 
 Adam Burns - burns7@illinois.edu
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;
int power1state = LOW;
int power2state = LOW;
boolean offState = true;
boolean setPotential = false;

int StablePotential = 0;
const int Power1 = 6; // power control for MCP4161 #1 to D6
const int Power2 = 5; // power control for MCP4161 #2 to D5
unsigned long offDuration = 300000; // off mode for 30 mins (1,800,000ms)
int numOfDigits = 5;
int netCycles=2; // target number of cycles
int delayInterval = 1000; // rate of sweep (1000=1s -> 1mv/1s)
int csPin1 = 7; //Chip select Digital Pin 7 for digital pot #1
int csPin2 = 3; //Chip select D3 for digital pot #2

int trans_sig = 0;
int cnt = 0;
int resistor = 98.2; // R2 resistance in Ohms
int lsv_finished = 0;

double target_value;
double startingPotential=0;
//double tol = 0.001;
#define DEBUG 0 // set to 1 to print debug data to serial monitor


/*#############################################################################
##############################  Setup Function  ###############################
#############################################################################*/
void setup()
{
  Serial.begin(9600);
  SPI.begin(); //Init SPI
  
// ==================== ADC Initilization =========================
  ads.begin();
  ads.setGain(GAIN_ONE); // 1x gain,  +/- 4.096V, 1 bit = 0.125mV


  pinMode(csPin1, OUTPUT);
  pinMode(csPin2, OUTPUT);

  pinMode(Power1, OUTPUT);
  pinMode(Power2, OUTPUT);
  digitalWrite(Power1,HIGH);
  delay(200);
  digitalWrite(Power2, LOW);

  // Reset digipot to 0
  digitalWrite(csPin1, LOW);
  SPI.transfer(0);
  SPI.transfer(0);
  digitalWrite(csPin1, HIGH);
  delay(200);

  // Setting digipot to 0 again in case 1st attempt failed
  digitalWrite(csPin1, LOW);
  SPI.transfer(0);
  SPI.transfer(0);
  digitalWrite(csPin1, HIGH);
  delay(200);
  digitalWrite(Power1, LOW);

}
/*######################### end setup function ##############################*/


/*#############################################################################
############################ Loop Function ####################################
#############################################################################*/
void loop()
{
  float multiplier = 0.125F; // ADS1115  1x gain   +/- 4.096V (16-bit results) 0.125mV
  double anodePotential;
  double last_anode;
  double current;
  double cell_vol;
  double vol;
  
  
  if(offState==true){    
    unsigned long currentMillis = millis();
    if((currentMillis>offDuration) && (offState==true))
    {
      power1state=HIGH;
      power2state=HIGH;
      offState=false;
      setPotential=true;
      digitalWrite(Power1,power1state);
      digitalWrite(Power2,power2state);
      delay(150); //allow current to stabilize
    }
  }


  if((offState==false) && (setPotential==true) && (StablePotential<=5))
  {

  /* =====================================================
  ============== Take ADC Readings =======================
  ======================================================*/
  vol = (ads.readADC_Differential_0_1()) * multiplier; // Voltage reading
  current = ((vol) / (98.2)); // ohm's law
  anodePotential = ((ads.readADC_Differential_2_3()) * multiplier) / 1000;
  cell_vol = ((ads.readADC_SingleEnded(1)) * multiplier) / 1000;
    
    if (anodePotential < -0.352) trans_sig ++;
    else if (anodePotential > -0.342 && trans_sig >0) trans_sig --;

    digitalWrite(csPin1, LOW);
    SPI.transfer(0);
    SPI.transfer(trans_sig);
    digitalWrite(csPin1, HIGH);
    delay(500);
    
    if((anodePotential>=-0.36) && (anodePotential<= -0.33)){
      StablePotential++;
    }

  }

  if (StablePotential>5){
    setPotential=false;
  }

  /* =====================================================
  ============== Take ADC Readings =======================
  ======================================================*/
  vol = (ads.readADC_Differential_0_1()) * multiplier; // Voltage reading
  current = ((vol) / (98.2)); // ohm's law
  anodePotential = ((ads.readADC_Differential_2_3()) * multiplier) / 1000;
  cell_vol = ((ads.readADC_SingleEnded(1)) * multiplier) / 1000;

#if DEBUG
  Serial.println();
  Serial.print("trans_sig: ");
  Serial.print(trans_sig);
  Serial.print(",  current: ");
  Serial.print(current, numOfDigits);
  Serial.print(",  annode potential:");
  Serial.print(anodePotential, numOfDigits);
  Serial.print(",  Cell vol:");
  Serial.print(cell_vol, numOfDigits);
  Serial.print(", Cnt: ");
  Serial.println(cnt);
  Serial.print(", Target: ");
  Serial.println(target_value, numOfDigits); 
#endif

  if (cnt==0){
    target_value=anodePotential;
    startingPotential=anodePotential;
  }

  //========== begin new code ============

  if((offState==false)&&(setPotential==false)){
    startingPotential=anodePotential;

    for(int i=0; i<netCycles; i++){

      // increase until target > 0
      while(target_value<=0){
        cnt ++;
        if (cnt % 2 == 0 && cnt != 0) 
        {
          target_value += 0.002;
        }
        if (cnt < 10) target_value = anodePotential; 
        else {
          if (cnt % 1 ==0)
          {
            if (anodePotential < target_value)
            {
              trans_sig ++;
              if(trans_sig > 255){ 
                trans_sig = 255; 
              }
              digitalWrite(csPin1, LOW);
              SPI.transfer(0);
              SPI.transfer(trans_sig);
              digitalWrite(csPin1, HIGH);
              delay(100); // allow voltage to stabilize before reading
            }
          }
        }
        
  /* =====================================================
  ============== Take ADC Readings =======================
  ======================================================*/
  vol = (ads.readADC_Differential_0_1()) * multiplier; // Voltage reading
  current = ((vol) / (98.2)); // ohm's law
  anodePotential = ((ads.readADC_Differential_2_3()) * multiplier) / 1000;
  cell_vol = ((ads.readADC_SingleEnded(1)) * multiplier) / 1000;

        Serial.print(trans_sig);
        Serial.print("      ");
        Serial.print(current, 10);
        Serial.print("  ");
        Serial.print(anodePotential, 10);
        Serial.print("  ");
        Serial.print(cell_vol, 10);
        Serial.println("  ");


        delay(delayInterval);
      }



      // decrease until trans_sig = 0
      while(trans_sig>0){
        cnt++;

        if (cnt % 2 == 0 && cnt != 0) 
        {
          target_value -= 0.002;
        }
        if (cnt % 1 ==0)
        {
          if (anodePotential > target_value)
          {
            trans_sig --;
            if(trans_sig <=0 ){ 
              trans_sig = 0; 
            }
            digitalWrite(csPin1, LOW);
            SPI.transfer(0);
            SPI.transfer(trans_sig);
            digitalWrite(csPin1, HIGH);
            delay(100); // allow voltage to stabilize
          }
        }
        
  /* =====================================================
  ============== Take ADC Readings =======================
  ======================================================*/
  vol = (ads.readADC_Differential_0_1()) * multiplier; // Voltage reading
  current = ((vol) / (98.2)); // ohm's law
  anodePotential = ((ads.readADC_Differential_2_3()) * multiplier) / 1000;
  cell_vol = ((ads.readADC_SingleEnded(1)) * multiplier) / 1000; 

        Serial.print(trans_sig);
        Serial.print("      ");
        Serial.print(current, 10);
        Serial.print("  ");
        Serial.print(anodePotential, 10);
        Serial.print("  ");
        Serial.print(cell_vol, 10);
        Serial.println("  ");


        delay(delayInterval);
      }


      // increase back up to startingPotential
      while(target_value<startingPotential){

        cnt ++;
        if (cnt % 2 == 0 && cnt != 0) 
        {
          target_value += 0.002;
        }
        if (cnt < 10) target_value = anodePotential; 
        else {
          if (cnt % 1 ==0)
          {
            if (anodePotential < target_value)
            {
              trans_sig ++;
              if(trans_sig > 255){ 
                trans_sig = 255; 
              }
              digitalWrite(csPin1, LOW);
              SPI.transfer(0);
              SPI.transfer(trans_sig);
              digitalWrite(csPin1, HIGH);
              delay(100); // allow voltage to stabilize
            }
          }
        }
  /* =====================================================
  ============== Take ADC Readings =======================
  ======================================================*/
  vol = (ads.readADC_Differential_0_1()) * multiplier; // Voltage reading
  current = ((vol) / (98.2)); // ohm's law
  anodePotential = ((ads.readADC_Differential_2_3()) * multiplier) / 1000;
  cell_vol = ((ads.readADC_SingleEnded(1)) * multiplier) / 1000; 

        Serial.print(trans_sig);
        Serial.print("      ");
        Serial.print(current, 10);
        Serial.print("  ");
        Serial.print(anodePotential, 10);
        Serial.print("  ");
        Serial.print(cell_vol, 10);
        Serial.println("  ");


        delay(delayInterval);
      }

    }
    Serial.println("  ");
    Serial.println(" CV completed ");

    //halt the code
    while(true);
  }


  delay(1000);

}
