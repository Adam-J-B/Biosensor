/*
AtlasSciProbes_temp.ino
 
 Version: 1.1.2
 
 ******** Currently not working *******
 
 Measures PH, electrical conductivity, dissolved oxygen, and temperature
 
**************
Configuration:
  Serial 1: pH
  Serial 2: EC
  Serial 3: DO
**************
 

 Last modified: 1/29/15 - Adam Burns - burns7@illinois.edu
 
 Changes:
 V1.1:
 - Comment cleanup
 
 
 */


//  ===================== Atlas Sci =======================


char computerdata[20];               //A 20 byte character array to hold incoming data from a pc/mac/other 
char sensordata[30];                 //A 30 byte character array to hold incoming data from the sensors
byte computer_bytes_received=0;            
byte sensor_bytes_received=0;  
String sensorstring="";

// ============== end Atlas Sci ==========================


// =================== temperature probe ==================
#include <DallasTemperature.h>
#include <OneWire.h>

#define ONE_WIRE_BUS 3 // temperature sensor to D3
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);
//=============== end temperature probe ===================
byte arduino_only=1;               //set 1 to operate w/ Arduino only and w/o the serial monitor commands
byte startup=0;  
#define DEBUG 0 // set to 1 to print debug data to serial monitor


void setup(void)
{

/**************
Configuration:
  Serial 1: pH
  Serial 2: EC
  Serial 3: DO
**************/
 
  Serial.begin(38400);              //Set the hardware serial port to 38400
  Serial2.begin(38400);
  Serial3.begin(38400);
  sensorstring.reserve(30);

    // Temp Probe initilization
  sensors.begin(); // IC Default 9 bit (change to 12 if issues exist)
}


void Arduino_Control(){

  if(startup==0){        //if the Arduino just booted up, we need to set some things up first.   
#if DEBUG
/**************
Configuration:
  Serial 1: pH
  Serial 2: EC
  Serial 3: DO
**************/
 
Serial.println();
    Serial.print("pH Cercuit Version:");
    
#endif
/*
      delay(100);
      altSerial.print(i);
      altSerial.print(":c,");   //take the pH Circuit out of continues mode. 
      altSerial.print(0);
      altSerial.print("\r");
      delay(50);                 //on start up sometimes the first command is missed. 
      altSerial.print(i);
      altSerial.print(":c");   
      altSerial.print(0);
      altSerial.print("\r");  //so, let’s send it twice.
      delay(50);                 //a short delay after the pH Circuit was taken out of continues mode is used to make sure we don’t over load it with commands.
    
    startup=1;                 //startup is completed, let's not do this again during normal operation. 
    
    8*/
  }
}

void serialEvent(){               //This interrupt will trigger when data from the serial monitor(pc) is received   
  computer_bytes_received=Serial.readBytesUntil(13,computerdata,20); //Read from serial monitor(pc) until a <CR> & count # of recieved characters   
  computerdata[computer_bytes_received]=0; //Append a 0 to array after last recieved character
}    



void loop(void)
{ 
/**************
Configuration:
  Serial 1: pH
  Serial 2: EC
  Serial 3: DO
**************/
 
  // *********************************************************
  // ******** does this cause an issue w/ EZO sensors? *******
  // *********************************************************
  if((arduino_only==1)&&(startup==0)){
    Arduino_Control();
  }
  // *********************************************************
  // *********************************************************


  // get temperature reading
  sensors.requestTemperatures();
  Serial.print("Temp: ");
  Serial.print(sensors.getTempCByIndex(0)); 
  Serial.print(" C, ");
  delay(100);
  
  
  //================= atlas sci code ========================

  // Channel Y0
  Serial.print("DO: ");
  
  delay(500);
  /*
  digitalWrite(s0, LOW);                       //S0 and S1 control what channel opens 
   digitalWrite(s1, LOW);
   delay(500);
   altSerial.print("0:");                         //Send the command from the computer to the Atlas Scientific device using the softserial port 
   altSerial.print("\r");
   delay(500);
   
  altSerial.print("0:r");                         //Send the command from the computer to the Atlas Scientific device using the softserial port 
  altSerial.print("\r"); 
  delay(1500);  //After we send the command we send a carriage return <CR> 
  */
  getAtlasReading();

  delay(1000);
  Serial.print(", ");

  // Channel Y1
  Serial.print("EC: ");
  //open_channel(1);

  delay(500);
 
  getAtlasReading();
  /*
  delay(500);
   digitalWrite(s0, HIGH);
   digitalWrite(s1, LOW);
   delay(500);
   altSerial.print("1:");                         //Send the command from the computer to the Atlas Scientific device using the softserial port 
   altSerial.print("\r");
   */
  delay(1500);
 
  delay(1000);
  Serial.print(", ");

  // Channel Y2
  Serial.print("PH: ");
  /*
  digitalWrite(s0, LOW);
   digitalWrite(s1, HIGH);
   delay(500);
   digitalWrite(s0, LOW);
   digitalWrite(s1, HIGH);
   delay(500);
   altSerial.print("2:");                         //Send the command from the computer to the Atlas Scientific device using the softserial port 
   altSerial.print("\r");
   delay(500);
   
 
  altSerial.print("2:r");                         //Send the command from the computer to the Atlas Scientific device using the softserial port 
  altSerial.print("\r");  
  delay(1500);  //After we send the command we send a carriage return <CR> 
  getAtlasReading();
  delay(1000);
  Serial.print(";");
*/
  Serial.println(); //new line
  //delay(500);
  //open_channel(0);
  delay(1000);
}



void getAtlasReading(){
  
  /**************
Configuration:
  Serial 1: pH
  Serial 2: EC
  Serial 3: DO
**************/
 
  if(Serial3.available() > 0){                   //If data has been transmitted from an Atlas Scientific device
    sensor_bytes_received=Serial3.readBytesUntil(13,sensordata,30); ///Read from serial monitor(pc) until a <CR> & count # of recieved characters 
    sensordata[sensor_bytes_received]=0;           //Append a 0 to array after last recieved character
    Serial.print(sensordata);      // transmit data from the Atlas Scientific probe to serial monitor   
    sensor_bytes_received=0;
    //int length = 0; 
    //sensordata[]=0;
    Serial3.flush();
    Serial3.flush();
  }
  else if(Serial3.available()<=0){

    Serial.print("Atlas Sci Probe error");
  }

}












