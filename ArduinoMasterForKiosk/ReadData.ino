

void SendJSData(int JS)
{
  int check = JoyStick[JS][2]+ Trim[JS][0] +MaxValueCalib[JS][0] +MinValueCalib[JS][2] ;
  byte jsChar = (byte)(JS);
 Serial.print(JS);
 if(debug)Serial.print("  ");
 for (int i = 0; i <= 2;  i++)
  {
    int deadState = 1;
    long X;   // read all values from the joystick
  
    X = analogRead(JoyStick[JS][i]) ; // will be 0-1023
     if( debug) {Serial.print(X); Serial.print("   "); delay(10);}
  
    //Adjust min & max for in line calibration. Values offset by one to prevent overcorrection in single loop

    if (X < MinValueCalib[JS][i]) MinValueCalib[JS][i] = X + 1;
    if (X > MaxValueCalib[JS][i]) MaxValueCalib[JS][i] = X - 1;


    
  if (X + Trim[JS][i]  > 512)
    {
      if (calibCount == 0 && deadState == 1 && X + Trim[JS][i] > 511) {Trim[JS][i] = Trim[JS][i] - 1;  calibCount++;}
    }
    else 
    {
      if (calibCount == 0 && deadState == 1 && X + Trim[JS][i] < 511){ Trim[JS][i] = Trim[JS][i] + 1;calibCount++;}
    }
  
    if (X + Trim[JS][i]  >= 518)
    {
      X = (512L - (X + Trim[JS][i])) * (512L - (X + Trim[JS][i]));
      X = map (X ,  0L, (518L - (MaxValueCalib[JS][i] + Trim[JS][i])) * (518L - (MaxValueCalib[JS][i] + Trim[JS][i])), 150L, 250L)  ; // will result in 14-255
    }
    else if (X + Trim[JS][i]  < 506)
    {
      X = (511L - (X + Trim[JS][i])) * (511L - (X + Trim[JS][i]));
      X = map (X , (506L - (MinValueCalib[JS][i] + Trim[JS][i])) * (506L - (MinValueCalib[JS][i] + Trim[JS][i])), 0L,  50L, 150L)  ; // will result in 14-255
    }
    else
    {
      X = 150;
    }

    if (X > 250) X = 250;
    if (X < 50) X = 50;

    byte XChar = (byte)X;
    if(!debug)Serial.write(XChar);
  

 }
Serial.println();
}


void SendButtonData()
{

  byte jsChar = (byte)5;
   if(!debug)Serial.print(5);;
  int ButtonState = 0;
  int ButtonBit = 1;
   for(int i = 0; i < 8; i++)
   {
      int j = (int)digitalRead(Buttons[0][i]);
      ButtonState = ButtonState + (ButtonBit * j);
      ButtonBit = ButtonBit * 2;
   }
   for (int i = 0; i <= 2;  i++)
  {
     //if (ButtonState > 250) ButtonState = 250;
    //if (ButtonState < 50) ButtonState = 50;
    byte XChar = (byte)ButtonState;
    if(!debug)Serial.write(XChar);
    if(debug) {Serial.print("Buttons: "); Serial.print(ButtonState);}

 }
if(!debug)Serial.println();
}


