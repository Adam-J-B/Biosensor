
/*

Atlas_Sci_v1_0.ino

 
 Version: 1.0.1
 
 ******** Currently not working *******
 
 Measures PH, electrical conductivity, dissolved oxygen
 
  /**************
   * Configuration:
   * Serial 1: pH
   * Serial 2: EC
   * Serial 3: DO
   **************
   
Last modified: 3/24/15 - Adam Burns - burns7@illinois.edu



*/


// DO probe


String inputstring = "";                                                       //a string to hold incoming data from the PC
String inputstring1 = "";   
String inputstring2 = "";  
String inputstring3 = "";   
String sensorstring1 = "";                                                      //a string to hold the data from the Atlas Scientific product
String sensorstring2 = "";     
String sensorstring3 = "";     
boolean input1_stringcomplete = false;                                          //have we received all the data from the PC
boolean input2_stringcomplete = false;
boolean input3_stringcomplete = false;
boolean sensor_stringcomplete = false;                                         //have we received all the data from the Atlas Scientific product
boolean EC_stringComplete=false;
boolean pH_stringComplete=false;
boolean DO_stringComplete=false;
#define DEBUG 1 // set to 1 to print debug data to serial monitor
#define pH 1
#define DO 1
#define EC 1

void setup(){                                                                //set up the hardware
  Serial.begin(9600);   //set baud rate for the hardware serial port_0 to 9600
  Serial1.begin(38400);
  Serial2.begin(9600);
  Serial3.begin(9600);                                                      //set baud rate for software serial port_3 to 9600
  inputstring.reserve(5);                                                   //set aside some bytes for receiving data from the PC
  inputstring1.reserve(5);                                                   //set aside some bytes for receiving data from the PC
  inputstring2.reserve(5);                                                   //set aside some bytes for receiving data from the PC
  inputstring3.reserve(5);                                                   //set aside some bytes for receiving data from the PC
  sensorstring1.reserve(30);                                                 //set aside some bytes for receiving data from Atlas Scientific product
  sensorstring2.reserve(30);                                                 //set aside some bytes for receiving data from Atlas Scientific product
  sensorstring3.reserve(30);                                                 //set aside some bytes for receiving data from Atlas Scientific product
}



void serialEvent() {                                                         //if the hardware serial port_0 receives a char              
  char inchar = (char)Serial.read();                               //get the char we just received
  inputstring += inchar;                                           //add it to the inputString
  if(inchar == '\r') {
#if pH
    inputstring1=inputstring;
    input1_stringcomplete=true;
#endif

#if EC
    inputstring2=inputstring;
    input2_stringcomplete = true;
#endif

#if DO
    inputstring3=inputstring;
    input3_stringcomplete = true;
#endif
inputstring="";
  }                //if the incoming character is a <CR>, set the flag
}  

#if DO
void serialEvent3(){    //if the hardware serial port_3 receives a char 
  while(Serial3.available())
  {
    char inchar = (char)Serial3.read();   
    sensorstring3 += inchar;   
    if(inchar == '\r'){    
      DO_stringComplete = true;
    }
  }
}
#endif

#if EC
void serialEvent2(){
  while(Serial2.available()){
    char inchar=(char)Serial2.read();
    sensorstring2 += inchar;

    if((inchar == '\r')){
      EC_stringComplete=true;
    }
  }
}
#endif

#if pH
void serialEvent1(){
  while(Serial1.available()>0){
    char inchar=(char)Serial1.read();
    sensorstring1 += inchar;
    if(inchar == '\r'){
      pH_stringComplete=true;
    }
  }
}
#endif



void loop(){                                                                   //here we go....


#if DO
  if (input3_stringcomplete){                                                   //if a string from the PC has been received in its entirety 
    Serial3.print(inputstring3);                                              //send that string to the Atlas Scientific product
    inputstring3 = "";                                                        //clear the string:
    input3_stringcomplete = false;                                            //reset the flag used to tell if we have received a completed string from the PC
  }
  if(DO_stringComplete){
   // if (Serial3.available()>0){        //if a string from the Atlas Scientific product has been received in its entierty 
      Serial.print("DO: ");
      Serial.print("[");
      Serial.print(sensorstring3);                                            //send that string to to the PC's serial monitor
      Serial.println("]");
      sensorstring3 = "";                                                       //clear the string:
      DO_stringComplete = false;                                           //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
   // }
  }
#endif


#if EC

  if (input2_stringcomplete){                                                   //if a string from the PC has been received in its entirety 
    Serial2.print(inputstring2);                                              //send that string to the Atlas Scientific product
    inputstring2 = "";                                                        //clear the string:
    input2_stringcomplete = false;                                            //reset the flag used to tell if we have received a completed string from the PC
  }

  while(Serial2.available()&&(EC_stringComplete)){
    if (EC_stringComplete){     //if a string from the Atlas Scientific product has been received in its entierty 
      if(Serial2.available()){
        Serial.print(" EC: [");
        Serial.print(sensorstring2);                                            //send that string to to the PC's serial monitor
        Serial.println("]");
        sensorstring2 = "";                                                       //clear the string:
        EC_stringComplete = false;                                           //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
      }
    }
  }

#endif

#if pH

  if (input1_stringcomplete){                                                   //if a string from the PC has been received in its entirety 
    Serial1.print(inputstring1);                                              //send that string to the Atlas Scientific product
    inputstring1 = "";                                                        //clear the string:
    input1_stringcomplete = false;                                            //reset the flag used to tell if we have received a completed string from the PC
  }

  if (pH_stringComplete){    //if a string from the Atlas Scientific product has been received in its entierty 
    if(Serial1.available()){
      Serial.print(" pH: [");

      Serial.print(sensorstring1);                                            //send that string to to the PC's serial monitor
      Serial.print("]");
      Serial.println();
      sensorstring1 = "";                                                       //clear the string:
      pH_stringComplete = false;                                           //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
    }
  }
#endif
}








