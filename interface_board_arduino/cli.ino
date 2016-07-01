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


  if(Serial.available())    //is there any data available?
  {
    incomingData = Serial.readString(); //read data into this string

    //compare with known commands
    //##########################################################################################
    if(!(incomingData.compareTo("help")))   // print help file
    {
      
      print_command_list();  

    //##########################################################################################
    }else if(incomingData.startsWith("V1"))  //valve control (debug mode only)
        {
              if(incomingData.endsWith(":O"))
              {
                valve1.writeMicroseconds(OPEN);
                Serial.println("V1 open");
              }
              else if(incomingData.endsWith(":C"))
              {
                valve1.writeMicroseconds(CLOSED);
                Serial.println("V1 closed");
              }else
              {
                Serial.println("unknown command");
              }
    }else if(incomingData.startsWith("V2"))  //valve control (debug mode only)
        {
              if(incomingData.endsWith(":O"))
              {
                valve2.writeMicroseconds(OPEN);
                Serial.println("V2 open");
              }
              else if(incomingData.endsWith(":C"))
              {
                valve2.writeMicroseconds(CLOSED);
                Serial.println("V2 closed");
              }else
              {
                Serial.println("unknown command");
              }

    //##########################################################################################
    }else if(incomingData.startsWith("PSU"))  //PSU control commands
        {
              if(incomingData.endsWith("?"))
              {
                I2C_read();
              }else
              if(incomingData.endsWith("CH1:1"))
              {
                I2C_write(11);
                I2C_read();
              }else
              if(incomingData.endsWith("CH1:0"))
              {
                I2C_write(10);
                I2C_read();
              }else
              if(incomingData.endsWith("CH2:1"))
              {
                I2C_write(21);
                I2C_read();
              }else
              if(incomingData.endsWith("CH2:0"))
              {
                I2C_write(20);
                I2C_read();
              }else
              {
                Serial.println("unknown command");
              }

    //##########################################################################################
    }else if(incomingData.startsWith("N2O"))  //oxidator valve commands
        {
              if(incomingData.endsWith(":U"))
              {
                if(self_test(1))
                {
                  Serial.println("self test found errors!");
                }
                Serial.println("Do you REALLY want to unlock the oxidator valve?");
                  if(get_confirmation())
                  {
                    N2O_lock = 0;
                    Serial.println("oxidator valve unlocked!");
                  }else{
                    Serial.println("sequence aborted, oxidator valve remains locked!");
                  }
              }else
              if(incomingData.endsWith(":L"))
              {
                if(!N2O_lock)
                {
                  N2O_lock = 1;
                  Serial.println("oxidator valve locked!");
                }else{
                  Serial.println("oxidator valve is already locked");
                }
              }else
              if(incomingData.endsWith("?"))
              {
                if(N2O_lock)
                {
                  Serial.println("oxidator valve is LOCKED");
                }else{
                  Serial.println("oxidator valve is UNLOCKED");
                }
              }else
              {
                Serial.println("unknown command");
              }


    //##########################################################################################          
    }else if(incomingData.endsWith("?")){   // these commands are requests. they return data

          if(!incomingData.compareTo("temp?"))
          {
            temp1 = read_temp(1,AVG);
            temp2 = read_temp(2,AVG);
            Serial.print("temp.sensor 1: ");
            Serial.print(temp1, DEC);
            Serial.println(" degC");
            Serial.print("temp.sensor 2: ");
            Serial.print(temp2, DEC);
            Serial.println(" degC");
            
          }else
          if(!incomingData.compareTo("press?"))
          {
            pressure = read_pressure(AVG);
            Serial.print("N2O pressure: ");
            Serial.print(pressure, DEC);
            Serial.println(" bar");
            
          }else
          if(!incomingData.compareTo("status?"))
          {
            
            self_test(0);
            if(N2O_lock)
            {
              Serial.println("oxidator valve is LOCKED");
            }else{
              Serial.println("oxidator valve is UNLOCKED");
            }
          }else{
            Serial.println("unknown command");
          }
          
    
    //##########################################################################################
    }else
    {
          Serial.print("unknown command: ");
          Serial.println(incomingData);
    }
  }
}


void I2C_read()
{
  Wire.requestFrom(PSU_EC_ADR, 1);    // request 1 byte from PSU

  while(Wire.available())    // slave may send less than requested
  { 
    int c = Wire.read();    // receive a byte
    Serial.print("PSU Channel 1: ");
    if(c&(1<<0))
    {
      Serial.println("on");
    }else
    {
      Serial.println("off");
    }
    Serial.print("PSU Channel 2: ");
    if(c&(1<<1))
    {
      Serial.println("on");
    }else
    {
      Serial.println("off");
    }
    if((c&(1<<1))||(c&(1<<0)))
    {
      digitalWrite(10,HIGH);
    }else{
      digitalWrite(10,LOW);
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
  while(!Serial.available())
  {
      //need to add some kind of timeout to prevent endless loop!
  }
  confirmation =  Serial.readString();

  if(confirmation.equalsIgnoreCase("Y"))
  {
    return 1;
  }else{
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

