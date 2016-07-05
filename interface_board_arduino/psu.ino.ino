#include <Wire.h>

uint8_t PSUstatus = 0;   //PSU status byte I
uint8_t temp = 0;   //PSU status byte II

void setup_psu() {
  // put your setup code here, to run once:
Wire.begin(10);
pinMode(11,OUTPUT);
pinMode(12,OUTPUT);
digitalWrite(12,HIGH);
digitalWrite(11,HIGH);

}

void I2C_T()
{
  Wire.write(PSUstatus);
}

void I2C_R(int bytes)
{
  temp = Wire.read();
  if(temp==11)
  {
    PSUstatus |= (1<<0);
    digitalWrite(12,LOW);
  }
  if(temp==10)
  {
    PSUstatus &= ~(1<<0);
    digitalWrite(12,HIGH);
  }
  if(temp==21)
  {
    PSUstatus |= (1<<1);
    digitalWrite(11,LOW);
  }
  if(temp==20)
  {
    PSUstatus &= ~(1<<1);
    digitalWrite(11,HIGH);
  }
}

void loop_psu() {

  Wire.onRequest(I2C_T);
  Wire.onReceive(I2C_R);
}
