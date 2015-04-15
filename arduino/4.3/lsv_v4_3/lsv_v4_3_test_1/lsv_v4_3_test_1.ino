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
#define MULTIPLIER 0.125F       // ADS1115  1x gain   +/- 4.096V (16-bit results) 0.125mV
#define RESISTANCE 98.2         // The value of the intrenal resistor
#define TOLERANCE 0.0           // The tolerance of the scan rate.
#define digiPotAdjRate 10       /* The number of ADCreads that are taken before
                                   adjusting the digiPotValue*/                      


Adafruit_ADS1115 ads;
volatile double anodePotential;
volatile double current;
volatile double cell_vol;
volatile double vol;
volatile double anodePotentialROC;       //The rate of change of the anodePotential
volatile bool adjustDigiPot = false;
double scanRate = 1.0;                  //The rate that the program is trying to achieve

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
int lsv_finished = 0;

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
    TCNT1 = initTimerCnt; 
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
    if (offState == true) 
    {
        unsigned long currentMillis = millis();
        if ((currentMillis > offDuration) && (offState == true))
        {
            power1state = HIGH;
            power2state = HIGH;
            offState = false;
            digitalWrite(Power1, power1state);
            digitalWrite(Power2, power2state);
            delay(150); //allow current to stabilize
        }
    }

    if(adjustDigiPot && offState == false && anodePotential < 0)
    {
        if(anodePotentialROC < scanRate-TOLERANCE)
        { 
            digiPotValue ++;
            writeDigiPotValue();
        }
        else if(anodePotentialROC > scanRate+TOLERANCE)
        {
            digiPotValue--;
            writeDigiPotValue();
        }
        adjustDigiPot = false;
    }
    else if(anodePotential >= 0)
    {
        digiPotValue = 0;
        digitalWrite(csPin1, LOW);
        SPI.transfer(0);
        SPI.transfer(digiPotValue);
        digitalWrite(csPin1, HIGH);
    }
}

ISR(TIMER1_OVF_vect)
{
    static int irqcnt = 0; 
    static double anodePotentialArray[digiPotAdjRate];
    static unsigned long timeArray[digiPotAdjRate];

    TCCR1B = 0; //Disabling Timer
    sei();      //Enabling interrupts
    
    readADC();

    anodePotentialArray[irqcnt % digiPotAdjRate] = anodePotential;
    timeArray[irqcnt % digiPotAdjRate] = micros()/1000000;

    irqcnt++;

    if(irqcnt % digiPotAdjRate == 0)
    {

        anodePotentialROC  = getROC(anodePotentialArray, timeArray); 
        adjustDigiPot == true;
    }
    cli();
    TCCR1B |= (1 << CS12) | (1 << CS10);
    TCNT1 = initTimerCnt;
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
    double sumAnode = 0;
    double sumTime = 0;
    double sumAnodeTime = 0;
    double sumTimeSquared = 0;

    
    for(int i; i< digiPotAdjRate; i++)
    {
        sumAnode += anodePotentialArray[i];
        sumTime += (double)timeArray[i];
        sumAnodeTime += anodePotentialArray[i]*(double)timeArray[i];
        sumTimeSquared += (double)(timeArray[i]*timeArray[i]);  
    }
    anodeMean = sumAnode/digiPotAdjRate;
    timeMean = sumTime/digiPotAdjRate;
    
    return (sumAnodeTime-(sumTime*anodeMean))/(sumTimeSquared - (sumTime*timeMean));
}

void writeDigiPotValue()
{
    if (digiPotValue > 255)
    {
        digiPotValue = 255;
        Serial.println(" ERROR: Digipot exceeding max value (255)");
    }

    if(digiPotValue < 0)
    {
        digiPotValue = 0;
        Serial.println(" ERROR: Digipot has fallen bellow the  min value (0)");
    }

    digitalWrite(csPin1, LOW);
    SPI.transfer(0);
    SPI.transfer(digiPotValue);
    digitalWrite(csPin1, HIGH);
}

void printADCreadings(int digits = 10)
{
    Serial.println();
    Serial.print("digiPotValue: ");
    Serial.print(digiPotValue);
    Serial.print(",  current: ");
    Serial.print(current, digits);
    Serial.print(",  annode potential:");
    Serial.print(anodePotential, digits);
    Serial.print(",  Cell vol:");
    Serial.print(cell_vol, digits);
    Serial.print(", Measured scan rate:");
    Serial.print(anodePotentialROC);
}
