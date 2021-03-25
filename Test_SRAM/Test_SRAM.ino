#include <SRAM_23LC.h>
#include <SPI.h>


SRAM_23LC SRAM(&SPI, A1, SRAM_23LC1024);

int16_t raw_data = 2520;                                //same type as Xdata,Ydata from the accelerometers
//char buf_in[10];                                        //buffer for SRAM write
float data = (raw_data / 1024) * 9.81;
void setup() 
{
  Serial.begin(9600);
  SRAM.begin(); 

}

void loop() 
{
    /*itoa(raw_data,buf_in,10);                           //place raw data in the buffer
    SRAM.writeBlock(250,sizeof(buf_in),buf_in);         //write the int_16 data stored in the buffer into the SRAM
    SRAM.writeByte(252,2500); 
    delay(1000);
    char buf_out [sizeof(buf_in)];                      //buffer for SRAM read 
    SRAM.readBlock(250,2,buf_out);        //read the int16 data stored in the SRAM
    Serial.println(SRAM.readByte(252));
    Serial.println((atoi(buf_out)/1024)*9.81);          // Convert char array to integer and do the maths 
    Serial.println(data);                               //serial print should be the same as data*/

    union ByteSplit
    {
      int16_t int16; 
      int8_t int8[2];
     };

    ByteSplit Value;
    Value.int16= raw_data; 
    Serial.println(Value.int16);
    delay(1000);
    Serial.println(Value.int8[0]);//LSB
    delay(1000);
    Serial.println(Value.int8[1]); //MSB
    delay(1000);
    int16_t wd = ((int16_t)Value.int8[1] << 8) | (Value.int8[0] & 0xFF);
    Serial.println(wd);
    delay(1000);

    /*SRAM.writeByte(250,Value.int8[0]);         //write the int_16 data stored in the buffer into the SRAM
    SRAM.writeByte(251,Value.int8[1]); 
    delay(1000);
    char buf_out [sizeof(buf_in)];                      //buffer for SRAM read 
    SRAM.readBlock(250,2,buf_out);        //read the int16 data stored in the SRAM
    Serial.println(SRAM.readByte(252));
    Serial.println((atoi(buf_out)/1024)*9.81);          // Convert char array to integer and do the maths 
    Serial.println(data);       */                        //serial print should be the same as data

}
