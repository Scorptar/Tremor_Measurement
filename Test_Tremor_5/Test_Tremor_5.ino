#include <SPI.h>
#include <ADXL362.h>
#include <SRAM_23LC.h>

ADXL362 xl;
ADXL362 x2;
ADXL362 x3;
ADXL362 x4;
ADXL362 x5;

SRAM_23LC SRAM(&SPI, 2, SRAM_23LC1024);
SRAM_23LC SRAM2(&SPI, 4, SRAM_23LC1024);
SRAM_23LC SRAM3(&SPI, 3, SRAM_23LC1024);
SRAM_23LC SRAM4(&SPI, 5, SRAM_23LC1024);
SRAM_23LC SRAM5(&SPI, 6, SRAM_23LC1024);

int16_t XValue, YValue, ZValue;


int NbrAcc[7]; // '7' (instead of '5') of size because we don't use number zero and we need to let the last one empty to hold the required null character.
int8_t NbrAccCounter = 1;

int incomingByte = 0; // for incoming serial data
int8_t slicesNbr = 0; // Contains the time asked by the user (time asked = slicesNbr * 30)


int64_t START_ADDRESS_WRITE=6; 
int64_t START_ADDRESS_READ=6;

int64_t START_ADDRESS_WRITE_2=6; 
int64_t START_ADDRESS_READ_2=6;

int64_t START_ADDRESS_WRITE_3=6; 
int64_t START_ADDRESS_READ_3=6;

int64_t START_ADDRESS_WRITE_4=6; 
int64_t START_ADDRESS_READ_4=6;

int64_t START_ADDRESS_WRITE_5=6; 
int64_t START_ADDRESS_READ_5=6;

void setup() {

  Serial.begin(115200);

  xl.begin(A1);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  xl.setFilterCL();               // MNT : Gforce=8, HALF_BW=1/2 (50Hz), ODR=100HZ  avant :GForce = 8, HALF_BW = 1/2 (100Hz), ODR = 200Hz 
  xl.setNoise();                  // UltraLow noise setting
  xl.beginMeasure();              // Switch ADXL362 to measure mode

  x2.begin(A2);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  x2.setFilterCL();               // MNT : Gforce=8, HALF_BW=1/2 (50Hz), ODR=100HZ  avant :GForce = 8, HALF_BW = 1/2 (100Hz), ODR = 200Hz 
  x2.setNoise();                  // UltraLow noise setting
  x2.beginMeasure();              // Switch ADXL362 to measure mode

  x3.begin(A5);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  x3.setFilterCL();               // MNT : Gforce=8, HALF_BW=1/2 (50Hz), ODR=100HZ  avant :GForce = 8, HALF_BW = 1/2 (100Hz), ODR = 200Hz 
  x3.setNoise();                  // UltraLow noise setting
  x3.beginMeasure();              // Switch ADXL362 to measure mode
  
  x4.begin(A4);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  x4.setFilterCL();               // MNT : Gforce=8, HALF_BW=1/2 (50Hz), ODR=100HZ  avant :GForce = 8, HALF_BW = 1/2 (100Hz), ODR = 200Hz 
  x4.setNoise();                  // UltraLow noise setting
  x4.beginMeasure();              // Switch ADXL362 to measure mode

  x5.begin(A3);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
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
  NbrAcc[1] = A1;
  NbrAcc[2] = A2;
  NbrAcc[3] = A5;
  NbrAcc[4] = A4;
  NbrAcc[5] = A3;

}

void loop() 
{

  incomingByte = Serial.read();
  if (incomingByte >= 65 and incomingByte <= 70)
    {
      slicesNbr = incomingByte - 64; // ASCII de A(30s) = 65, F(180s) = 70.
      // data processing started
      for(int i=1; i<=3000*slicesNbr; i++)
        {
          WriteSRAM(); 
          delay(10);        
        }
      for(int i=1; i<=3000*slicesNbr; i++)
        {
           ReadSRAM(); 
        }

        START_ADDRESS_WRITE=6;
        START_ADDRESS_READ=6;
  
        START_ADDRESS_WRITE_2=6;
        START_ADDRESS_READ_2=6;
        
        START_ADDRESS_WRITE_3=6;
        START_ADDRESS_READ_3=6;
  
        START_ADDRESS_WRITE_4=6;
        START_ADDRESS_READ_4=6;
  
        START_ADDRESS_WRITE_5=6;
        START_ADDRESS_READ_5=6;
    }
}

void WriteSRAM()
{
  union ByteSplit
    {
      int16_t int16; 
      int8_t int8[2];
     };

  
    xl.readXYZData(NbrAcc[1], XValue, YValue, ZValue);

    ByteSplit ValueX1;
    ValueX1.int16= XValue; 
    SRAM.writeByte(START_ADDRESS_WRITE,ValueX1.int8[1]); //Write MSB
    SRAM.writeByte(START_ADDRESS_WRITE+1,ValueX1.int8[0]); // Write LSB
    
    ByteSplit ValueY1;
    ValueY1.int16= YValue; 
    SRAM.writeByte(START_ADDRESS_WRITE+2,ValueY1.int8[1]); //Write MSB
    SRAM.writeByte(START_ADDRESS_WRITE+3,ValueY1.int8[0]); // Write LSB
    
    ByteSplit ValueZ1;
    ValueZ1.int16= ZValue; 
    SRAM.writeByte(START_ADDRESS_WRITE+4,ValueZ1.int8[1]); //Write MSB
    SRAM.writeByte(START_ADDRESS_WRITE+5,ValueZ1.int8[0]); // Write LSB
     
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

}

void ReadSRAM()
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
