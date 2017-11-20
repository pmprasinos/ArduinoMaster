
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

///////////////////Inputs//////////////////////////


static const uint8_t JoyStick[4][3] =  {{A0 , A1, A2}, {A3 , A4 , A5}, {A6 , A7 , A8},       {A9, A10, A11}}  ; // [JoystickNumber][x, y, z]
static uint8_t Buttons[4][8] = {{30, 31, 32, 33, 34, 35, 37, 39}, {2, 3, 4, 5, 6, 7, 8, 9}, { 10, 11, 12, 13, 14, 44, 45, 46}, {0, 0, 0, 0, 0, 0, 0, 0}}; //Buttons[0] = digital input, [1]=PWM for color 1, [2]=PWM for color 2, [3] = Color mode;
float Brightness[8] = {0, 0, 0, 0, 0, 0, 0, 0};
long iv1 = 970;
long MaxValueCalib[4][3] = {{iv1, iv1, iv1}, {iv1, iv1, iv1}, {iv1, iv1, iv1}, {iv1, iv1, iv1}}; //x y and z
long iv2 = 50;                             
long MinValueCalib[4][3] = {{iv2, iv2, iv2}, {iv2, iv2, iv2}, {iv2, iv2, iv2}, {iv2, iv2, iv2}};
long iv3 = 0;                 
long Trim[4][3] = {{iv3, iv3, iv3}, {iv3, iv3, iv3}, {iv3, iv3, iv3}, {iv3, iv3, iv3}};

int loopcount = 0; int calibCount = 0;
bool debug = false;
float adj = 0.09;
long blinkTime; 

void setup() {
  blinkTime = millis();
  analogReference(EXTERNAL);
  analogRead(0);
  Serial.begin(19200);
  Serial.write("IM THE MF CONSOLE CONTROLS");
  //   for (int i = 0; i <= sizeof(Buttons)/3;  i++)  {      for(int x = 0; x <=255; x++) {analogWrite(Buttons[1][i], x ); Serial.print("9"); delay(1);}  }
  //    delay(200);
  //   for (int i = 0; i <= sizeof(Buttons)/3;  i++)  {   analogWrite(Buttons[1][i], 0 ); Serial.print("9");}
     
 for (int i = 0; i <= sizeof(Buttons)/3;  i++)
  {
   
      pinMode(Buttons[0][i], INPUT);
      if(Buttons[1][i]!=0) pinMode(Buttons[1][i], OUTPUT);
      if(Buttons[2][i]!=0)   pinMode(Buttons[2][i], OUTPUT);
  }
}

void loop()
{
    if(Serial.available()>0)
    {
        int rs = Serial.read();
        if(rs != 10 && rs != 13)
        {
          int buttonNumber = rs / 8;
          if(debug){Serial.print("BUTTONS:"); Serial.print( buttonNumber); Serial.print( "STATE:"); Serial.println( rs%8);}
          Buttons[3][buttonNumber] = rs % 8;
        }
    }
    
     
    
 //if((int)(Brightness + 10)>254) Brightness = 0;
     // 

 

      loopcount = 0;
      calibCount++;
      if (calibCount == 10) calibCount = 0; //calibCount is used for the damping for center calibration

       for (int js = 0; js <= 3;  js++){SendJSData(js); delay(1);}
        SendButtonData();
        
 buttonBrightness();

}

      bool pulseMode[8] = {false,false,false,false,false,false,false, false};
      
void buttonBrightness()
{
   for (int i = 0; i <= sizeof(Buttons)/4;  i++)
  {
      int buttonState = Buttons[3][i];

      switch(buttonState)
      {
        case 0:
           Brightness[i] = 0;
          break;
        case 1: 
          Brightness[i] = 255;
          break;
        case 2: //slow blink
          Brightness[i] = ((int)(millis()-blinkTime > 800)) * 255;
          if(millis()-blinkTime > 1800) blinkTime = millis();
          break;
        case 3: //fast blink
          Brightness[i] = ((int)(millis()-blinkTime > 100)) * 255;
          if(millis()-blinkTime > 200) blinkTime = millis();
        case 4: //Slow Pulse
          if(!pulseMode[i]) adj = 3.21; pulseMode[i] = true;
          if((int)Brightness[i] < abs(adj)) adj=abs(adj);
          if((int)Brightness[i] > 250-abs(adj) ) adj = -adj;
          if(Brightness[i] < 60.0) Brightness[i] = Brightness[i] +(adj/3); else {Brightness[i] = Brightness[i] +adj; if(Brightness[i] > 160.0) Brightness[i] = Brightness[i] +(adj);}
          break;
       case 5: //Pulse
            if(!pulseMode[i]) adj = 6.49; pulseMode[i] = true;
                 if((int)Brightness[i] < abs(adj)) adj=abs(adj);
          if((int)Brightness[i] > 250-abs(adj) ) adj = -adj;
          if(Brightness[i] < 60.0) Brightness[i] = Brightness[i] +(adj/3); else {Brightness[i] = Brightness[i] +adj;  if(Brightness[i] > 160.0) Brightness[i] = Brightness[i] +(adj); }
          break;     
       }
        if(buttonState<4) pulseMode[i] = false;
      Serial.println(Brightness[i]);
       analogWrite(Buttons[1][i], (int)Brightness[i]);
      
      
  }
     
 
}





