/*
  ADXL362 simulator in order to run TM with a ARDUINO NANO without ADXL362 accelerometers
  Needed hardware setup : Arduino nano only, plugged on COM3 for example using micro-usb cable.
  !!!16MHZ microcontroller (adjust OCR1A value in the code for 8MHz

*/

#include <SPI.h>

int8_t NbrAccCounter = 5;
int incomingByte = 0; // for incoming serial data
int8_t slicesNbr = 0; // Contains the time asked by the user (time asked = slicesNbr * 30)

void setup() {
  Serial.begin(115200);
}

void loop() {
    incomingByte = Serial.read();
    if (incomingByte >= 65 and incomingByte <= 70)
    {
      slicesNbr = incomingByte - 64; // ASCII de A(30s) = 65, F(180s) = 70.
      delay(2000); //wait 5 seconds to see on TM interface what happen during what should be the measure
      for (int i=1;i<=slicesNbr*3000;i++) {
        for (int j=1;j<=NbrAccCounter;j++){
          Serial.flush();
          Serial.print(j*100);
          Serial.flush();
          Serial.print(",");
          Serial.flush();
          Serial.print(j*200);
          Serial.flush();
          Serial.print(",");
          Serial.flush();
          Serial.print(j*300);
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
