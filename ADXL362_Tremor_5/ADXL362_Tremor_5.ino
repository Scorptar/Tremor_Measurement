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
ADXL362 x2;
ADXL362 x3;
ADXL362 x4;
ADXL362 x5;

SRAM_23LC SRAM(&SPI, A1, SRAM_23LC1024);
SRAM_23LC SRAM2(&SPI, A0, SRAM_23LC1024);
SRAM_23LC SRAM3(&SPI, A2, SRAM_23LC1024);
SRAM_23LC SRAM4(&SPI, A3, SRAM_23LC1024);
SRAM_23LC SRAM5(&SPI, 10, SRAM_23LC1024);

int16_t temp;
int16_t XValue, YValue, ZValue, Temperature;
float temp_m_s2;


int NbrAcc[7]; // '7' (instead of '5') of size because we don't use number zero and we need to let the last one empty to hold the required null character.
int8_t NbrAccCounter = 1;

int incomingByte = 0; // for incoming serial data
int8_t processing = 0; // Flag to show if we are already gathering data from accelerometers
int16_t Counter_30sec = 0; // We have 640 interrupt each seconds
int8_t Counter_slice = 0; // Counts the number of 30seconds slices. 30sec to 180sec <=> 1 to 6
int8_t slicesNbr = 0; // Contains the time asked by the user (time asked = slicesNbr * 30)


int START_ADDRESS_WRITE=100; 
int START_ADDRESS_READ=100;

int START_ADDRESS_WRITE_2=100; 
int START_ADDRESS_READ_2=100;

int START_ADDRESS_WRITE_3=100; 
int START_ADDRESS_READ_3=100;

int START_ADDRESS_WRITE_4=100; 
int START_ADDRESS_READ_4=100;

int START_ADDRESS_WRITE_5=100; 
int START_ADDRESS_READ_5=100;

void setup() {

  Serial.begin(115200);

  xl.begin(9);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  xl.setFilterCL();               // MNT : Gforce=8, HALF_BW=1/2 (50Hz), ODR=100HZ  avant :GForce = 8, HALF_BW = 1/2 (100Hz), ODR = 200Hz 
  xl.setNoise();                  // UltraLow noise setting
  xl.beginMeasure();              // Switch ADXL362 to measure mode

  x2.begin(8);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  x2.setFilterCL();               // MNT : Gforce=8, HALF_BW=1/2 (50Hz), ODR=100HZ  avant :GForce = 8, HALF_BW = 1/2 (100Hz), ODR = 200Hz 
  x2.setNoise();                  // UltraLow noise setting
  x2.beginMeasure();              // Switch ADXL362 to measure mode

  x3.begin(7);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  x3.setFilterCL();               // MNT : Gforce=8, HALF_BW=1/2 (50Hz), ODR=100HZ  avant :GForce = 8, HALF_BW = 1/2 (100Hz), ODR = 200Hz 
  x3.setNoise();                  // UltraLow noise setting
  x3.beginMeasure();              // Switch ADXL362 to measure mode
  
  x4.begin(6);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  x4.setFilterCL();               // MNT : Gforce=8, HALF_BW=1/2 (50Hz), ODR=100HZ  avant :GForce = 8, HALF_BW = 1/2 (100Hz), ODR = 200Hz 
  x4.setNoise();                  // UltraLow noise setting
  x4.beginMeasure();              // Switch ADXL362 to measure mode

  x5.begin(5);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  x5.setFilterCL();               // MNT : Gforce=8, HALF_BW=1/2 (50Hz), ODR=100HZ  avant :GForce = 8, HALF_BW = 1/2 (100Hz), ODR = 200Hz 
  x5.setNoise();                  // UltraLow noise setting
  x5.beginMeasure();              // Switch ADXL362 to measure mode

  SRAM.begin();
  SRAM2.begin();
  SRAM3.begin();
  SRAM4.begin();
  SRAM5.begin(); 

  delay(500);
  //Serial.println("Start Demo");

  //Pin assignement
  NbrAcc[1] = 9;
  NbrAcc[2] = 8;
  NbrAcc[3] = 7;
  NbrAcc[4] = 6;
  NbrAcc[5] = 5;

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

    ByteSplit ValueX1;
    ValueX1.int16= XValue; 
    SRAM.writeByte(START_ADDRESS_WRITE,ValueX1.int8[1]); //Write MSB
    SRAM.writeByte(START_ADDRESS_WRITE+1,ValueX1.int8[0]); // Write LSB
    
    /*temp_m_s2 = XValue;
    temp_m_s2 = (temp_m_s2 / 1024) * 9.81; //1024 (avant 256 (car 2g au lieu de 8)
    //Serial.print("Capt1");
    Serial.print(temp_m_s2);*/

    ByteSplit ValueY1;
    ValueY1.int16= YValue; 
    SRAM.writeByte(START_ADDRESS_WRITE+2,ValueY1.int8[1]); //Write MSB
    SRAM.writeByte(START_ADDRESS_WRITE+3,ValueY1.int8[0]); // Write LSB
    
    /*Serial.print(",");
    temp_m_s2 = YValue;
    temp_m_s2 = (temp_m_s2 / 1024) * 9.81; //1024 (avant 256)
    Serial.print(temp_m_s2);*/

    ByteSplit ValueZ1;
    ValueZ1.int16= ZValue; 
    SRAM.writeByte(START_ADDRESS_WRITE+4,ValueZ1.int8[1]); //Write MSB
    SRAM.writeByte(START_ADDRESS_WRITE+5,ValueZ1.int8[0]); // Write LSB
    
    
    /*Serial.print(",");
    temp_m_s2 = ZValue;
    temp_m_s2 = (temp_m_s2 / 1024) * 9.81; //1024 (avant 256)
    Serial.print(temp_m_s2);  // Serial.println(ZValue); //To have a clean receiving
    Serial.print(",");*/

   
    x2.readXYZData(NbrAcc[2], XValue, YValue, ZValue);

    ByteSplit ValueX2;
    ValueX2.int16= XValue; 
    SRAM2.writeByte(START_ADDRESS_WRITE_2,ValueX2.int8[1]); //Write MSB
    SRAM2.writeByte(START_ADDRESS_WRITE_2+1,ValueX2.int8[0]); // Write LSB

    ByteSplit ValueY2;
    ValueY2.int16= YValue; 
    SRAM2.writeByte(START_ADDRESS_WRITE_2+2,ValueY2.int8[1]); //Write MSB
    SRAM2.writeByte(START_ADDRESS_WRITE_2+3,ValueY2.int8[0]); // Write LSB
    

    ByteSplit ValueZ2;
    ValueZ2.int16= ZValue; 
    SRAM2.writeByte(START_ADDRESS_WRITE_2+4,ValueZ2.int8[1]); //Write MSB
    SRAM2.writeByte(START_ADDRESS_WRITE_2+5,ValueZ2.int8[0]); // Write LSB

    
    x3.readXYZData(NbrAcc[3], XValue, YValue, ZValue);

    ByteSplit ValueX3;
    ValueX3.int16= XValue; 
    SRAM3.writeByte(START_ADDRESS_WRITE_3,ValueX3.int8[1]); //Write MSB
    SRAM3.writeByte(START_ADDRESS_WRITE_3+1,ValueX3.int8[0]); // Write LSB

    ByteSplit ValueY3;
    ValueY3.int16= YValue; 
    SRAM3.writeByte(START_ADDRESS_WRITE_3+2,ValueY3.int8[1]); //Write MSB
    SRAM3.writeByte(START_ADDRESS_WRITE_3+3,ValueY3.int8[0]); // Write LSB
    

    ByteSplit ValueZ3;
    ValueZ3.int16= ZValue; 
    SRAM3.writeByte(START_ADDRESS_WRITE_3+4,ValueZ3.int8[1]); //Write MSB
    SRAM3.writeByte(START_ADDRESS_WRITE_3+5,ValueZ3.int8[0]); // Write LSB


    x4.readXYZData(NbrAcc[4], XValue, YValue, ZValue);

    ByteSplit ValueX4;
    ValueX4.int16= XValue; 
    SRAM4.writeByte(START_ADDRESS_WRITE_4,ValueX4.int8[1]); //Write MSB
    SRAM4.writeByte(START_ADDRESS_WRITE_4+1,ValueX4.int8[0]); // Write LSB

    ByteSplit ValueY4;
    ValueY4.int16= YValue; 
    SRAM4.writeByte(START_ADDRESS_WRITE_4+2,ValueY4.int8[1]); //Write MSB
    SRAM4.writeByte(START_ADDRESS_WRITE_4+3,ValueY4.int8[0]); // Write LSB
    

    ByteSplit ValueZ4;
    ValueZ4.int16= ZValue; 
    SRAM4.writeByte(START_ADDRESS_WRITE_4+4,ValueZ4.int8[1]); //Write MSB
    SRAM4.writeByte(START_ADDRESS_WRITE_4+5,ValueZ4.int8[0]); // Write LSB

    
    x5.readXYZData(NbrAcc[5], XValue, YValue, ZValue);

    ByteSplit ValueX5;
    ValueX5.int16= XValue; 
    SRAM5.writeByte(START_ADDRESS_WRITE_5,ValueX5.int8[1]); //Write MSB
    SRAM5.writeByte(START_ADDRESS_WRITE_5+1,ValueX5.int8[0]); // Write LSB

    ByteSplit ValueY5;
    ValueY5.int16= YValue; 
    SRAM5.writeByte(START_ADDRESS_WRITE_5+2,ValueY5.int8[1]); //Write MSB
    SRAM5.writeByte(START_ADDRESS_WRITE_5+3,ValueY5.int8[0]); // Write LSB
    

    ByteSplit ValueZ5;
    ValueZ5.int16= ZValue; 
    SRAM5.writeByte(START_ADDRESS_WRITE_5+4,ValueZ5.int8[1]); //Write MSB
    SRAM5.writeByte(START_ADDRESS_WRITE_5+5,ValueZ5.int8[0]); // Write LSB

    START_ADDRESS_WRITE+=6;
    START_ADDRESS_WRITE_2+=6;
    START_ADDRESS_WRITE_3+=6;
    START_ADDRESS_WRITE_4+=6;
    START_ADDRESS_WRITE_5+=6;

    Counter_30sec++;

    if (Counter_30sec >= (100 * 30)) //200 = 1second   //Before: (640*30)) //640 = 1second
    {
      Counter_slice++;
      Counter_30sec = 0;
      //Serial.println(Counter_slice);
    }
    if (Counter_slice == slicesNbr)
    {
      for(int i=1; i<=3000*slicesNbr; i++)
      {
        
        int8_t MSB;
        int8_t LSB; 
        int16_t wd;

        // Sensor 1
        MSB=SRAM.readByte(START_ADDRESS_READ); //Read MSB
        LSB=SRAM.readByte(START_ADDRESS_READ+1); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.flush(); 
        Serial.print((wd/1024)*9.81);

        Serial.flush(); 
        Serial.print(",");   
        MSB=SRAM.readByte(START_ADDRESS_READ+2); //Read MSB
        LSB=SRAM.readByte(START_ADDRESS_READ+3); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.flush(); 
        Serial.print((wd/1024)*9.81); 

        Serial.flush(); 
        Serial.print(",");
        MSB=SRAM.readByte(START_ADDRESS_READ+4); //Read MSB
        LSB=SRAM.readByte(START_ADDRESS_READ+5); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.flush(); 
        Serial.print((wd/1024)*9.81);


        // Sensor 2
        Serial.flush(); 
        Serial.print(",");
        MSB=SRAM2.readByte(START_ADDRESS_READ_2); //Read MSB
        LSB=SRAM2.readByte(START_ADDRESS_READ_2+1); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.flush(); 
        Serial.print((wd/1024)*9.81);

        Serial.flush(); 
        Serial.print(",");   
        MSB=SRAM2.readByte(START_ADDRESS_READ_2+2); //Read MSB
        LSB=SRAM2.readByte(START_ADDRESS_READ_2+3); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.flush(); 
        Serial.print((wd/1024)*9.81); 

        Serial.flush(); 
        Serial.print(",");
        MSB=SRAM2.readByte(START_ADDRESS_READ_2+4); //Read MSB
        LSB=SRAM2.readByte(START_ADDRESS_READ_2+5); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.flush(); 
        Serial.print((wd/1024)*9.81);


        //Sensor 3
        Serial.flush(); 
        Serial.print(",");
        MSB=SRAM3.readByte(START_ADDRESS_READ_3); //Read MSB
        LSB=SRAM3.readByte(START_ADDRESS_READ_3+1); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.flush(); 
        Serial.print((wd/1024)*9.81);

        Serial.flush(); 
        Serial.print(",");   
        MSB=SRAM3.readByte(START_ADDRESS_READ_3+2); //Read MSB
        LSB=SRAM3.readByte(START_ADDRESS_READ_3+3); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.flush(); 
        Serial.print((wd/1024)*9.81); 

        Serial.flush(); 
        Serial.print(",");
        MSB=SRAM3.readByte(START_ADDRESS_READ_3+4); //Read MSB
        LSB=SRAM3.readByte(START_ADDRESS_READ_3+5); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.flush(); 
        Serial.print((wd/1024)*9.81);

        //Sensor 4
        Serial.flush(); 
        Serial.print(",");
        MSB=SRAM4.readByte(START_ADDRESS_READ_4); //Read MSB
        LSB=SRAM4.readByte(START_ADDRESS_READ_4+1); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.flush(); 
        Serial.print((wd/1024)*9.81);

        Serial.flush(); 
        Serial.print(",");   
        MSB=SRAM4.readByte(START_ADDRESS_READ_4+2); //Read MSB
        LSB=SRAM4.readByte(START_ADDRESS_READ_4+3); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.flush(); 
        Serial.print((wd/1024)*9.81); 

        Serial.flush(); 
        Serial.print(",");
        MSB=SRAM4.readByte(START_ADDRESS_READ_4+4); //Read MSB
        LSB=SRAM4.readByte(START_ADDRESS_READ_4+5); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.flush(); 
        Serial.print((wd/1024)*9.81);

        //Sensor 5
        Serial.flush(); 
        Serial.print(",");
        MSB=SRAM5.readByte(START_ADDRESS_READ_5); //Read MSB
        LSB=SRAM5.readByte(START_ADDRESS_READ_5+1); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.flush(); 
        Serial.print((wd/1024)*9.81);

        Serial.flush(); 
        Serial.print(",");   
        MSB=SRAM5.readByte(START_ADDRESS_READ_5+2); //Read MSB
        LSB=SRAM5.readByte(START_ADDRESS_READ_5+3); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.flush(); 
        Serial.print((wd/1024)*9.81); 

        Serial.flush(); 
        Serial.print(",");
        MSB=SRAM5.readByte(START_ADDRESS_READ_5+4); //Read MSB
        LSB=SRAM5.readByte(START_ADDRESS_READ_5+5); // Read LSB
        wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
        Serial.flush(); 
        Serial.println((wd/1024)*9.81);

        START_ADDRESS_READ+=6; 
        START_ADDRESS_READ_2+=6; 
        START_ADDRESS_READ_3+=6; 
        START_ADDRESS_READ_4+=6; 
        START_ADDRESS_READ_5+=6; 

      }
      processing = 0;
      //Serial.println(processing);
      Counter_slice = 0;
      Counter_30sec = 0;
      START_ADDRESS_WRITE=100;
      START_ADDRESS_READ=100;

      START_ADDRESS_WRITE_2=100;
      START_ADDRESS_READ_2=100;
      
      START_ADDRESS_WRITE_3=100;
      START_ADDRESS_READ_3=100;

      START_ADDRESS_WRITE_4=100;
      START_ADDRESS_READ_4=100;

      START_ADDRESS_WRITE_5=100;
      START_ADDRESS_READ_5=100;
      
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
