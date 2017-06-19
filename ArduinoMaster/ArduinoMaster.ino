
#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

/////////////////////////Controller Name//////////////////////////////////

char controllername[5] = {'0', '0', '2', '6'} ; // To be changed with every controller.

///////////////////////////Serial Declaration//////////////////////////////
int bluetoothTx = 2;  // TX-O pin of bluetooth mate
int bluetoothRx = 3;  // RX-I pin of bluetooth mate
SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

///////////////////Inputs//////////////////////////

#define VERT  A6 // analog SHOULD BE A4
#define HORIZ  A7 // analog
#define AccelX  A1
#define AccelY  A2
#define AccelZ  A3


#define Deadman  4
#define Estop  5
#define Zaxis  6
#define Sync  7


int indicatorled = 13;
//////////////////Global Variables/////////////////
int ShapeStateDeadMan = 0;  //for smaller objects like Estop, and interlocks
int ShapeStatePin = 0;
int ShapeStateCarrier = 0;

long hMaxValueCalib = 824; long hMinValueCalib = 200; long hTrim = 0;//put initial calibration values inside expected range of 0-1024, with centers at 0
long vMaxValueCalib = 824; long vMinValueCalib = 200; long vTrim = 0;


int lastSendSum = 0;
int ButtonState = 0;

int battVolts;   // made global for wider avaliblity throughout a sketch if needed, example a low voltage alarm, etc
int DeadmanState = 0;
int SyncState = 0 ;
int ZaxisState = 0;
int EstopState = 0;

int previousbtc = 0; //Previous bluetooth command

long btTimeLast = 10000;

int btTryCount = 999; //This increases when nothing is sent to the contoller and resets when a command is recieved. Used to test connection.
int CurrentScreenState = 0;
int btc;
int loopcount = 0; int calibCount = 0;


bool debug = false;
void setup() {

  Wire.begin();
  //Wire.beginTransmission(1);
  if (debug) Serial.begin(9600);
  bluetooth.begin(9600);
  pinMode(Sync, INPUT);
  pinMode(Deadman, INPUT);
  pinMode(Zaxis, INPUT);
  pinMode(Estop, INPUT);
  pinMode(indicatorled, OUTPUT);
   pinMode(AccelX, INPUT);
    pinMode(AccelY, INPUT);
   pinMode(AccelZ, INPUT);
   
    delay (3000);
  vTrim = 512 - analogRead(VERT) ;
  hTrim = 512 - analogRead(HORIZ) ;
  if(vTrim > 50) vTrim = 50;
  if(vTrim < -50) vTrim = -50;
    if(hTrim > 50) hTrim = 50;
  if(hTrim < -50) hTrim = -50;

  //  CurrentScreenState = bluetooth.read();

}

void loop() {
  loopcount ++;

  if (bluetooth.available() >= 1 )
  {
    // read the incoming byte:
    btc = bluetooth.read();
   // bluetooth.write(btc);

    if (btc != previousbtc && btc > 47 && btc < 100 ) //if the character has changed store it to check for doubles on next loop
    {
      previousbtc = btc;
    }
    else if (btc == previousbtc && btc < 100 && btc > 47) //if the same character is read twice, and it is between 2 and 100
    {

      if (btc == 69 || btc == 70)
      {
        if (ShapeStateDeadMan != btc) {         //for smaller objects like Estop, and interlocks
          if (debug)Serial.print("DEADMAN: ");
          if (debug)Serial.println(btc);
          Wire.beginTransmission(1);
          Wire.write(btc);
          Wire.endTransmission();
          ShapeStateDeadMan = btc;
        }

      }
      else if ( (btc < 69 && btc >  64))
      {
        if (ShapeStatePin != btc) {
          if (debug)Serial.print("    PIN: ");
          if (debug)Serial.println(btc);
          Wire.beginTransmission(1);
          Wire.write(btc);
          Wire.endTransmission();
          ShapeStatePin = btc;
        }
        //for smaller objects like Estop, and interlocks
      }
      else if ( (btc > 49 && btc < 80 ) && (btc < 57 || btc > 65))
      {
        //if (debug)Serial.println(btc);
        if (CurrentScreenState != btc)
        {
          if (debug)Serial.println(btc);
          CurrentScreenState = btc;
          if (debug)Serial.print("CURRENT SCREEN: ");
          if (debug)Serial.println(CurrentScreenState);
          

       
        }
                   Wire.beginTransmission(1);
                    Wire.write(CurrentScreenState);
                     Wire.endTransmission();
                     
        if (debug)Serial.print("BYTE READ: ");
        if (debug) Serial.print(btc, DEC);
        if (debug) Serial.write(" ");
        if (debug) Serial.write(btc);
        if (debug) Serial.println();
            
      }

      previousbtc = btc;
btTimeLast = millis();
    }
    btTryCount = 0;

  }
  else
  {
    if(millis() - btTimeLast > 10000 & CurrentScreenState != 72)
    {
      if (debug)Serial.print("btTimeLast:");
      if (debug)Serial.println(btTimeLast);
      Wire.beginTransmission(1);
      Wire.write(72);
      Wire.endTransmission();
      btTimeLast = millis();
      CurrentScreenState = 72;

    }
  }

  SendJSData();

}


void SendJSData()
{

  long vertical, horizontal ;   // read all values from the joystick
  if(debug) Serial.print(analogRead(AccelZ));
  vertical = analogRead(VERT) ; // will be 0-1023
  horizontal = analogRead(HORIZ) ; // will be 0-1023

  if (vertical >= 512 )
  {
  //  vertical = (512L * 512L) + ((vertical - 512L)*(vertical - 512L)/512L);
  }
  else
  {
    
  }

   if (horizontal >= 512 )
  {
    horizontal = 512L + ((horizontal - 512L)*(horizontal - 512L)/512L);
  }
  else
  {
   
  }

  //Adjust min & max for in line calibration. Values offset by one to prevent overcorrection in single loop
  if (vertical < vMinValueCalib ) vMinValueCalib = vertical + 1;
  if (vertical > vMaxValueCalib) vMaxValueCalib = vertical - 1;
  if (horizontal < hMinValueCalib) hMinValueCalib = horizontal + 1;
  if (horizontal > hMaxValueCalib) hMaxValueCalib = horizontal - 1;

  int deadState;
  deadState = digitalRead(Deadman);

  if (loopcount % 40 == 35 && debug) {
    Serial.print("VERT: ");
    Serial.print(vertical  );
    Serial.print("    ");
    Serial.print(vMinValueCalib);
    Serial.print("    ");
    Serial.print(vMaxValueCalib);
    Serial.print("    ");
    Serial.print(vTrim);

    Serial.print("HORIZ: ");
    Serial.print( horizontal );
    Serial.print("    ");
    Serial.print(hMinValueCalib );
    Serial.print("    ");
    Serial.print(hMaxValueCalib);
    Serial.print("    ");
    Serial.print(hTrim);
    Serial.println();
  }


  if (vertical + vTrim  >= 512L)
  {
    if (calibCount == 2 && deadState == 1 && vTrim > -50l) vTrim = vTrim - 1; //highly damped calibration for center point of joystick
   // vertical = (512L  + ((vertical + vTrim - 512L )*(vertical + vTrim - 512L )/512L ) * (1024L/ vMaxValueCalib));
     vertical =(512L - (vertical+ vTrim)) * (512L - (vertical+ vTrim));
    vertical = map ( vertical,  0L, (511L-(vMaxValueCalib + vTrim)) * (511L-(vMaxValueCalib + vTrim)), 150L, 255L)  ; // will result in 14-255
  }
  else
  {
    if (calibCount == 2 && deadState == 1 && vTrim < 50l) vTrim = vTrim + 1;
    
    vertical =(511L - (vertical+ vTrim)) * (511L - (vertical+ vTrim));
    vertical = map (vertical , (511L-(vMinValueCalib + vTrim)) * (511L-(vMinValueCalib + vTrim)), 0L, 40L, 150L)  ; // will result in 14-255
  }

  if (horizontal + hTrim  >= 512L)
  {
    if (calibCount == 2 && deadState == 1 && hTrim > -50l) hTrim = hTrim - 1;
     horizontal =(512L - (horizontal+ hTrim)) * (512L - (horizontal+ hTrim));
    horizontal = map (horizontal ,  0L, (511L -(hMaxValueCalib + hTrim))*(511L -(hMaxValueCalib + hTrim)), 150L, 255L)  ; // will result in 14-255
  }
  else
  {
    if (calibCount == 2 && deadState == 1 && hTrim < 50l) hTrim = hTrim + 1;
      horizontal = (511L - (horizontal+ hTrim)) * (511L - (horizontal+ hTrim));
    horizontal = map (horizontal , (511L-(hMinValueCalib + hTrim))* (511L-(hMinValueCalib + hTrim)), 0L,  40L, 150L)  ; // will result in 14-255
  }
     

 if(calibCount == 2) calibCount = 0; //calibCount is used for the damping for center calibration

 if(horizontal > 250) horizontal = 250;
 if(horizontal < 50) horizontal = 50;
 if(vertical > 250) vertical = 250;
 if(vertical < 50) vertical = 50;




  ///////////////////////////////////////Make Packet/////////////////////
  //////////////////////////Packet will consist of 4 bytes [vertical, horizontal, ButtonState, Screen] with one checkbit included in
  //CurrentScreenState = 72;


  ButtonState = deadState  * 4 +
                digitalRead(Estop) * 8 +
                digitalRead(Zaxis) * 16 +
                digitalRead(Sync) * 32;



  byte ScreenChar = (byte)CurrentScreenState ;
  byte VertChar = (byte)vertical;
  byte HorizChar = (byte)horizontal;

  int checkval =  VertChar + HorizChar + ButtonState + ScreenChar;

  int  ParityBit =  checkval % 2; //will result in 0 for odd and 1 for even values. Used for error checking at reciever
  ButtonState = ButtonState + ( 64 * ParityBit) ;
  byte ButtonChar = (byte)ButtonState ;



  if (abs(lastSendSum - ( ButtonState + vertical + horizontal + CurrentScreenState)) > 2 || loopcount >= 40 || (loopcount % 10 == 5 && deadState == 0)) //only send state every 10ms if a value has changed OR //send state twice if 40/41 cycles have passed OR  //send state every 10 cycles if Deadman is pressed
  {         
     if(deadState ==0) loopcount = 0;
     
    if (loopcount == 40)                                                                                                                              
    {
      loopcount = 0;
      calibCount++;
    }

   
    
    lastSendSum = ButtonState + vertical + horizontal + CurrentScreenState;
    byte PacketChar[] = {VertChar, HorizChar, ButtonChar, ScreenChar};
    bluetooth.write(PacketChar[0]);
    bluetooth.write(PacketChar[1]);
    bluetooth.write(PacketChar[2]);
    bluetooth.write(PacketChar[3]);
    bluetooth.println();
    delay(5); //refresh rate delay when value is changing

  }
  else
  {
      delay(1); //determines cycle time for com status and Deadman updates
  }



}

////////////////////////////////////END OF LOOP////////////////////////////////


