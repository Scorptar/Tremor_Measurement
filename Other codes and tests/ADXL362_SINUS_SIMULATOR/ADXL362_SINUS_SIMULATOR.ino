/*
  ADXL362 simulator in order to run TM with a ARDUINO NANO without ADXL362 accelerometers
  Needed hardware setup : Arduino nano only, plugged on COM3 for example using micro-usb cable.
  !!!16MHZ microcontroller (adjust OCR1A value in the code for 8MHz
  !!!Set simu on True in TM.py code (global variables section)

*/

#include <SPI.h>

#define   N         3000
#define   pi        3.1415926535897932384626433832795
#define   f0        15
#define   fe        100

int8_t NbrAccCounter = 5;
int incomingByte = 0; // for incoming serial data
int8_t slicesNbr = 0; // Contains the time asked by the user (time asked = slicesNbr * 30)

int8_t pos;

unsigned int i_sin=0; 
double sin_x;

void setup() {
  Serial.begin(1000000);
  i_sin=0;  
}

void loop() {
    incomingByte = Serial.read();
    if (incomingByte >= 65 and incomingByte <= 70)
    {
      slicesNbr = incomingByte - 64; // ASCII de A(30s) = 65, F(180s) = 70.
      delay(2000); //wait 5 seconds to see on TM interface what happen during what should be the measure
      for (int i=1;i<=slicesNbr*3000;i++) {
        sin_x=5*sin(2.00*pi*f0*(float)i/((float)fe));
        for (int j=1;j<=NbrAccCounter;j++){
          Serial.flush();
          if (i_sin==N and j==1)
          {
            i_sin=0;
            Serial.print(sin_x);
            i_sin++; 
          }
          else if (i_sin !=N and j==1)
          {
            Serial.print(sin_x);
            i_sin++;
          }
          else
          {
            Serial.print((float)i/fe);
          }
          Serial.flush();
          Serial.print(",");
          Serial.flush();
          Serial.print(j);
          Serial.flush();
          Serial.print(",");
          Serial.flush();
          Serial.print(j);
          Serial.flush();
          if (j != NbrAccCounter) //between each sensor
            Serial.print(",");
          Serial.flush();
          if (j==NbrAccCounter)
            Serial.println("");
          Serial.flush();
        }
      }
    }
}
