
#include <Servo.h>
#include <Wire.h>

//HyCOMET-1 Interface Board
//Arduino Nano

#define serial_speed 9600     //serial interface (UART) baud rate [default: 9k6 Baud, 8 data bits, no parity bit, one stop bit]
#define TIMEOUT 250           //serial interface timeout in ms (keep low for a quicker response)
#define PSU_EC_ADR 10         //I2C adress of PSU in electronic compartment
#define PSU_NC_ADR 11         //I2C adress of PSU in nose cone

//How many ADC-samples are used to calculate one average reading [4,8,16,32]
#define AVG 8

//calibration data for temperature sensors
#define CAL_MULT_1 1
#define CAL_MULT_2 0.978
#define CAL_OFFSET -3

//calibration data for pressure sensor (1bar == 10mV => 1bit == 0,93bar)
#define CAL_PRESSURE 9.3


//servo motor controls
//servo timings for 90° turn!!
//programming interface needed to change servo to 180°!
#define OPEN 1100        //pulse lenght in µs for open valve    
#define CLOSED 1900      //pulse lenght in µs for closed valve


int N2O_lock = 1;   //prohibits valve1 from opening until user disables lock via serial command
int warning = 0;    //warning flag for accidential fire signals
Servo valve1;
Servo valve2;


// ####################################################
// setup routine. prepare µC for operation
// ####################################################
void setup() {

  pinMode(8, OUTPUT);  //V1 servo control unlocked LED
  pinMode(7, OUTPUT);  //error LED
  pinMode(10, OUTPUT); //PSU active LED
  pinMode(11, OUTPUT); //temp sensor ok LED
  pinMode(12, OUTPUT); //self-test OK LED
  pinMode(9, OUTPUT); //heater

  pinMode(5, OUTPUT);   //servo PWM
  pinMode(6, OUTPUT);   //servo PWM

  pinMode(2, INPUT_PULLUP); //fire line B input

  Serial.begin(serial_speed);     //start up serial interface
  Serial.setTimeout(TIMEOUT);     //serial timeout

  Wire.begin();                   //start up I2C bus as busmaster

  analogReference(INTERNAL);      //use internal 1.1V reference for ADC
  read_temp(1, 32);               //prime ADC for use after reference change

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

  for (i = 1; i <= avg; i++)
  {
    temp_read = temp_read + analogRead(channel);
    if (i == avg)
    {
      if (channel == 1)
      {
        temp = (temp_read / avg / 1.87 * CAL_MULT_1) - 273 + CAL_OFFSET;
      } else if (channel == 2)
      {
        temp = (temp_read / avg / 1.87 * CAL_MULT_2) - 273 + CAL_OFFSET;
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

  for (i = 1; i <= avg; i++)
  {
    pressure_read = pressure_read + analogRead(3);
    if (i == avg)
    {
      pressure = ((pressure_read / avg) - 93) / CAL_PRESSURE;
      pressure_read = 0;
    }
  }
  return pressure;
}

// ################################################
// this function handles the user interface
// check for new serial data and execute commands
// ################################################

void command_line_interface()
{
  String incomingData;

  int temp1 = 0;
  int temp2 = 0;
  int pressure = 0;


  if (Serial.available())   //is there any data available?
  {
    incomingData = Serial.readString(); //read data into this string

    //compare with known commands
    //##########################################################################################
    if (!(incomingData.compareTo("help")))  // print help file
    {

      print_command_list();

      //##########################################################################################
    } else if (incomingData.startsWith("V1")) //valve control (debug mode only)
    {
      if (incomingData.endsWith(":O"))
      {
        valve1.writeMicroseconds(OPEN);
        Serial.println("V1 open");
      }
      else if (incomingData.endsWith(":C"))
      {
        valve1.writeMicroseconds(CLOSED);
        Serial.println("V1 closed");
      } else
      {
        Serial.println("unknown command");
      }
    } else if (incomingData.startsWith("V2")) //valve control (debug mode only)
    {
      if (incomingData.endsWith(":O"))
      {
        valve2.writeMicroseconds(OPEN);
        Serial.println("V2 open");
      }
      else if (incomingData.endsWith(":C"))
      {
        valve2.writeMicroseconds(CLOSED);
        Serial.println("V2 closed");
      } else
      {
        Serial.println("unknown command");
      }

      //##########################################################################################
    } else if (incomingData.startsWith("PSU")) //PSU control commands
    {
      if (incomingData.endsWith("?"))
      {
        I2C_read();
      } else if (incomingData.endsWith("CH1:1"))
      {
        I2C_write(11);
        I2C_read();
      } else if (incomingData.endsWith("CH1:0"))
      {
        I2C_write(10);
        I2C_read();
      } else if (incomingData.endsWith("CH2:1"))
      {
        I2C_write(21);
        I2C_read();
      } else if (incomingData.endsWith("CH2:0"))
      {
        I2C_write(20);
        I2C_read();
      } else
      {
        Serial.println("unknown command");
      }

      //##########################################################################################
    } else if (incomingData.startsWith("N2O")) //oxidator valve commands
    {
      if (incomingData.endsWith(":U"))
      {
        if (self_test(1))
        {
          Serial.println("self test found errors!");
        }
        Serial.println("Do you REALLY want to unlock the oxidator valve?");
        if (get_confirmation())
        {
          N2O_lock = 0;
          Serial.println("oxidator valve unlocked!");
        } else {
          Serial.println("sequence aborted, oxidator valve remains locked!");
        }
      } else if (incomingData.endsWith(":L"))
      {
        if (!N2O_lock)
        {
          N2O_lock = 1;
          Serial.println("oxidator valve locked!");
        } else {
          Serial.println("oxidator valve is already locked");
        }
      } else if (incomingData.endsWith("?"))
      {
        if (N2O_lock)
        {
          Serial.println("oxidator valve is LOCKED");
        } else {
          Serial.println("oxidator valve is UNLOCKED");
        }
      } else
      {
        Serial.println("unknown command");
      }


      //##########################################################################################
    } else if (incomingData.endsWith("?")) { // these commands are requests. they return data

      if (!incomingData.compareTo("temp?"))
      {
        temp1 = read_temp(1, AVG);
        temp2 = read_temp(2, AVG);
        Serial.print("temp.sensor 1: ");
        Serial.print(temp1, DEC);
        Serial.println(" degC");
        Serial.print("temp.sensor 2: ");
        Serial.print(temp2, DEC);
        Serial.println(" degC");

      } else if (!incomingData.compareTo("press?"))
      {
        pressure = read_pressure(AVG);
        Serial.print("N2O pressure: ");
        Serial.print(pressure, DEC);
        Serial.println(" bar");

      } else if (!incomingData.compareTo("status?"))
      {

        self_test(0);
        if (N2O_lock)
        {
          Serial.println("oxidator valve is LOCKED");
        } else {
          Serial.println("oxidator valve is UNLOCKED");
        }
      } else {
        Serial.println("unknown command");
      }


      //##########################################################################################
    } else
    {
      Serial.print("unknown command: ");
      Serial.println(incomingData);
    }
  }
}


void I2C_read()
{
  Wire.requestFrom(PSU_EC_ADR, 1);    // request 1 byte from PSU

  while (Wire.available())   // slave may send less than requested
  {
    int c = Wire.read();    // receive a byte
    Serial.print("PSU Channel 1: ");
    if (c & (1 << 0))
    {
      Serial.println("on");
    } else
    {
      Serial.println("off");
    }
    Serial.print("PSU Channel 2: ");
    if (c & (1 << 1))
    {
      Serial.println("on");
    } else
    {
      Serial.println("off");
    }
    if ((c & (1 << 1)) || (c & (1 << 0)))
    {
      digitalWrite(10, HIGH);
    } else {
      digitalWrite(10, LOW);
    }
  }

}

void I2C_write(int data)
{
  Wire.beginTransmission(PSU_EC_ADR);
  Wire.write(data);
  Wire.endTransmission();
}

// #################################################
// get confirmation from user. important on mission
// critical functions like unlocking the oxidator valve
// #################################################
int get_confirmation()
{
  String confirmation;
  Serial.println("[ Y / N ]");
  while (!Serial.available())
  {
    //need to add some kind of timeout to prevent endless loop!
  }
  confirmation =  Serial.readString();

  if (confirmation.equalsIgnoreCase("Y"))
  {
    return 1;
  } else {
    return 0;
  }
}


// #################################################
// print a list with all known commands
// #################################################
void print_command_list()
{
  Serial.println("command list: \n");

  Serial.println("help          list all known commands");
  Serial.println("temp?         read temperature");
  Serial.println("press?        read pressure");
  Serial.println("status?       read all status, including self test\n");
  Serial.println("N2O:[arg]     lock/unlock N2O valve control [L/U/?]");
  Serial.println("V1:[arg]      open/close V1 [O/C] DEBUG MODE ONLY!!");
  Serial.println("V2:[arg]      open/close V2 [O/C] DEBUG MODE ONLY!!");
  Serial.println("PSU:[arg]     control EC PSU [CH1:1/0, CH2:1/0, ?]");
}


// ####################################################
// main loop
// ####################################################
void loop() {

  command_line_interface();


  if (!N2O_lock && !digitalRead(8))
  {
    digitalWrite(8, HIGH);
  } else if (N2O_lock && digitalRead(8))
  {
    digitalWrite(8, LOW);
  }

  if (!N2O_lock && !digitalRead(2))
  {
    //open oxidator valve
    valve1.writeMicroseconds(OPEN);
  } else if (!digitalRead(2) && !warning && N2O_lock)
  {
    //Serial.println("\n#######");
    Serial.println("WARNING");
    Serial.println("Fire Line B signal recieved. \nHOWEVER, oxidator valve is still LOCKED");
    //Serial.println("#######\n");
    warning = 1;
  } else if (digitalRead(2))
  {
    warning = 0;
  }

}

