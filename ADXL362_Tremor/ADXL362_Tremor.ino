/*
  ADXL362_SimpleRead.ino -  Simple XYZ axis reading example
  for Analog Devices ADXL362 - Micropower 3-axis accelerometer
  go to http://www.analog.com/ADXL362 for datasheet


  License: CC BY-SA 3.0: Creative Commons Share-alike 3.0. Feel free
  to use and abuse this code however you'd like. If you find it useful
  please attribute, and SHARE-ALIKE!

  Created June 2012
  by Anne Mahaffey - hosted on http://annem.github.com/ADXL362

  Modified May 2013
  by Jonathan Ruiz de Garibay

  Connect SCLK, MISO, MOSI, and CSB of ADXL362 to
  SCLK, MISO, MOSI, and DP 10 of Arduino
  (check http://arduino.cc/en/Reference/SPI for details)

*/

#include <SPI.h>
#include <ADXL362.h>
#include <SRAM_23LC.h>



ADXL362 xl;
SRAM_23LC SRAM(&SPI, A1, SRAM_23LC1024);

int16_t temp;
int16_t XValue, YValue, ZValue, Temperature;
float temp_m_s2;
char buf_in[10];

//int8_t  XLValue, YLValue, ZLValue, XHValue, YHValue, ZHValue;

int NbrAcc[7]; // '7' (instead of '5') of size because we don't use number zero and we need to let the last one empty to hold the required null character.
int8_t NbrAccCounter = 1;

int incomingByte = 0; // for incoming serial data
int8_t processing = 0; // Flag to show if we are already gathering data from accelerometers
int16_t Counter_30sec = 0; // We have 640 interrupt each seconds
int8_t Counter_slice = 0; // Counts the number of 30seconds slices. 30sec to 180sec <=> 1 to 6
int8_t slicesNbr = 0; // Contains the time asked by the user (time asked = slicesNbr * 30)
int START_ADDRESS_WRITE=250; 
int START_ADDRESS_READ=250; 

void setup() {

  Serial.begin(115200);
  xl.begin(9);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  xl.setFilterCL();               // MNT : Gforce=8, HALF_BW=1/2 (50Hz), ODR=100HZ  avant :GForce = 8, HALF_BW = 1/2 (100Hz), ODR = 200Hz 
  xl.setNoise();                  // UltraLow noise setting
  xl.beginMeasure();              // Switch ADXL362 to measure mode
  SRAM.begin(); 

  delay(500);
  //Serial.println("Start Demo");

  //Pin assignement
  NbrAcc[1] = 9;

  cli();//stop interrupts

  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;// initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 9999;// = [(8*10^6) / (100*8)] - 1 (must be <65536)  [Before := [(8*10^6) / (200*1)] - 1] //4999 pour 200Hz
  // turn on CTC modes
  TCCR1B |= (1 << WGM12);
  // Set CS10 bit for a prescaler of 1
  TCCR1B |= (1 << CS11); //prescaler de 
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts

}

ISR(TIMER1_COMPA_vect) { //timer1 interrupt

  union ByteSplit
    {
      int16_t int16; 
      int8_t int8[2];
     };
     
  if (processing == 127)
  {
    xl.readXYZData(NbrAcc[1], XValue, YValue, ZValue);

    ByteSplit ValueX;
    ValueX.int16= XValue; 
    SRAM.writeByte(START_ADDRESS_WRITE,ValueX.int8[1]); //Write MSB
    SRAM.writeByte(START_ADDRESS_WRITE+1,ValueX.int8[0]); // Write LSB
    /*temp_m_s2 = XValue;
    temp_m_s2 = (temp_m_s2 / 1024) * 9.81; //1024 (avant 256 (car 2g au lieu de 8)
    Serial.print(temp_m_s2);*/
    
    
    ByteSplit ValueY;
    ValueY.int16= YValue; 
    SRAM.writeByte(START_ADDRESS_WRITE+2,ValueY.int8[1]); //Write MSB
    SRAM.writeByte(START_ADDRESS_WRITE+3,ValueY.int8[0]); // Write LSB
    
    /*temp_m_s2 = YValue;
    temp_m_s2 = (temp_m_s2 / 1024) * 9.81; //1024 (avant 256)
    Serial.print(temp_m_s2);*/

   
    ByteSplit ValueZ;
    ValueZ.int16= ZValue; 
    SRAM.writeByte(START_ADDRESS_WRITE+4,ValueZ.int8[1]); //Write MSB
    SRAM.writeByte(START_ADDRESS_WRITE+5,ValueZ.int8[0]); // Write LSB

    START_ADDRESS_WRITE+=6;
    Counter_30sec++;

    if (Counter_30sec >= (100 * 30)) //200 = 1second   //Before: (640*30)) //640 = 1second
    {
      Counter_slice++;
      Counter_30sec = 0;
    }

    
    if (Counter_slice == slicesNbr) //Lorsque c'est terminé 
    {
      for(int i=0; i<=3000*slicesNbr; i++)
      {
        
        int8_t MSB;
        int8_t LSB; 
        int16_t wd;
        
        MSB=SRAM.readByte(START_ADDRESS_READ); //Read MSB
        LSB=SRAM.readByte(START_ADDRESS_READ+1); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.println((wd/1024)*9.81);
  
        Serial.print(",");
        MSB=SRAM.readByte(START_ADDRESS_READ+2); //Read MSB
        LSB=SRAM.readByte(START_ADDRESS_READ+3); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.println((wd/1024)*9.81);
  
        Serial.print(",");
        MSB=SRAM.readByte(START_ADDRESS_READ+4); //Read MSB
        LSB=SRAM.readByte(START_ADDRESS_READ+5); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.println((wd/1024)*9.81);
  
        START_ADDRESS_READ+=6; 
      }
      processing = 0;
      //Serial.println(processing);
      Counter_slice = 0;
      Counter_30sec = 0;
      START_ADDRESS_WRITE=250;
      START_ADDRESS_READ=250;
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
        // data processing started
        processing = 127;
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
