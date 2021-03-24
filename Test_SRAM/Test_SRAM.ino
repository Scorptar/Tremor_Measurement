#include <SRAM_23LC.h>
#include <SPI.h>


SRAM_23LC SRAM(&SPI, A1, SRAM_23LC1024);

void setup() 
{
  Serial.begin(9600);
  SRAM.begin(); 

}

void loop() 
{
    SRAM.writeByte(250, 2);
    
    delay(5000); 
    uint8_t byte = SRAM.readByte(250);
    Serial.println(byte); 
    Serial.println("Done"); 
}
