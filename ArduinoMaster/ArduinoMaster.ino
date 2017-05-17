
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

#define Deadman  4
#define Estop  5
#define Zaxis  6
#define Sync  7
#define AccelZ  A3


int indicatorled = 13;
//////////////////Global Variables/////////////////
int ShapeStateDeadMan = 0;  //for smaller objects like Estop, and interlocks
int ShapeStatePin = 0;
int ShapeStateCarrier = 0;

int hMaxValueCalib = 900; int hMinValueCalib = 100;//put initial calibration values inside expected range of 0-1024
int vMaxValueCalib = 900; int vMinValueCalib = 100;


int lastSendSum = 0;
int ButtonState = 0;

int battVolts;   // made global for wider avaliblity throughout a sketch if needed, example a low voltage alarm, etc
int DeadmanState = 0;
int SyncState = 0 ;
int ZaxisState = 0;
int EstopState = 0;

int previousbtc = 0; //Previous bluetooth command

int btTryCount = 1001; //This increases when nothing is sent to the contoller and resets when a command is recieved. Used to test connection.
int CurrentScreenState = 0;
int btc;
int loopcount = 0;


bool debug = false;
void setup() {

  Wire.begin();
  //Wire.beginTransmission(1);
  if(debug) Serial.begin(9600);

  bluetooth.begin(9600);
  pinMode(Sync, INPUT);
  pinMode(Deadman, INPUT);
  pinMode(Zaxis, INPUT);
  pinMode(Estop, INPUT);
  pinMode(indicatorled, OUTPUT);
  delay (3000);
  //  CurrentScreenState = bluetooth.read();

}

void loop() {
  loopcount ++;

  if (bluetooth.available() >= 1 ) {
    // read the incoming byte:
    btc = bluetooth.read();
    bluetooth.write(btc);



    if (btc == -1)
    {
      if (btTryCount < 10000) btTryCount ++;
    }
    else
    {
      if (debug)Serial.write(btc);
      if (debug)Serial.println(btc);
      btTryCount = 0;
    }

    if (btTryCount == 10000 )
    {
      if (debug)Serial.println(btc);
      Wire.beginTransmission(1);
      Wire.write(72);
      Wire.endTransmission();
      return;
      btTryCount ++;
    }


    if (btc != previousbtc && btc > 47 && btc < 100 ) //if the character has changed store it to check for doubles on next loop
    {
      previousbtc = btc;
    }
    else if (btc == previousbtc && btc < 100 && btc > 47) //if the same character is read twice, and it is between 2 and 100
    {

      if (btc == 69 || btc == 70)
      {
        if (ShapeStateDeadMan != btc) {

             if (debug)Serial.print("DEADMAN: ");
          if (debug)Serial.println(btc);
          Wire.beginTransmission(1);
          Wire.write(btc);
          Wire.endTransmission();
          ShapeStateDeadMan = btc;
          return;
     
        }
        //for smaller objects like Estop, and interlocks
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
          return;
          
        }
        //for smaller objects like Estop, and interlocks
      }
      else if ( (btc > 49 && btc < 73 ) && (btc < 57 || btc > 65))
      {
        //if (debug)Serial.println(btc);
        if (CurrentScreenState != btc)
        {
          if (debug)Serial.println(btc);
          CurrentScreenState = btc;
          if (debug)Serial.print("CURRENT SCREEN: ");
          if (debug)Serial.println(CurrentScreenState);
          Wire.beginTransmission(1);
          Wire.write(btc);
          Wire.endTransmission();
          return;

        }
        if (debug)Serial.print("BYTE READ: ");
        if (debug) Serial.print(btc, DEC);
        if (debug) Serial.write(" ");
        if (debug) Serial.write(btc);
        if (debug) Serial.println();
      }

      previousbtc = btc;

    }
  }

  SendJSData();

}




void SendJSData()
{

  int vertical, horizontal ;   // read all values from the joystick

  vertical = analogRead(VERT); // will be 0-1023
  horizontal = analogRead(HORIZ); // will be 0-1023

  //Adjust min & max for in line calibration. Values offset by one to prevent overcorrection in single loop
  if (vertical < vMinValueCalib ) vMinValueCalib = vertical + 1;
  if (vertical > vMaxValueCalib) vMaxValueCalib = vertical - 1;
  if (horizontal < hMinValueCalib) hMinValueCalib = horizontal + 1;
  if (horizontal > hMaxValueCalib) hMaxValueCalib = horizontal - 1;


  vertical = map (vertical, vMinValueCalib, vMaxValueCalib, 14, 255)  ; // will result in 14-255
  horizontal = map(horizontal, hMinValueCalib, hMaxValueCalib, 14, 255) ; // will result in 14-255


  ///////////////////////////////////////Make Packet/////////////////////
  //////////////////////////Packet will consist of 4 bytes [vertical, horizontal, ButtonState, Screen] with one checkbit included in
  //CurrentScreenState = 72;


  ButtonState = digitalRead(Deadman) * 4 +
                digitalRead(Estop) * 8 +
                digitalRead(Zaxis) * 16 +
                digitalRead(Sync) * 32;



  byte ScreenChar = (byte)CurrentScreenState ;
  byte VertChar = (byte)vertical;
  byte HorizChar = (byte)horizontal;

  int checkval =  VertChar + HorizChar + ButtonState + ScreenChar;

  int  ParityBit =  checkval % 2; //will result in 0 for odd and 1 for even values. Used for error checking at reciever
  ButtonState = ButtonState + ( ParityBit) ;
  byte ButtonChar = (byte)ButtonState ;



  if (abs(lastSendSum -( ButtonState + vertical + horizontal + CurrentScreenState)) > 2 || loopcount > 40 )
  {
    if(debug) Serial.println(lastSendSum);
    if(debug) Serial.println(loopcount);
    if(loopcount == 40) loopcount++;
    if(loopcount == 41) loopcount = 0;
    lastSendSum = ButtonState + vertical + horizontal + CurrentScreenState;

    byte PacketChar[] = {VertChar, HorizChar, ButtonChar, ScreenChar};
    bluetooth.write(PacketChar[0]);
    bluetooth.write(PacketChar[1]);
    bluetooth.write(PacketChar[2]);
    bluetooth.write(PacketChar[3]);
    bluetooth.println();
    delay(4);
  } else{
    delay(6);
  }

}

////////////////////////////////////END OF LOOP////////////////////////////////


