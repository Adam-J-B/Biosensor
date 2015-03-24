/*
 lsv_v4_3.ino

 Version: 1.0.3

 Measures current while increasing anode potential at a set rate (1mV/s)


 ==================================================
 ***ONLY WORKS ON CIRCUIT V4.3********
 ==================================================

 Last modified: March 17, 2015

 1.0.0 Changes:
 - Moved digital pot communication to end of loop
 - Added ADS1115 library + code
 - Changed SPI CS pin for digipot to D7
 - MOSFET between cathod + digipot & GND - pin D6
 - Read from the ADC after changing the digipot digiPotValue

 1.0.1 Changes:
  - Refactored anodePotential to anodePotential
  - Moved ADC readings to end of loop after delay (Issue #9)

 1.0.2 Changes:
  - Added comments

 1.0.3 Changes:
  - Refactored trans_sig to digiPotValue

 Adam Burns - burns7@illinois.edu
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>

#define initTimerCnt  63973     //2^16  - 16MHz/1024/(10Hz = interrupt rate)
#define MULTIPLIER 0.125F   // ADS1115  1x gain   +/- 4.096V (16-bit results) 0.125mV
#define RESISTANCE 98.2     // @TODO
#define DEBUG 0 // set to 1 to print debug data to serial monitor

Adafruit_ADS1115 ads;
volatile double anodePotential;
volatile double current;
volatile double cell_vol;
volatile double vol;
volatile double anodePotentialROC;       //The rate of change of the anodePotential
volatile bool adjustDigiPot == false;

int power1state = LOW;
int power2state = LOW;
boolean offState = true;
const int Power1 = 6; // power control for MCP4161 #1 to D6
const int Power2 = 5; // power control for MCP4161 #2 to D5
unsigned long offDuration = 300000; // off mode for 30 mins (1,800,000ms)
int numOfDigits = 5;

int csPin1 = 7; //Chip select Digital Pin 7 for digital pot #1
int csPin2 = 3; //Chip select D3 for digital pot #2

int digiPotValue = 0;
int cnt = 0;
int resistor = 98.2; // R2 resistance in Ohms
int lsv_finished = 0;

double target_value;
//double tol = 0.001;

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
  digitalWrite(Power1, HIGH);
  digitalWrite(Power2, LOW);
  delay(200);

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
  //analogReference(INTERNAL);
  //delay(offDuration); // stay open for 30 minutes
  //digitalWrite(Power2, HIGH); // power on anode #2

    // initialize Timer1
    cli();          // disable global interrupts
    TCCR1A = 0;     // set entire TCCR1A register to 0
    TCCR1B = 0;
    TCNT1 = iniTimerCnt;
    // enable Timer1 overflow interrupt:
    TIMSK1 = (1 << TOIE1);
    // Set CS10 bit so timer runs at clock speed:
    TCCR1B |= (1 << CS12) | (1 << CS10);
    // enable global interrupts:
    sei();

}
/*######################### end setup function ##############################*/


/*#############################################################################
############################ Loop Function ####################################
#############################################################################*/
void loop()
{
    static bool firstTime = true;
    if (offState == true) { /*TODO Does the program start in the offstate?*/
        while(true){/*Keep Spinning*/ }
    }
    if(adjustDigiPot && offState == false)
    {

        Serial.println();
        Serial.print("digiPotValue: ");
        Serial.print(digiPotValue);
        Serial.print("      ");
        Serial.print(current, 10);
        Serial.print("  ");
        Serial.print(anodePotential, 10);
        Serial.print("  ");
        Serial.print(cell_vol, 10);
        Serial.println("  ");

        if(cnt == 0)
        {
            target_value = anodePotential;
        }


        if(/*TODO*/)
        {
            target_value += 0.002;
            if (target_value > 0)
            {
                lsv_finished = 1;
            }
        }
        if (lsv_finished == 0)
        {
            if (cnt < 10) target_value = anodePotential;
            else
            {
                if (anodePotential < target_value)
                {
                  digiPotValue ++;
                  if (digiPotValue > 255)
                  {
                        digiPotValue = 255;
                        Serial.println(" ERROR: Digipot exceeding max value (255)");
                  }
                  digitalWrite(csPin1, LOW);
                  SPI.transfer(0);
                  SPI.transfer(digiPotValue);
                  digitalWrite(csPin1, HIGH);
                }
            } 
        }
        else
        {
             digiPotValue = 0;
             digitalWrite(csPin1, LOW);
             SPI.transfer(0);
             SPI.transfer(digiPotValue);
             digitalWrite(csPin1, HIGH);
        }
        adjustDigiPot = false;
    }







 

//  if (offState == true) {
//    unsigned long currentMillis = millis();
//    if ((currentMillis > offDuration) && (offState == true))
//    {
//      power1state = HIGH;
//      power2state = HIGH;
//      offState = false;
//      digitalWrite(Power1, power1state);
//      digitalWrite(Power2, power2state);
//      delay(150); //allow current to stabilize
//    }
//  }

  if (cnt == 0) {
    target_value = anodePotential;
  }


  Serial.print(digiPotValue);
  Serial.print("      ");
  Serial.print(current, 10);
  Serial.print("  ");
  Serial.print(anodePotential, 10);
  Serial.print("  ");
  Serial.print(cell_vol, 10);
  Serial.println("  ");


  if (offState == false) {
    cnt ++;
    if (cnt % 2 == 0 && cnt != 0)
    {
      target_value += 0.002;
      if (target_value > 0)
      {
        lsv_finished = 1;
      }
    }

    if (lsv_finished == 0)
    {
      if (cnt < 10) target_value = anodePotential;
      else {
        if (cnt % 1 == 0)
        {
          if (anodePotential < target_value)
          {
            digiPotValue ++;
            if (digiPotValue > 255) {
              digiPotValue = 255;
              Serial.println(" ERROR: Digipot exceeding max value (255)");
            }
            digitalWrite(csPin1, LOW);
            SPI.transfer(0);
            SPI.transfer(digiPotValue);
            digitalWrite(csPin1, HIGH);
            delay(100); //allow voltage to stabilize
          }
        }
      }
    }
    else {
      digiPotValue = 0;
      digitalWrite(csPin1, LOW);
      SPI.transfer(0);
      SPI.transfer(digiPotValue);
      digitalWrite(csPin1, HIGH);
    }
  }



  delay(1000);


  /* =====================================================
  ============== Take ADC Readings =======================
  ======================================================*/
  vol = (ads.readADC_Differential_0_1()) * multiplier; // Voltage reading
  current = ((vol) / (98.2)); // ohm's law
  anodePotential = ((ads.readADC_Differential_2_3()) * multiplier) / 1000;
  cell_vol = ((ads.readADC_SingleEnded(1)) * multiplier) / 1000;
  /*================= end ADC readings ==================*/


  /* =====================================================
  ===================== Diagnostic =======================
  ======================================================*/
#if DEBUG // if DEBUG is set to 1 (line 60), print out readings
  Serial.println();
  Serial.print("digiPotValue: ");
  Serial.print(digiPotValue);
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
  /* =====================================================*/

}

ISR(TIMER1_OVF_vect)
{
    static int irqcnt = 0; 
    static double anodePotentialArray[10];
    static unsigned long timeArray[10]

    TCNT1 = initTimerCnt;
    readADC();

    anodePotentialArray[irqcnt%10] = anodePotential;
    timeArray[irqcnt%10] = micros();

    irqcnt++;

    if(irqcnt % 10 == 0)
    {

        anodePotentialROC  = getROC(anodePotentialArray, timeArray); 
        adjustDigiPot == true;
    }
}

void readADC()
{
    vol = (ads.readADC_Differential_0_1()) * MULTIPLIER; // Voltage reading
    current = ((vol) / (RESISTANCE)); // ohm's law
    anodePotential = ((ads.readADC_Differential_2_3()) * MULTIPLIER) / 1000;
    cell_vol = ((ads.readADC_SingleEnded(1)) * MULTIPLIER) / 1000;
}

double getROC(double* anodePotentialArray, unsigned long* timeArray) 
{
    
    double anodeMean = 0;
    double timeMean = 0;
    double sumAnodoe = 0;
    double sumTime = 0;
    double sumAnodeTime  0;
    double sumTimeSquared = 0;

    
    for(int i; i< 10; i++)
    {
        sumAnode += anodePotentialArray[i];
        sumTime += (double)timeArray[i];
        sumAnodeTime += anodePotentialArray[i]*(double)timeArray[i];
        sumTimeSqared += (double)(timeArray[i]*timeArray[i]);  
    }
    anodeMean = sumAnode/10;
    timeMean = sumTime/10;
    
    return (sumAnodeTime-(sumTime*anodeMean))/(sumTimeSquared - (sumTime*timeMean));
}
