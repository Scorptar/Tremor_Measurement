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

ADXL362 x1;
ADXL362 x2;
//ADXL362 x3;
//ADXL362 x4;
//ADXL362 x5;

int16_t temp;
int16_t XValue, YValue, ZValue, Temperature;
float temp_m_s2;
//int8_t  XLValue, YLValue, ZLValue, XHValue, YHValue, ZHValue;

int NbrAcc[7]; // 7 of size because we don't use number zero and we need to let the last one empty to hold the required null character.
int8_t NbrAccCounter = 0;

int incomingByte = 0; // for incoming serial data
int8_t processing = 0; // Flag to show if we are already gathering data from accelerometers
int16_t Counter_30sec = 0; // We have 640 interrupt each seconds
int8_t Counter_slice = 0; // Counts the number of 30seconds slices. 30sec to 180sec <=> 1 to 6
int8_t slicesNbr = 0; // Contains the time asked by the user (time asked = slicesNbr * 30)

void setup(){
  
  Serial.begin(115200);
  x1.begin(10);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  x1.setFilterCL();               // GForce = 8, HALF_BW = 1/2 (100Hz), ODR = 200Hz
  x1.setNoise();                  // UltraLow noise setting
  x1.beginMeasure();              // Switch ADXL362 to measure mode 
  
  x2.begin(9);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  x2.setFilterCL();               // GForce = 8, HALF_BW = 1/2 (100Hz), ODR = 200Hz
  x2.setNoise();                  // UltraLow noise setting
  x2.beginMeasure();              // Switch ADXL362 to measure mode 
  //x3.begin(8);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  //x4.begin(7);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  //x5.begin(6);                   // Setup SPI protocol, issue device soft reset and make a reinitialisation
  
	delay(500);
  //Serial.println("Start Demo");

  //Pin assignement
  NbrAcc[1] = 10;
  NbrAcc[2] = 9;
  //NbrAcc[3] = 8;
  //NbrAcc[4] = 7;
  //NbrAcc[5] = 6;  

  cli();//stop interrupts

//set timer1 interrupt at 1000Hz = (Fe * Nbr Acc) = (200 * 5)  [Before: 640Hz = (Fe * Nbr Acc) = (128 * 5)]
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;// initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 39999;// = [(8*10^6) / (1000*1)] - 1 (must be <65536)  [Before := [(8*10^6) / (640*1)] - 1]  avant c 7999
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 bit for a prescaler of 1
  TCCR1B |= (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts
  
}

ISR(TIMER1_COMPA_vect){//timer1 interrupt

  NbrAccCounter=0;
  if (processing == 127)
  {
    NbrAccCounter=1;
  
    x1.readXYZData(NbrAcc[NbrAccCounter], XValue, YValue, ZValue);
    Serial.print("X");
    Serial.print(NbrAccCounter);
    Serial.print(":");  
    Serial.print(",");
    temp_m_s2 = XValue;
    temp_m_s2 = (temp_m_s2 / 256) * 9.81;
    Serial.print(temp_m_s2);
    
    Serial.print("Y");    // Serial.print("\tY"); //To have a clean receiving
    Serial.print(NbrAccCounter);
    Serial.print(":");
    Serial.print(","); 
    temp_m_s2 = YValue;
    temp_m_s2 = (temp_m_s2 / 256) * 9.81;
    Serial.print(temp_m_s2); 
    
    Serial.print("Z");    // Serial.print("\tZ"); //To have a clean receiving
    Serial.print(NbrAccCounter);
    Serial.print(":");
    Serial.print(","); 
    temp_m_s2 = ZValue;
    temp_m_s2 = (temp_m_s2 / 256) * 9.81;
    Serial.print(temp_m_s2); // Serial.println(ZValue); //To have a clean receiving
    
    if (NbrAccCounter >= 2){
      NbrAccCounter = 0;
      Serial.print("\n"); 
    }
    
  
    Counter_30sec++;
    
    if (Counter_30sec >= (200*30)) //1000 = 1second   //Before: (640*30)) //640 = 1second
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
      //Serial.println("END");
    }
  }
}

void loop(){
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
