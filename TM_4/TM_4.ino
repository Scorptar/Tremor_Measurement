/***************************
         INCLUDES
***************************/

#include <SPI.h>
#include <ADXL362.h>
#include <SRAM_23LC.h>

/***************************
     ADXL362 - INIT
***************************/

ADXL362 xl;
ADXL362 x2;
ADXL362 x3;
ADXL362 x4;


/***************************
     SRAM - INIT
***************************/

SRAM_23LC SRAM(&SPI, 2, SRAM_23LC1024);
SRAM_23LC SRAM2(&SPI, 4, SRAM_23LC1024);
SRAM_23LC SRAM3(&SPI, 3, SRAM_23LC1024);
SRAM_23LC SRAM4(&SPI, 5, SRAM_23LC1024);

/***************************
    Variables globales
***************************/

int16_t XValue, YValue, ZValue;

int NbrAcc[7];                                    // '7' (instead of '5') of size because we don't use number zero and we need to let the last one empty to hold the required null character.
int8_t NbrAccCounter = 1;

int incomingByte = 0;                             // for incoming serial data
int8_t slicesNbr = 0;                             // Contains the time asked by the user (time asked = slicesNbr * 30)

// Initialisation des adresses pour l'écriture et la lecture de la SRAM
int64_t START_ADDRESS_WRITE=6; 
int64_t START_ADDRESS_READ=6;

int64_t START_ADDRESS_WRITE_2=6; 
int64_t START_ADDRESS_READ_2=6;

int64_t START_ADDRESS_WRITE_3=6; 
int64_t START_ADDRESS_READ_3=6;

int64_t START_ADDRESS_WRITE_4=6; 
int64_t START_ADDRESS_READ_4=6;


/***************************
         SETUP
***************************/

void setup() {

  Serial.begin(1000000);          //1Mhz serial (max)

  // Setup ADXL362

  // Capteur 1
  xl.begin(A1);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  xl.setFilterCL();               // MNT : Gforce=8, HALF_BW=1/2 (50Hz), ODR=100HZ  avant :GForce = 8, HALF_BW = 1/2 (100Hz), ODR = 200Hz 
  xl.setNoise();                  // UltraLow noise setting
  xl.beginMeasure();              // Switch ADXL362 to measure mode

  // Capteur 2
  x2.begin(A2);                   
  x2.setFilterCL();               
  x2.setNoise();                  
  x2.beginMeasure();              

  // Capteur 3
  x3.begin(A5);                   
  x3.setFilterCL();               
  x3.setNoise();                 
  x3.beginMeasure();             

  // Capteur 4
  x4.begin(A4);                  
  x4.setFilterCL();               
  x4.setNoise();                  
  x4.beginMeasure();              
            

  // Setup SRAM
  
  SRAM.begin();
  SRAM2.begin();
  SRAM3.begin();
  SRAM4.begin();
 

  delay(500);

  // Pin assignement
  NbrAcc[1] = A1;
  NbrAcc[2] = A2;
  NbrAcc[3] = A5;
  NbrAcc[4] = A4;


}

/***************************
         LOOP
***************************/

void loop() 
{

  incomingByte = Serial.read();                                                 // Lecture des données Serial
  if (incomingByte >= 65 and incomingByte <= 70)
    {
      slicesNbr = incomingByte - 64; // ASCII de A(30s) = 65, F(180s) = 70.     // Durée d'un test 
      // data processing started
      for(int i=1; i<=3000*slicesNbr; i++)
        {
          WriteSRAM();                                                          // Ecriture dans les SRAMs
          delay(10);        
        }
      for(int i=1; i<=3000*slicesNbr; i++)
        {
           ReadSRAM();                                                          // Lecture des données contenues dans les SRAMs
        }

        // Réinitialisation des adresses des SRAMs
        START_ADDRESS_WRITE=6;
        START_ADDRESS_READ=6;
  
        START_ADDRESS_WRITE_2=6;
        START_ADDRESS_READ_2=6;
        
        START_ADDRESS_WRITE_3=6;
        START_ADDRESS_READ_3=6;
  
        START_ADDRESS_WRITE_4=6;
        START_ADDRESS_READ_4=6;

    }
}

/***************************
         WRITESRAM
***************************/

void WriteSRAM()
{
  union ByteSplit                                                       // union class
    {
      int16_t int16;                                                    // 2 octets
      int8_t int8[2];                                                   // 1 octet
     };

    // Capteur 1
    xl.readXYZData(NbrAcc[1], XValue, YValue, ZValue);                  // Lecture des données provenant du capteur 1 et stockage des données dans les variables correspondantes (XValue, YValue, ZValue)

    ByteSplit ValueX1;                                                  // Déclaration de la variable ValueX1 de type ByteSplit
    ValueX1.int16= XValue;                                              // Affectation de la valeur de XValue à ValueX1
    SRAM.writeByte(START_ADDRESS_WRITE,ValueX1.int8[1]);                // Write MSB
    SRAM.writeByte(START_ADDRESS_WRITE+1,ValueX1.int8[0]);              // Write LSB
    
    ByteSplit ValueY1;
    ValueY1.int16= YValue; 
    SRAM.writeByte(START_ADDRESS_WRITE+2,ValueY1.int8[1]); 
    SRAM.writeByte(START_ADDRESS_WRITE+3,ValueY1.int8[0]); 
    
    ByteSplit ValueZ1;
    ValueZ1.int16= ZValue; 
    SRAM.writeByte(START_ADDRESS_WRITE+4,ValueZ1.int8[1]); 
    SRAM.writeByte(START_ADDRESS_WRITE+5,ValueZ1.int8[0]); 
    
    // Capteur 2
    x2.readXYZData(NbrAcc[2], XValue, YValue, ZValue);

    ByteSplit ValueX2;
    ValueX2.int16= XValue; 
    SRAM2.writeByte(START_ADDRESS_WRITE_2,ValueX2.int8[1]); 
    SRAM2.writeByte(START_ADDRESS_WRITE_2+1,ValueX2.int8[0]); 

    ByteSplit ValueY2;
    ValueY2.int16= YValue; 
    SRAM2.writeByte(START_ADDRESS_WRITE_2+2,ValueY2.int8[1]); 
    SRAM2.writeByte(START_ADDRESS_WRITE_2+3,ValueY2.int8[0]); 
    

    ByteSplit ValueZ2;
    ValueZ2.int16= ZValue; 
    SRAM2.writeByte(START_ADDRESS_WRITE_2+4,ValueZ2.int8[1]); 
    SRAM2.writeByte(START_ADDRESS_WRITE_2+5,ValueZ2.int8[0]); 

    // Capteur 3
    x3.readXYZData(NbrAcc[3], XValue, YValue, ZValue);

    ByteSplit ValueX3;
    ValueX3.int16= XValue; 
    SRAM3.writeByte(START_ADDRESS_WRITE_3,ValueX3.int8[1]); 
    SRAM3.writeByte(START_ADDRESS_WRITE_3+1,ValueX3.int8[0]); 

    ByteSplit ValueY3;
    ValueY3.int16= YValue; 
    SRAM3.writeByte(START_ADDRESS_WRITE_3+2,ValueY3.int8[1]);
    SRAM3.writeByte(START_ADDRESS_WRITE_3+3,ValueY3.int8[0]); 
    

    ByteSplit ValueZ3;
    ValueZ3.int16= ZValue; 
    SRAM3.writeByte(START_ADDRESS_WRITE_3+4,ValueZ3.int8[1]); 
    SRAM3.writeByte(START_ADDRESS_WRITE_3+5,ValueZ3.int8[0]);

    // Capteur 4
    x4.readXYZData(NbrAcc[4], XValue, YValue, ZValue);

    ByteSplit ValueX4;
    ValueX4.int16= XValue; 
    SRAM4.writeByte(START_ADDRESS_WRITE_4,ValueX4.int8[1]); 
    SRAM4.writeByte(START_ADDRESS_WRITE_4+1,ValueX4.int8[0]); 

    ByteSplit ValueY4;
    ValueY4.int16= YValue; 
    SRAM4.writeByte(START_ADDRESS_WRITE_4+2,ValueY4.int8[1]); 
    SRAM4.writeByte(START_ADDRESS_WRITE_4+3,ValueY4.int8[0]); 
    

    ByteSplit ValueZ4;
    ValueZ4.int16= ZValue; 
    SRAM4.writeByte(START_ADDRESS_WRITE_4+4,ValueZ4.int8[1]); 
    SRAM4.writeByte(START_ADDRESS_WRITE_4+5,ValueZ4.int8[0]); 


    // Incrémentation des adresses 
    START_ADDRESS_WRITE+=6;
    START_ADDRESS_WRITE_2+=6;
    START_ADDRESS_WRITE_3+=6;
    START_ADDRESS_WRITE_4+=6;

}

/***************************
         READSRAM
***************************/

void ReadSRAM()
{
  
  // Déclaration des variables 
  int8_t MSB;
  int8_t LSB; 
  int16_t wd;

  // Capteur 1
  MSB=SRAM.readByte(START_ADDRESS_READ);                  // Read MSB
  LSB=SRAM.readByte(START_ADDRESS_READ+1);                // Read LSB
  wd = ((int16_t)MSB << 8) | (LSB & 0xFF);                // Data conversion 1 octet to 2 octets
  wd = (wd/1024)*9.81*1000;                               // Convert LSB/g => mm/s²
  Serial.flush();                                         // Waits for the transmission of outgoing serial data to complete
  Serial.print(wd);                                       // Data printed

  Serial.flush(); 
  Serial.print(",");   
  MSB=SRAM.readByte(START_ADDRESS_READ+2); 
  LSB=SRAM.readByte(START_ADDRESS_READ+3); 
  wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
  wd = (wd/1024)*9.81*1000; 
  Serial.flush(); 
  Serial.print(wd); 

  Serial.flush(); 
  Serial.print(",");
  MSB=SRAM.readByte(START_ADDRESS_READ+4); 
  LSB=SRAM.readByte(START_ADDRESS_READ+5); 
  wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
  wd = (wd/1024)*9.81*1000; 
  Serial.flush(); 
  Serial.print(wd);

  // Capteur 2
  Serial.flush(); 
  Serial.print(",");
  MSB=SRAM2.readByte(START_ADDRESS_READ_2);
  LSB=SRAM2.readByte(START_ADDRESS_READ_2+1); 
  wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
  wd = (wd/1024)*9.81*1000; 
  Serial.flush(); 
  Serial.print(wd);

  Serial.flush(); 
  Serial.print(",");   
  MSB=SRAM2.readByte(START_ADDRESS_READ_2+2); 
  LSB=SRAM2.readByte(START_ADDRESS_READ_2+3); 
  wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
  wd = (wd/1024)*9.81*1000;
  Serial.flush(); 
  Serial.print(wd); 

  Serial.flush(); 
  Serial.print(",");
  MSB=SRAM2.readByte(START_ADDRESS_READ_2+4); 
  LSB=SRAM2.readByte(START_ADDRESS_READ_2+5);
  wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
  wd = (wd/1024)*9.81*1000; 
  Serial.flush(); 
  Serial.print(wd);

  // Capteur 3
  Serial.flush(); 
  Serial.print(",");
  MSB=SRAM3.readByte(START_ADDRESS_READ_3); 
  LSB=SRAM3.readByte(START_ADDRESS_READ_3+1); 
  wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
  wd = (wd/1024)*9.81*1000; 
  Serial.flush(); 
  Serial.print(wd);

  Serial.flush(); 
  Serial.print(",");   
  MSB=SRAM3.readByte(START_ADDRESS_READ_3+2); 
  LSB=SRAM3.readByte(START_ADDRESS_READ_3+3); 
  wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
  wd = (wd/1024)*9.81*1000;
  Serial.flush(); 
  Serial.print(wd); 

  Serial.flush(); 
  Serial.print(",");
  MSB=SRAM3.readByte(START_ADDRESS_READ_3+4); 
  LSB=SRAM3.readByte(START_ADDRESS_READ_3+5);
  wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
  wd = (wd/1024)*9.81*1000;
  Serial.flush(); 
  Serial.print(wd);

  // Capteur 4
  Serial.flush(); 
  Serial.print(",");
  MSB=SRAM4.readByte(START_ADDRESS_READ_4); 
  LSB=SRAM4.readByte(START_ADDRESS_READ_4+1); 
  wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
  wd = (wd/1024)*9.81*1000; 
  Serial.flush(); 
  Serial.print(wd);

  Serial.flush(); 
  Serial.print(",");   
  MSB=SRAM4.readByte(START_ADDRESS_READ_4+2); 
  LSB=SRAM4.readByte(START_ADDRESS_READ_4+3); 
  wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
  wd = (wd/1024)*9.81*1000; 
  Serial.flush(); 
  Serial.print(wd); 

  Serial.flush(); 
  Serial.print(",");
  MSB=SRAM4.readByte(START_ADDRESS_READ_4+4); 
  LSB=SRAM4.readByte(START_ADDRESS_READ_4+5); 
  wd = ((int16_t)MSB << 8) | (LSB & 0xFF);
  wd = (wd/1024)*9.81*1000; 
  Serial.flush(); 
  Serial.println(wd);


  //Incrémentation des adresses 
  START_ADDRESS_READ+=6; 
  START_ADDRESS_READ_2+=6; 
  START_ADDRESS_READ_3+=6; 
  START_ADDRESS_READ_4+=6; 
}
