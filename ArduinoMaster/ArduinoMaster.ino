
#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>



/////////////////////////Controller Name//////////////////////////////////

char controllername[5] = {'0', '0', '0', '7'} ; // To be changed with every controller.

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
//const int AccelX = A1;
//const int AccelY = A2;
#define AccelZ  A3

int indicatorled = 13;
//////////////////Global Variables/////////////////

int battVolts;   // made global for wider avaliblity throughout a sketch if needed, example a low voltage alarm, etc
int DeadmanState = 0;
int SyncState = 0 ;
int ZaxisState = 0;
int EstopState = 0;
int upperlimit = 1; //Counter for Toggle bit
int lowerlimit = 0; //Counter for Toggle bit
int count; // for the Toggle bit
int bluetoothcounter = 0; //This increases when nothing is sent to the contoller and resets when a command is recieved. Used to test connection.
int previousbtc = 0;
int ppbtc = 0;
int screencount = 0;
int loopcount = 0;

int btcount = 101;
char CurrentScreenState = '0';
int bluetoothread;
int notconnected = 0;

void setup() {


  int previousbtc = 0;
  int btc = 0;


  Wire.begin();
  Wire.beginTransmission(1);
  Serial.begin(9600);
  bluetooth.begin(19200);
  Serial.println(loopcount);
  pinMode(Sync, INPUT);
  pinMode(Deadman, INPUT);
  pinMode(Zaxis, INPUT);
  pinMode(Estop, INPUT);
  pinMode(indicatorled, OUTPUT);
  delay (5000);
  //bluetooth.listen();
}

void loop() {
  loopcount++;
  ////Serial.println(loopcount);


  bluetoothcounter = 0;
  if (count < upperlimit) {
    (count ++);
  }
  else if (count > lowerlimit) {
    (count --);
  }


  bluetooth.listen()
  bluetoothread = bluetooth.read();
  if (bluetoothread < 100) bluetoothread = bluetooth.read();

  ////Serial.println(loopcount);

  if (bluetoothread != -1) {
    btcount = 0;
  }
  if (bluetoothread == -1)
  {
    btcount ++;

    if (btcount == 100 )
    {
      Wire.beginTransmission(1);
      Wire.write('H');
      CurrentScreenState = 'H';
    }
  }

  if (bluetoothread != previousbtc && bluetoothread < 100 || bluetoothread == -1  )
  {
    previousbtc = bluetoothread;
    screencount = 0;
  }
  if (bluetoothread == previousbtc && bluetoothread < 100 && bluetoothread > -1 )
  {
    // Serial.println(bluetoothread);
    previousbtc = bluetoothread;
    screencount ++;
  }


  if (screencount == 3 && bluetoothread < 100)
  {

    if (bluetoothread == ppbtc) {
      screencount = 0;
    } else {

      int btc = bluetoothread;
      Serial.println(bluetoothread);
      Wire.beginTransmission(1);
      Wire.write(btc);

      if (btc == 'E' || btc == 'F')
      {} else {
        CurrentScreenState = btc;
      }

      ppbtc = bluetoothread;
      previousbtc = bluetoothread;
    }


  }



  ////Serial.println(loopcount);
  //////////////////////Joy Stick Read/////////////////////////////
  String vertical, horizontal ;   // read all values from the joystick

  vertical = ((map (analogRead(VERT), 0, 1023, 1023, 0) + 10000)); // will be 0-1023
  horizontal = (analogRead(HORIZ) + 10000); // will be 0-1023



  ///////////////////// Button State Read////////////////////////
  char DeadmanState; //= digitalRead(Deadman);
  char ZaxisState; //= digitalRead(Zaxis);
  char EstopState; //= digitalRead(Estop);
  char CountState;
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
  if (count == 1) {
    CountState = 'N';
  } else if (count == 0) {
    CountState = 'F';
  }
  ////Serial.println(loopcount);


  //delay(1);
  /////////////Toggle Bit///////////////////////////


  ////Serial.println(loopcount);
  ///////////////////////////////////////PrintValues/////////////////////

  Wire.endTransmission();

  String packet = String("V" + vertical + "v" + "H" + horizontal + "h" + "Z" + ZaxisState + "z" + "D" + DeadmanState + "d" + "E" + EstopState + "e" + "C" + CountState + "c" + "J" + controllername + "j" + "S" + CurrentScreenState + "s");
  bluetooth.println(packet);





}


////////////////////////////////////END OF LOOP////////////////////////////////


