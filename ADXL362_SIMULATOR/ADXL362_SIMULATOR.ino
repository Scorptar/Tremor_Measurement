/*
  ADXL362 simulator in order to run TM with a ARDUINO NANO without ADXL362 accelerometers
  Needed hardware setup : Arduino nano only, plugged on COM3 for example using micro-usb cable.
  !!!16MHZ microcontroller (adjust OCR1A value in the code for 8MHz

*/

#include <SPI.h>
#include <ADXL362.h>

ADXL362 xl;
/*ADXL362 x2;
  ADXL362 x3;
  ADXL362 x4;
  ADXL362 x5;*/

int16_t temp;
int16_t XValue, YValue, ZValue, Temperature;
float temp_m_s2;
float i=0; //value sent
//int8_t  XLValue, YLValue, ZLValue, XHValue, YHValue, ZHValue;

int NbrAcc[7]; // '7' (instead of '5') of size because we don't use number zero and we need to let the last one empty to hold the required null character.
int8_t NbrAccCounter = 5;

int incomingByte = 0; // for incoming serial data
int8_t processing = 0; // Flag to show if we are already gathering data from accelerometers
int16_t Counter_30sec = 0; // We have 640 interrupt each seconds
int8_t Counter_slice = 0; // Counts the number of 30seconds slices. 30sec to 180sec <=> 1 to 6
int8_t slicesNbr = 0; // Contains the time asked by the user (time asked = slicesNbr * 30)

void setup() {

  Serial.begin(115200);
  xl.begin(9);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  xl.setFilterCL();               // MNT : Gforce=8, HALF_BW=1/2 (50Hz), ODR=100HZ  avant :GForce = 8, HALF_BW = 1/2 (100Hz), ODR = 200Hz 
  xl.setNoise();                  // UltraLow noise setting
  xl.beginMeasure();              // Switch ADXL362 to measure mode

  /*x2.begin(9);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
    x3.begin(8);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
    x4.begin(7);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
    x5.begin(6);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  */
  //Serial.println("Start Demo");

  //Pin assignement
  NbrAcc[1] = 9;
  /*NbrAcc[2] = 9;
    NbrAcc[3] = 8;
    NbrAcc[4] = 7;
    NbrAcc[5] = 6;  */

  cli();//stop interrupts

  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;// initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 19999;// = [(16*10^6) / (100*8)] - 1 (must be <65536) -- 9999 for same config on 8MHz microcontroller
  // turn on CTC modes
  TCCR1B |= (1 << WGM12);
  // Set CS10 bit for a prescaler of 1
  TCCR1B |= (1 << CS11); //prescaler de 
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts

}

ISR(TIMER1_COMPA_vect) { //timer1 interrupt

  if (processing == 127)
  {
    i=i+1;//1-->3000*slicesNbr
    
    for (int j=1;j<=NbrAccCounter;j++){
      Serial.print(i);
      Serial.print(",");
      Serial.print(i);
      Serial.print(",");
      Serial.print(i);
      if (j != NbrAccCounter) //between each sensor
        Serial.print(",");
      if (j==NbrAccCounter)
        Serial.println("");
    }
    
    Counter_30sec++;

    if (Counter_30sec >= (100 * 30)) //200 = 1second   //Before: (640*30)) //640 = 1second
    {
      Counter_slice++;
      Counter_30sec = 0;
      //Serial.println(Counter_slice);
    }
    if (Counter_slice == slicesNbr)
    {
      processing = 0;
      //Serial.println(processing);
      Counter_slice = 0;
      Counter_30sec = 0;
      i=0; //reset data to send
      //Serial.println("END");
    }
  }
}

void loop() {
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    if (processing == 0)
    {
      if (incomingByte >= 65 and incomingByte <= 70)
      {
        slicesNbr = incomingByte - 64; // ASCII de A(30s) = 65, F(180s) = 70.
        delay(5000); //wait 5 seconds to see on TM interface what happen during what should be the measure
        // data processing started
        processing = 127;
        //Serial.println(processing);
      }
      else
      {
        Serial.println("Need to receive a letter from A (30 seconds) to F (180 seconds)");
      }
    }
    else
    {
      Serial.println("Data are already processing");
    }
  }

}
