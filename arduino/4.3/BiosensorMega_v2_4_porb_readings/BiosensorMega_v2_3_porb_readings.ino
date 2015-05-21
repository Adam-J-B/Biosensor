#include <DallasTemperature.h>

#include <DallasTemperature.h>
#include <OneWire.h>
OneWire oneWire(7); // Temperature probe data line (yellow wire) to D7
DallasTemperature sensors(&oneWire);


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

//int csPin1 = 7; //Chip select Digital Pin 7 for digital pot #1
int csPin2 = 3; //Chip select D3 for digital pot #2

int digiPotValue = 0;
int lsv_finished = 0;


int counter = 0;
int sensorFlag = 1;
int startup = 1;


#define csPin1 A12
#define SCKpin 52
#define SDIpin 51
#define Power1 A10
#define GND1 A11

#define DEBUG 1 // set to 1 to print debug data to serial monitor
#define pH 1
#define DO 0
#define EC 0
#define Temperature 0

void readADC();
void printADCreadings(int digits);

void setup() {                                                               //set up the hardware
  Serial.begin(115200);   // increased baud rate to catch all incoming data
  Serial1.begin(38400);   // pH circuit v4.0 needs baud rate = 38400
  Serial2.begin(9600);
  Serial3.begin(9600);                                                      //set baud rate for software serial port_3 to 9600
  SPI.begin(); //Init SPI

  // ==================== ADC Initilization =========================
  ads.begin();
  ads.setGain(GAIN_ONE); // 1x gain,  +/- 4.096V, 1 bit = 0.125mV


  pinMode(csPin1, OUTPUT);
  pinMode(csPin2, OUTPUT);

  pinMode(Power1, OUTPUT);
  pinMode(Power2, OUTPUT);
  pinMode(GND1, OUTPUT);
  digitalWrite(Power1, HIGH);
  digitalWrite(Power2, LOW);
  digitalWrite(GND1, LOW);
  delay(200);

  // Temp Probe initilization
  sensors.begin(); // IC Default 9 bit (change to 12 if issues exist)


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
  digitalWrite(Power1, LOW);  //set aside some bytes for receiving data from Atlas Scientific product
  //delay(100); // not sure if this is needed

    //Setting Up the sensors
    Serial1.flush();
    Serial2.flush();
    Serial3.flush();
    Serial2.print("C,0\r");
    Serial3.print("C,0\r");
    delay(3000);
    while(Serial1.available()){Serial1.read();}
    while(Serial2.available()){Serial2.read();}
    while(Serial3.available()){Serial3.read();}

}

void serialEvent1() {
    char serial1Char[5];
    int numChars = readSerial1(serial1Char);
    if(numChars < 1)
    {
        //Serial.print("Error Not OK"); 
    } 
    else{
        Serial.print("PH: ");
        printCharArray(serial1Char,numChars);
    } 
}
void serialEvent2() {
    char serial2Char[5];
    int numChars = readSerial2(serial2Char);
    if(numChars < 1)
    {
        //Serial.print("Error Not OK"); 
    } 
    else{
        Serial.print("EC: "); 
        printCharArray(serial2Char,numChars);
    } 
}
void serialEvent3() {
    char serial3Char[5];
    int numChars = readSerial3(serial3Char);
    if(numChars < 1)
    {
        //Serial.print("Error Not OK"); 
    } 
    else{
        Serial.print("DO: "); 
        printCharArray(serial3Char,numChars);
        
        
        Serial.print("Temp: ");
        Serial.print(sensors.getTempCByIndex(0));
        Serial.println();
    }           
}
boolean  wait  = false;
void loop()
{        
    if(millis()%5000 == 0)
    {
        
        if(wait == false){
            Serial1.print("r\r");
            Serial2.print("R\r");
            Serial3.print("R\r");
            sensors.requestTemperatures();
            wait = true;
        }
    }
    else{
        wait = false;
    }
}

double readSerial1(char* serial1Char)
{
    int byteRead;
    int numChars = 0;
    while(true){   
        while (Serial1.available() > 0) {
            byteRead = Serial1.read();
            if(byteRead == 13){
                while(Serial1.available()){Serial1.read();}
                return numChars;
            }
            else if((byteRead < 48 || byteRead > 57) && byteRead != 46){
                return -1;
            }
            else{
                serial1Char[numChars] = (char)byteRead;
                numChars++;
            }
        }  
    }
}

int readSerial2(char* serial2Char)
{
    int byteRead;
    char resp[3];
    int respCNT = 0;
    int msgSect = 1;
    int numChars = 0;
    while(true){   
        while (Serial2.available() > 0) {
           byteRead = Serial2.read();
           if(msgSect == 0){
                 if(byteRead == 13 || respCNT == 3){
                     if(resp[0] != '*' && resp[1] != 'O' && resp[2] != 'K')
                     {
                         
                         while(Serial2.available()){Serial2.read();}
                         return -1;
                     }
                     if(numChars > 0){
                         while(Serial2.available()){Serial2.read();}
                         return numChars;
                     }
                     else{                     
                         msgSect = 1;
                         respCNT = 0;
                     }
                 }
                 else{
                     resp[respCNT] = (char)byteRead;
                     respCNT++;
                 }
             }
             else if(msgSect == 1){
                 if(byteRead == 44)
                 {
                     msgSect=2;
                 }
                 else{
                     serial2Char[numChars] = (char)byteRead;
                     numChars++;
                 }
             }
             else
             {
                 if(byteRead == 13)
                 {
                     msgSect=0;
                 }
             }
        }  
    }
}
int readSerial3(char* serial3Char)
{
    int byteRead;
    int numChars = 0;
    while(true){   
        while (Serial3.available() > 0) {
            byteRead = Serial3.read();
            if(byteRead == 13){
                while(Serial3.available()){Serial3.read();}
                return numChars;
            }
            else if((byteRead < 48 || byteRead > 57) && byteRead != 46){
                return -1;
            }
            else{
                serial3Char[numChars] = (char)byteRead;
                numChars++;
            }
        }  
    }   
} 

void printCharArray(char * array, int numChars)
{
    for(int i = 0; i< numChars; i++)
    {
        Serial.print(array[i]);
    }
    Serial.print(" ");
}


