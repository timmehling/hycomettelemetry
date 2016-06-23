#include "config.h"
#include <Servo.h>

//HyCOMET-1 Interface Board
//Arduino Nano


int N2O_lock = 1;   //prohibits valve1 from opening until user disables lock via serial command
int warning = 0;    //warning flag for accidential fire signals 
Servo valve1;
Servo valve2;


// ####################################################
// setup routine. prepare µC for operation
// ####################################################
void setup() {

    pinMode(8,OUTPUT);   //V1 servo control unlocked LED
    pinMode(7,OUTPUT);   //error LED
    pinMode(10,OUTPUT);  //PSU active LED
    pinMode(11,OUTPUT);  //temp sensor ok LED
    pinMode(12,OUTPUT);  //self-test OK LED
    pinMode(9,OUTPUT);  //heater

    pinMode(5,OUTPUT);    //servo PWM
    pinMode(6,OUTPUT);    //servo PWM

    pinMode(2,INPUT_PULLUP);  //fire line B input

    Serial.begin(serial_speed);     //start up serial interface
    Serial.setTimeout(TIMEOUT);     //serial timeout
    analogReference(INTERNAL);      //use internal 1.1V reference for ADC 
    read_temp(1,32);                //prime ADC for use after reference change

    Serial.println("HyCOMET-1 serial interface");
    Serial.println("type 'help' for a list of commands\n");
    self_test(0);
    valve1.attach(6);       //Servo for valve 1
    valve2.attach(5);       //Servo for valve 2

    valve1.writeMicroseconds(CLOSED);
    valve2.writeMicroseconds(CLOSED);
}



// ####################################################
// read temperature sensor [sensor, number of averages]
// ####################################################
int16_t read_temp(int channel, int avg) {
  
          int i = 0;
          int32_t temp_read = 0;
          int16_t temp = 0;
  
          for(i=1;i<=avg;i++)
        {
            temp_read = temp_read + analogRead(channel);
            if(i==avg)
            {
              if(channel==1)
              {
                temp = (temp_read/avg/1.87*CAL_MULT_1)-273+CAL_OFFSET;
              }else if (channel==2)
              {
                temp = (temp_read/avg/1.87*CAL_MULT_2)-273+CAL_OFFSET;
              }
              temp_read = 0;  
            }
        }
        return temp;
}

// ####################################################
// read pressure sensor [number of averages]
// ####################################################
int16_t read_pressure(int avg) {
  
          int i = 0;
          int32_t pressure_read = 0;
          int16_t pressure = 0;
  
          for(i=1;i<=avg;i++)
        {
            pressure_read = pressure_read + analogRead(3);
            if(i==avg)
            {
              pressure = ((pressure_read/avg)-93)/CAL_PRESSURE;
              pressure_read = 0;  
            }
        }
        return pressure;
}



// ####################################################
// main loop
// ####################################################
void loop() {

  command_line_interface(); 

  
  if(!N2O_lock && !digitalRead(2))  
  {
    //open oxidator valve
    valve1.writeMicroseconds(OPEN);
  }else if(!digitalRead(2)&&!warning&&N2O_lock)
  {
    Serial.println("\n#######");
    Serial.println("WARNING");
    Serial.println("Fire Line B signal recieved. \nHOWEVER, oxidator valve is still LOCKED");
    Serial.println("#######\n");
    warning = 1;    
  }else if(digitalRead(2))
  {
    warning = 0;
  }

}


