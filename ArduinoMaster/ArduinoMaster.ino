
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


int battVolts;   // made global for wider avaliblity throughout a sketch if needed, example a low voltage alarm, etc
int DeadmanState = 0;
int SyncState = 0 ;
int ZaxisState = 0;
int EstopState = 0;

int previousbtc = 0; //Previous bluetooth command

int btTryCount = 1001; //This increases when nothing is sent to the contoller and resets when a command is recieved. Used to test connection.
int CurrentScreenState = 0;
int btc;
int loopcount=0;


bool debug = true;
void setup() {
 
  Wire.begin();
  Wire.beginTransmission(1);
  if (debug)Serial.begin(9600);
  bluetooth.begin(19200);
  pinMode(Sync, INPUT);
  pinMode(Deadman, INPUT);
  pinMode(Zaxis, INPUT);
  pinMode(Estop, INPUT);
  pinMode(indicatorled, OUTPUT);
    delay (3000);
  //  CurrentScreenState = bluetooth.read();
  //bluetooth.listen();
}

void loop() {
loopcount ++;
if(loopcount %100 == 5) Serial.println(loopcount);
  bluetooth.listen();
  btc = bluetooth.read();

  if (btc == -1)
  {
    if (btTryCount < 10000) btTryCount ++;
  }
  else
  {
      
    btTryCount = 0;
  }

  if (btTryCount == 10000 )
  {
    if (debug)Serial.println(btc);
    Wire.write(72);
    btTryCount ++;
   }


if (btc != previousbtc && btc > 47 && btc < 100 ) //if the character has changed store it to check for doubles on next loop
{
     previousbtc = btc; 
}
else if (btc == previousbtc && btc < 100 && btc > 47) //if the same character is read twice, and it is between 2 and 100
{ 
   
    if( (btc == 69 || btc == 70))
    {
        if (ShapeStateDeadMan != btc) {Wire.write(btc); if (debug)Serial.print("DEADMAN: "); if (debug)Serial.println(btc);}
        ShapeStateDeadMan = btc;  //for smaller objects like Estop, and interlocks
    }
    else if ( (btc < 69 && btc >  64))
    {
        if (ShapeStatePin != btc) {Wire.write(btc); if (debug)Serial.print("    PIN: "); if (debug)Serial.println(btc);}
        ShapeStatePin = btc;  //for smaller objects like Estop, and interlocks
    }
    else if ( (btc > 49 && btc <73 ) && (btc <55 || btc > 65))
    {
        //if (debug)Serial.println(btc);
        if(CurrentScreenState != btc) 
        { 
            Wire.write(btc); 
            if(debug)Serial.println(btc);
            CurrentScreenState = btc; 
            if(debug)Serial.print("CURRENT SCREEN: ");
            if(debug)Serial.println(CurrentScreenState);
        }
          
     }
     
      previousbtc = btc; 
   
}




////Serial.println(loopcount);
//////////////////////Joy Stick Read/////////////////////////////
String vertical, horizontal ;   // read all values from the joystick

vertical = analogRead(VERT); // will be 0-1023
horizontal = analogRead(HORIZ); // will be 0-1023

//Adjust min & max for in line calibration
if (vertical.toInt() < vMinValueCalib ) vMinValueCalib = vertical.toInt() + 1;
if (vertical.toInt() > vMaxValueCalib) vMaxValueCalib = vertical.toInt() - 1;
if (horizontal.toInt() < hMinValueCalib) hMinValueCalib = horizontal.toInt() + 1;
if (horizontal.toInt() > hMaxValueCalib) hMaxValueCalib = horizontal.toInt() - 1;

if ( false && loopcount % 40 ==5  )
{
  Serial.print("hMIN: " );  Serial.print(hMinValueCalib );  Serial.print("   hMAX: " );  Serial.print( hMaxValueCalib);  Serial.print("   hRAW:" + horizontal);
  Serial.print("    vMIN: " );  Serial.print(vMinValueCalib );  Serial.print("   vMAX: " );  Serial.print( vMaxValueCalib);  Serial.println("   vRAW:" + vertical);
}


vertical = (map (vertical.toInt(), vMinValueCalib, vMaxValueCalib, 1, 1023) ) + 10000; // will be 10-1010
horizontal = (map(horizontal.toInt(), hMinValueCalib, hMaxValueCalib, 1, 1023)+ 10000 )  ; // will be 10-1010


///////////////////// Button State Read////////////////////////
char DeadmanState = '0'; //= digitalRead(Deadman);
char ZaxisState= '0'; //= digitalRead(Zaxis);
char EstopState= '0'; //= digitalRead(Estop);
char CountState= '0';
int CheckVal;

CheckVal = digitalRead(Deadman);
if (CheckVal == 1) {
  DeadmanState = 'F';
} else if (CheckVal == 0) {
  DeadmanState = 'N';
}
CheckVal = digitalRead(Zaxis);
if (CheckVal == 1) {
  ZaxisState = 'F';
} else if (CheckVal == 0) {
  ZaxisState = 'N';
}
CheckVal = digitalRead(Estop);
if (CheckVal == 1) {
  EstopState = 'N';
} else if (CheckVal == 0) {
  EstopState = 'F';
}
 if(CountState == 'N') CountState = 'F';
 else CountState = 'N';

///////////////////////////////////////PrintValues/////////////////////
//CurrentScreenState = 72;
char CurrentScreenChar = (char)CurrentScreenState + '0';

String CurrentScreenString = String(CurrentScreenChar);
CurrentScreenString.toUpperCase();

Serial.print(CurrentScreenState);
Serial.print(CurrentScreenChar);

String packet = String("V" + vertical + "v" + "H" + horizontal + "h" + "Z" + ZaxisState + "z" + "D" + DeadmanState + "d" + "E" + EstopState + "e" + "C" + CountState + "c" + "J" + controllername + "j" + "S" + CurrentScreenString + "s");
bluetooth.println(packet);


Wire.endTransmission();
if(debug)Serial.println(packet);
Wire.beginTransmission(1);


}


////////////////////////////////////END OF LOOP////////////////////////////////


