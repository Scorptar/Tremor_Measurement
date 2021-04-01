#include <SPI.h>
#include <ADXL362.h>


ADXL362 x1;
ADXL362 x2;
ADXL362 x3;
ADXL362 x4;
ADXL362 x5;

int16_t temp;
int16_t Xvalue, Yvalue, Zvalue, Temperature;
float temp_m_s2;


int NbrAcc[7]; // '7' (instead of '5') of size because we don't use number zero and we need to let the last one empty to hold the required null character.

int16_t Counter_30sec = 0; // We have 640 interrupt each seconds
int8_t Counter_slice = 0; // Counts the number of 30seconds slices. 30sec to 180sec <=> 1 to 6
int8_t slicesNbr = 0; // Contains the time asked by the user (time asked = slicesNbr * 30)

void setup() 
{
  Serial.begin(115200);

  x1.begin(9);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  x1.setFilterCL();               // MNT : Gforce=8, HALF_BW=1/2 (50Hz), ODR=100HZ  avant :GForce = 8, HALF_BW = 1/2 (100Hz), ODR = 200Hz 
  x1.setNoise();                  // UltraLow noise setting
  x1.beginMeasure();              // Switch ADXL362 to measure mode
  x1.checkAllControlRegs();

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


  delay(500);
  Serial.println("Start Demo");

  //Pin assignement
  NbrAcc[1] = 9;
  NbrAcc[2] = 8;
  NbrAcc[3] = 7;
  NbrAcc[4] = 6;
  NbrAcc[5] = 5;


}

void loop() 
{

    x1.readXYZData(NbrAcc[1], Xvalue, Yvalue, Zvalue);
    Serial.print("X1value: "); 
    Serial.print(Xvalue); 
    Serial.print(" Y1value: "); 
    Serial.print(Yvalue); 
    Serial.print(" Z1value: "); 
    Serial.println(Zvalue); 
    Temperature= x1.readTemp(); 
    Serial.print(" Temperature 1: "); 
    Serial.println(Temperature); 
    delay(1000); 
    
    /*x2.readXYZData(NbrAcc[2], Xvalue, Yvalue, Zvalue);
    Serial.print("X2value: "); 
    Serial.print((Xvalue/1024)*9.81); 
    Serial.print("  Y2value: "); 
    Serial.print((Yvalue/1024)*9.81); 
    Serial.print("  Z2value: "); 
    Serial.println((Zvalue/1024)*9.81); 
    
    x3.readXYZData(NbrAcc[3], Xvalue, Yvalue, Zvalue);
    Serial.print("X3value: "); 
    Serial.print((Xvalue/1024)*9.81); 
    Serial.print("  Y3value: "); 
    Serial.print((Yvalue/1024)*9.81); 
    Serial.print("  Z3value: "); 
    Serial.println((Zvalue/1024)*9.81); 
    
    x4.readXYZData(NbrAcc[4], Xvalue, Yvalue, Zvalue);
    Serial.print("X4value: "); 
    Serial.print((Xvalue/1024)*9.81); 
    Serial.print("  Y4value: "); 
    Serial.print((Yvalue/1024)*9.81); 
    Serial.print("  Z4value: "); 
    Serial.println((Zvalue/1024)*9.81);
   
    x5.readXYZData(NbrAcc[5], Xvalue, Yvalue, Zvalue);
    Serial.print("X5value: "); 
    Serial.print((Xvalue/1024)*9.81);  
    Serial.print("  Y5value: "); 
    Serial.print((Yvalue/1024)*9.81); 
    Serial.print("  Z5value: "); 
    Serial.println((Zvalue/1024)*9.81); */
}
