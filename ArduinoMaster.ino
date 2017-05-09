
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

int btcount = 101;
char CurrentScreenState = '0';

int notconnected = 0;

void setup() {


  int previousbtc = 0;
  int btc = 0;
 

  Wire.begin();
  Serial.begin(9600);
  bluetooth.begin(19200);

  pinMode(Sync, INPUT);
  pinMode(Deadman, INPUT);
  pinMode(Zaxis, INPUT);
  pinMode(Estop, INPUT);
  pinMode(indicatorled, OUTPUT);
  delay (5000);
}
void loop() {



  bluetoothcounter = 0;
  if (count < upperlimit) {
    (count ++);
  }
  else if (count > lowerlimit) {
    (count --);
  }


  int bluetoothread = bluetooth.read();

  if (bluetoothread != -1) {
    btcount = 0;
  }
  if (bluetoothread == -1) {
    btcount ++;

    if (btcount == 100 ) {
      Wire.beginTransmission(1);
      Wire.write('H');
      Wire.endTransmission();
      CurrentScreenState = 'H';

    }
  }

  if (bluetoothread != previousbtc && bluetoothread < 100 || bluetoothread == -1  ) {
    previousbtc = bluetoothread;
    screencount = 0;
  }
  if (bluetoothread == previousbtc && bluetoothread < 100 && bluetoothread > -1 ) {

    previousbtc = bluetoothread;
    screencount ++;
  }







  if (screencount == 3 && bluetoothread < 100) {

    if (bluetoothread == ppbtc) {
      screencount = 0;
    } else {

      int btc = bluetoothread;

         Wire.beginTransmission(1);
        Wire.write(btc);
        Wire.endTransmission();
        if (btc == 'E' ||btc == 'F')
        {}else{
          CurrentScreenState = btc;
        }
    
      ppbtc = bluetoothread;
      previousbtc = bluetoothread;
    }


  }

  //////////////////////Joy Stick Read/////////////////////////////
  String vertical, horizontal ;   // read all values from the joystick
 
  vertical = ((map (analogRead(VERT), 0, 1023, 1023, 0) + 10000)); // will be 0-1023
  horizontal = (analogRead(HORIZ) + 10000); // will be 0-1023



  ///////////////////// Button State Read////////////////////////
  char DeadmanState; //= digitalRead(Deadman);
  char ZaxisState; //= digitalRead(Zaxis);
  char EstopState; //= digitalRead(Estop);
  char CountState;
  if (digitalRead(Deadman) == 1) {
    DeadmanState = 'F';
  } else if (digitalRead(Deadman) == 0) {
    DeadmanState = 'N';
  }
  if (digitalRead(Zaxis) == 1) {
    ZaxisState = 'F';
  } else if (digitalRead(Zaxis) == 0) {
    ZaxisState = 'N';
  }
  if (digitalRead(Estop) == 1) {
    EstopState = 'N';
  } else if (digitalRead(Estop) == 0) {
    EstopState = 'F';
  }
  if (count == 1) {
    CountState = 'N';
  } else if (count == 0) {
    CountState = 'F';
  }





  /////////////Toggle Bit///////////////////////////


  ///////////////////////////////////////PrintValues/////////////////////

  String packet = String("V" + vertical + "v" + "H" + horizontal + "h" + "Z" + ZaxisState + "z" + "D" + DeadmanState + "d" + "E" + EstopState + "e" + "C" + CountState + "c" + "J" + controllername + "j" + "S" + CurrentScreenState + "s");
  bluetooth.println(packet);




  ///////////////Battery indicator.//////////////////////////////////////////////

  //battVolts = getBandgap(); 

}


////////////////////////////////////END OF LOOP////////////////////////////////


/////////////////////Battery Checking Function//////////////////////////////

int getBandgap(void) // Returns actual value of Vcc (x 100)
{



  // For 168/328 boards
  const long InternalReferenceVoltage = 1056L;  // Adjust this value to your boards specific internal BG voltage x1000
  // REFS1 REFS0          --> 0 1, AVcc internal ref. -Selects AVcc external reference
  // MUX3 MUX2 MUX1 MUX0  --> 1110 1.1V (VBG)         -Selects channel 14, bandgap voltage, to measure
  ADMUX = (0 << REFS1) | (1 << REFS0) | (0 << ADLAR) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0);


  delay(50);  // Let mux settle a little to get a more stable A/D conversion
  // Start a conversion
  ADCSRA |= _BV( ADSC );
  // Wait for it to complete
  while ( ( (ADCSRA & (1 << ADSC)) != 0 ) );
  // Scale the value
  int results = (((InternalReferenceVoltage * 1024L) / ADC) + 5L) / 10L; // calculates for straight line value
  return results;


}

/*void State(void){

  char currentState = Wire.read();
  Serial.println (currentState);
  }*/





