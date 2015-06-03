

#include <Adafruit_ADS1015.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <SPI.h>
#include <Wire.h>



//Pin declarations
#define csPin1 A12
//#define csPin2
#define Power1 A10
//#define Power2
#define GND1 A11

//Constant Variables
#define MULTIPLIER 0.125F       // ADS1115  1x gain   +/- 4.096V (16-bit results) 0.125mV
#define RESISTANCE 98.2         // The value of the intrenal resistor
#define TOLERANCE 0.0           // The tolerance of the scan rate.
#define probeRate 5000 //60000         // Number of millis seconds between consecutive probe reaadings
#define offDuration 10000           // The amount of time the program will wait affter setup to begin the experiment 
#define numOfDigits 5           // The nuber of didgits a the ADC reading will print 
#define digiPotAdjRate 10       /* The number of ADCreads that are taken before
                                   adjusting the digiPotValue*/
boolean STARTUPTIME=true;

//Global Varibles

Adafruit_ADS1115 ads;
OneWire oneWire(7); // Temperature probe data line (yellow wire) to D7
DallasTemperature sensors(&oneWire);

double vol  = 0;
double current=0; 
double anodePotential=-5;
double cell_vol=0;

int digiPotValue = 0; // Value sent to digipot

int power1state = LOW;
int power2state = LOW;
boolean offState = true;

int cnt = 0;
double target_value = 0;
int lsv_finished = 0;

boolean probeWait = false;

char serial1Char[5];
char serial2Char[5];
char serial3Char[5];
int numChars1 = 0;
int numChars2 = 0;
int numChars3 = 0;

void readADC();
void printADCreadings(int digits);
void writeDigiPotValue();
int readSerial1(char* serial1Char);
int readSerial2(char* serial1Char);
int readSerial3(char* serial1Char);
void printCharArray(char * array, int numChars);

/*#############################################################################
##############################  Setup Function  ###############################
#############################################################################*/
void setup()
{
  Serial.begin(115200);   // increased baud rate to catch all incoming data
  Serial1.begin(38400);   // pH circuit v4.0 needs baud rate = 38400
  Serial2.begin(9600);
  Serial3.begin(9600);  
  SPI.begin();
  
// ==================== ADC Initilization =========================
  ads.begin();
  ads.setGain(GAIN_ONE); // 1x gain,  +/- 4.096V, 1 bit = 0.125mV
  
    pinMode(csPin1, OUTPUT);
    //pinMode(csPin2, OUTPUT);
    pinMode(Power1, OUTPUT);
    //pinMode(Power2, OUTPUT)
    pinMode(GND1, OUTPUT);
    digitalWrite(Power1, LOW);
    delay(200);
    
    // Temp Probe initilization
    sensors.begin(); // IC Default 9 bit (change to 12 if issues exist)
    
    // Reset digipot to 0
    digitalWrite(Power1, HIGH);
    digitalWrite(GND1,LOW);
    delay(5000);
    //digitalWrite(csPin1, HIGH);
    delay(200);
    
    // Reset digipot to 0
    writeDigiPotValue();
    delay(200);
    // Setting digipot to 0 again in case 1st attempt failed'
    writeDigiPotValue();
    delay(200);
    
    digiPotValue = 1;
    writeDigiPotValue();
    delay(200);
    digiPotValue = 2;
    writeDigiPotValue();
    delay(200);
//    digiPotValue = 0;
//    writeDigiPotValue();
//    delay(200);    
    
    
    digitalWrite(Power1, LOW);
    delay(200);
    
    //Setting Up the sensors
    Serial1.flush();
    Serial2.flush();
    Serial3.flush();
    Serial1.print("r\r");
    Serial2.print("C,0\r");
    Serial3.print("C,0\r");
    delay(2000);
    while(Serial1.available()){Serial1.read();}
    while(Serial2.available()){Serial2.read();}
    while(Serial3.available()){Serial3.read();}    
}
/*######################### end setup function ##############################*/

/*################ Serial Events to handle probe readings ##################*/
void serialEvent1() {
    numChars1 = readSerial1(serial1Char);
    if(numChars1 < 1)
    {
        Serial.print("Error Not 1 OK"); 
    } 
}
void serialEvent2() {

    numChars2 = readSerial2(serial2Char);
    if(numChars2 < 1)
    {
        Serial.print("Error Not 2 OK"); 
    } 
}
void serialEvent3() {
    
    numChars3 = readSerial3(serial3Char);
    if(numChars3 < 1)
    {
        Serial.print("Error Not 3 OK"); 
    }          
}

/*#############################################################################
############################ Loop Function ####################################
#############################################################################*/
void loop()
{
    if(anodePotential < -.1){
        if(millis()%1000 == 0){      
            if(offState==true){
                if(millis()%500)
                {
                     printADCreadings(4);
                }
                if(STARTUPTIME){
                    digitalWrite(Power1,HIGH);
                    delay(300);
                    writeDigiPotValue();
                    delay(300);
                    digitalWrite(Power1,LOW);
                    STARTUPTIME=false;
                    }
                unsigned long currentMillis = millis();
                if((currentMillis>offDuration) && (offState==true))
                {
                    power1state=HIGH;
                    offState=false;
                    digitalWrite(Power1,power1state);
                    Serial.println("Experiment Begin");
                }
            }
            readADC();
            if (cnt==0){
                target_value=anodePotential;
            }
            if(offState==false){
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
                        if (cnt % 1 ==0)
                        {
                            if (anodePotential < target_value)
                            {
                                digiPotValue++;
                                writeDigiPotValue();
                                //printADCreadings(4);
                            }
                        }
                    }
                } 
                else {
                    digiPotValue = 0;
                    writeDigiPotValue();
                }
            }
//            if(millis()%probeRate == 0)
//            {
//                if(probeWait == false){
//                    //Serial.print("Probe Read");
//                    Serial1.print("r\r");
//                    Serial2.print("R\r");
//                    Serial3.print("R\r");
//                    sensors.requestTemperatures();
//                    probeWait = true;
//                }
//            }
//            else{
//                probeWait = false;
//            }
              printADCreadings(4);
        }
        
    }
    else
    {
        digiPotValue =0; 
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

int readSerial1(char* serial1Char)
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
    int msgSect = 0;
    char resp[3];
    int respCNT = 0;
    while(true){   
        while (Serial3.available() > 0) {
            
            byteRead = Serial3.read();
//            Serial.print((char)byteRead);
//            Serial.print("{");
//            Serial.print(byteRead);
//            Serial.print("}");
            if(msgSect == 0){
                if(byteRead == 13){
                    msgSect = 1;
                    //return numChars;
                }
                else if((byteRead < 48 || byteRead > 57) && byteRead != 46){
                    while(Serial3.available()){Serial3.read();}
                    return -1;
                }
                else{
                    serial3Char[numChars] = (char)byteRead;
                    numChars++;
                }
            }
            else{
                
                if(byteRead == 13 || respCNT == 3){
                     if(resp[0] != '*' && resp[1] != 'O' && resp[2] != 'K')
                     {
                         while(Serial3.available()){Serial3.read();}
                         return -1;
                     }
                     if(numChars > 0){
                         while(Serial3.available()){Serial3.read();}
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
        }  
    }   
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
    //Serial.print(", Measured scan rate:");
    //Serial.print(anodePotentialROC);
}
void printCharArray(char * array, int numChars)
{
    Serial.print(" ");
    for(int i = 0; i< numChars; i++)
    {
        Serial.print(array[i]);
    }
}
