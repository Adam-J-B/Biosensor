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

#define MULTIPLIER 0.125F       // ADS1115  1x gain   +/- 4.096V (16-bit results) 0.125mV
#define RESISTANCE 98.2         // The value of the intrenal resistor
#define TOLERANCE 0.0           // The tolerance of the scan rate.
#define digiPotAdjRate 10       /* The number of ADCreads that are taken before
                                   adjusting the digiPotValue*/                      

//Arduino Pin Assignments
//#define csPin1 A12
//#define Power1 A10
//#define GND1 A11

Adafruit_ADS1115 ads;

//Ports
const int csPin1 = 7; //Chip select Digital Pin 7 for digital pot #1
const int Power1 = 6; // power control for MCP4161 #1 to D6


//Reading Variables
double anodePotential;
double current;
double cell_vol;
double vol;


float anodePotentialROC;       //The rate of change of the anodePotential
double scanRate = 1.0;                  //The rate that the program is trying to achieve

//State Variables
boolean offState = true;
bool adjustDigiPot = false;
bool wait = false;


unsigned long offDuration = 10000;//300000; // off mode for 30 mins (1,800,000ms)
int numOfDigits = 5;
double anodePotentialArray[digiPotAdjRate];
double timeArray[digiPotAdjRate];

int digiPotValue = 0;
int lsv_finished = 0;
void printADCreadings(int digits);


/*#############################################################################
##############################  Setup Function  ###############################
#############################################################################*/
void setup()
{
    Serial.begin(9600);
	SPI.begin();
    ads.begin();
    pinMode(csPin1, OUTPUT);
	pinMode(Power1, OUTPUT);
    digitalWrite(Power1,HIGH);
    writeDigiPotValue();
    digitalWrite(Power1,LOW);
    
    ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV

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
            Serial.println(" ");
            Serial.println("Done With Offstate");
            offState = false;
            digitalWrite(Power1, HIGH);
            delay(150); //allow current to stabilize
        }
    }
    if(millis()%100 == 0)
    {
        if(wait == false)
        {
            static int irqcnt = 0; 
            
            
            readADC();
            //printADCreadings(10);

            anodePotentialArray[irqcnt % digiPotAdjRate] = anodePotential*1000;
            timeArray[irqcnt % digiPotAdjRate] = (double)millis()/1000.0;
            irqcnt++;
            if(irqcnt % digiPotAdjRate == 9)
            {
                //anodePotentialROC = anodePotential - lastAP;
                //lastAP = anodePotential;
                anodePotentialROC  = getROC();//anodePotentialArray, timeArray, anodePotentialROC); 
                adjustDigiPot = true;
            }
            wait = true;
        }
    }
    else
    {
        wait = false;
    }
    
  
    
    if(adjustDigiPot == true && offState == false && anodePotential < 0)
    {
        printADCreadings(10);
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
        writeDigiPotValue();
    }
}

void readADC()
{
    vol = (ads.readADC_Differential_0_1()) * MULTIPLIER; // Voltage reading
    current = ((vol) / (RESISTANCE)); // ohm's law
    anodePotential = ((ads.readADC_Differential_2_3()) * MULTIPLIER) / 1000;
    cell_vol = ((ads.readADC_SingleEnded(1)) * MULTIPLIER) / 1000;
}

float getROC()//double* anodePotentialArray, unsigned long* timeArray, float& roc) 
{
    
    float anodeMean = 0;
    float timeMean = 0;
    float sumAnode = 0;
    float sumTime = 0;
    float sumAnodeTime = 0;
    float sumTimeSquared = 0;
    float ret; 
    for(int i = 0; i< digiPotAdjRate; i++)
    {
        sumAnode += (float)anodePotentialArray[i];
        sumTime += (float)timeArray[i];
        sumAnodeTime += ((float)anodePotentialArray[i])*((float)timeArray[i]);
        sumTimeSquared += ((float)(timeArray[i])*((float)timeArray[i]));  
    }
    anodeMean = sumAnode/digiPotAdjRate;
    timeMean = sumTime/digiPotAdjRate;
    ret = (sumAnodeTime-(sumTime*anodeMean))/((sumTimeSquared)-(sumTime*timeMean));
    if(ret != ret)
    {
        Serial.print("Sum Anode: ");
        Serial.println(sumAnode, 5);
        Serial.print("Sum Time: ");
        Serial.println(sumTime, 5);
        Serial.print("Sum Anode*Time: ");
        Serial.println(sumAnodeTime, 5);
        Serial.print("Sum Time Squared: ");
        Serial.println(sumTimeSquared, 5);
        Serial.print("Anode Mean: ");
        Serial.println(anodeMean, 5);
        Serial.print("Time Mean: ");
        Serial.println(timeMean, 5);
        
        for(int i = 0; i < digiPotAdjRate; i++)
        {
            Serial.print("Time: ");
            Serial.print(timeArray[i], 10);
            Serial.print(", Anode Potential: ");
            Serial.println(anodePotentialArray[i]);
        }
    }
    return ret;
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
    SPI.transfer(0);
    digitalWrite(csPin1, HIGH);
}

void printADCreadings(int digits)
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
    Serial.println(anodePotentialROC, digits);
}
