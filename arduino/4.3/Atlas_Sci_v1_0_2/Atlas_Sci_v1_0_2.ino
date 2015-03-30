
/*

 Atlas_Sci_v1_0_2.ino

Unconfirmed

 Version: 1.0.2

 Measures PH, electrical conductivity, dissolved oxygen

/**************
 * Configuration:
 * Serial 1: pH
 * Serial 2: EC
 * Serial 3: DO
 **************
 *
 * To-Do:
 * - Implement temperature sensor
 * - parse Atlas Sci probe data
 * - disable continuous readings

 IMplement code to save only numbers incoming from probes:

     //listen for numbers between 0-9, allow decimal points, etc
    if(byteRead>47 && byteRead<58){

      // if there is a decimal point, keep saving data until comma
      /* If mySwitch is true, then populate the num1 variable
          otherwise populate the num2 variable
       if(!mySwitch){
         num1=(num1*10)+(byteRead-48);
       }else{
         num2=(num2*10)+(byteRead-48);
       }




 *
 *

 CHanges:

 - Increased baud rate to 115200
 - Implemented serial.parseFloat instead of using strings
 *
 * Last modified: 3/27/15 - Adam Burns - burns7@illinois.edu
 *
 */


int counter = 0;
int sensorFlag = 1;
float DO_reading = 0;
float pH_reading = 0;
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
boolean EC_stringComplete = false;
boolean pH_stringComplete = false;
boolean DO_stringComplete = false;
char startup = 1;
String junk = "";
char junk2;
#define DEBUG 1 // set to 1 to print debug data to serial monitor
#define pH 1
#define DO 1
#define EC 1

void setup() {                                                               //set up the hardware
  Serial.begin(115200);   // increased baud rate to catch all incoming data
  Serial1.begin(38400); // pH circuit v4.0 needs baud rate = 38400
  Serial2.begin(9600);
  Serial3.begin(9600);                                                      //set baud rate for software serial port_3 to 9600
  inputstring.reserve(5);                                                   //set aside some bytes for receiving data from the PC
  inputstring1.reserve(5);                                                   //set aside some bytes for receiving data from the PC
  inputstring2.reserve(5);                                                   //set aside some bytes for receiving data from the PC
  inputstring3.reserve(5);                                                   //set aside some bytes for receiving data from the PC
  sensorstring1.reserve(30);                                                 //set aside some bytes for receiving data from Atlas Scientific product
  sensorstring2.reserve(30);                                                 //set aside some bytes for receiving data from Atlas Scientific product
  sensorstring3.reserve(30);                                                 //set aside some bytes for receiving data from Atlas Scientific product
  //delay(100); // not sure if this is needed




}
void triggerReadings();
void DO_tx();
void DO_rx();
void EC_tx();
void EC_rx();
void pH_tx();
void pH_rx();


void serialEvent() {                                                         //if the hardware serial port_0 receives a char
  char inchar = (char)Serial.read();                               //get the char we just received
               inputstring += inchar;                             //add it to the inputString
  if (inchar == '\r') {
#if pH
    inputstring1 = inputstring;
    input1_stringcomplete = true;
#endif
    //delay(100); // not sure if this is needed
#if EC
    inputstring2 = inputstring;
    input2_stringcomplete = true;
#endif
    // delay(100); // not sure if this is needed
#if DO
    inputstring3 = inputstring;
    input3_stringcomplete = true;
#endif
    inputstring = "";
  }                //if the incoming character is a <CR>, set the flag
 
}

#if DO
void serialEvent3() {   //if the hardware serial port_3 receives a char
  while (Serial3.available() > 0)
  {
    char inchar = (char)Serial3.read();
    sensorstring3 += inchar;
    // DO_reading = Serial3.parseFloat();

    // ASCII code of comma (,) is -4, asteist (*) is -6
    if ((inchar == '\r') || (inchar == 44) || (inchar = -6)) {
      DO_stringComplete = true;
    }
    else {
      //sensorstring3 += inchar;
    }
  }
}
#endif

#if EC
void serialEvent2() {
  while (Serial2.available() > 0) {
    char inchar = (char)Serial2.read();
          sensorstring2 += inchar;

    if ((inchar == '\r') || (inchar == 44) || (inchar = ',')) {
      EC_stringComplete = true;
      //Serial2.flush();
    }
    else {
      //sensorstring2 += inchar;
    }
  }
}

#endif

#if pH
void serialEvent1() {
  while (Serial1.available() > 0) {
    char inchar = (char)Serial1.read();
 sensorstring1 += inchar;
    if (inchar == '\r') {
      //pH_reading = Serial1.parseFloat();
      pH_stringComplete = true;
      //Serial1.flush();
    }
    else {
     // sensorstring1 += inchar;
    }
  }
}

#endif



void loop() {

  counter++;




  if (startup) {
    Serial1.flush();
Serial2.flush();
Serial3.flush();
sensorstring1 = "";
sensorstring2 = "";
sensorstring3 = "";
    Serial1.print("R");
    Serial1.print('\r');
    delay(2000); //wait 2 seconds and try again
    Serial1.print("R");
    Serial1.print('\r');
    delay(2000); //wait 2 seconds and try again
    Serial2.print("C,0");
    Serial2.print('\r');
    delay(2000); //wait 2 seconds and try again
    Serial3.print("C,0");
    Serial3.print('\r');
    startup = 0;
    Serial.print("     setup finished    ");
  }
  
  if(counter%250==0){

#if DO
DO_tx();
DO_rx();
#endif


#if EC
EC_tx();
EC_rx();
#endif

#if pH
pH_tx();
pH_rx();
#endif

  }
  if (counter % 500 == 0) {
//Serial1.flush();
//Serial2.flush();
//Serial3.flush();
sensorstring1 = "";
sensorstring2 = "";
sensorstring3 = "";
    //triggerReadings();
    counter = 0;
   
  }

}

float getData(String sensorstring) {
  // This function checks the readings for incorrect data

#if DEBUG
  Serial.print(sensorstring);
  Serial.print("  ");
  Serial.println();
#endif
  float reading = sensorstring.toFloat();


}

void triggerReadings() {
  inputstring = "R";
  inputstring += '\r';
#if pH
  inputstring1 = inputstring;
  input1_stringcomplete = true;
#endif

#if EC
  inputstring2 = inputstring;
  input2_stringcomplete = true;
#endif

#if DO
  inputstring3 = inputstring;
  input3_stringcomplete = true;
#endif
  inputstring = "";
}

void DO_tx(){
 if (input3_stringcomplete) {                                                  //if a string from the PC has been received in its entirety
    Serial3.print(inputstring3);                                              //send that string to the Atlas Scientific product
    inputstring3 = "";
    //Serial3.flush();    // unsure
    input3_stringcomplete = false;                                            //reset the flag used to tell if we have received a completed string from the PC
  }
}

void DO_rx(){
if (DO_stringComplete) {
    // if (Serial3.available()>0){        //if a string from the Atlas Scientific product has been received in its entierty
float DO_reading = getData(sensorstring3);  //if a string from the PC has been received in its entirety
#if DEBUG
    Serial.print("      DO parseFloat:  ");
    Serial.println(DO_reading);
#endif

    
    Serial.print("DO: ");
    Serial.print("[");
    Serial.print(sensorstring3);                                            //send that string to to the PC's serial monitor
    Serial.println("]");
    sensorstring3 = "";
    //Serial.flush();    //clear the string:
    DO_stringComplete = false;                                           //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
    // }
  }
}

void EC_tx(){
  if (input2_stringcomplete) {
//Serial2.flush();    //clear the string:
    Serial2.print(inputstring2);                                              //send that string to the Atlas Scientific product
    inputstring2 = "";
    
    input2_stringcomplete = false;                                            //reset the flag used to tell if we have received a completed string from the PC
  }
}

void EC_rx(){
    //if (Serial2.available() && (EC_stringComplete)) {
    if (EC_stringComplete) {    //if a string from the Atlas Scientific product has been received in its entierty
      if (Serial2.available()) {
        float EC_reading = getData(sensorstring2);
        Serial.print(" EC: [");
        Serial.print(sensorstring2);                                            //send that string to to the PC's serial monitor
        Serial.println("]");
        sensorstring2 = "";                                                       //clear the string:
        EC_stringComplete = false;                                           //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
      }
    }
  }
//}

void pH_tx(){
  if (input1_stringcomplete) {                                                  //if a string from the PC has been received in its entirety
     //Serial1.flush();    //clear the string:
    Serial1.print(inputstring1);                                              //send that string to the Atlas Scientific product
    inputstring1 = "";
   
    input1_stringcomplete = false;                                            //reset the flag used to tell if we have received a completed string from the PC
  }
}

void pH_rx(){
  if (pH_stringComplete) {   //if a string from the Atlas Scientific product has been received in its entierty
    if (Serial1.available()) {


      float pH_reading = getData(sensorstring1);
      
      #if DEBUG
      Serial.print("      pH parseFloat:  ");
      Serial.println(pH_reading);
#endif

      Serial.print(" pH: [");

      Serial.print(sensorstring1);                                            //send that string to to the PC's serial monitor
      Serial.print("]");
      Serial.println();
      sensorstring1 = "";                                                       //clear the string:
      pH_stringComplete = false;                                           //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
    }
  }
}





