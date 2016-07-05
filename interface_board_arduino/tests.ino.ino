// ####################################################
// self test routine
// this function will test all sensors and all I2C slaves (not implementet yet)
// if any component is missing or defective, setup_error count in incremented
// if setup_error is > 0 after all tests, the self test will fail and return
// the number of errors. Also, details are available via serial console to 
// aid in fault finding
// ####################################################
int self_test(int silent){

    int setup_error = 0;
    int16_t temp_check_1 = 0;
    int16_t temp_check_2 = 0;
    int16_t pressure_check = 0;
    
    if(!silent)Serial.println("\nstarting self test... \n");

    //read sensors
    temp_check_1 = read_temp(1,AVG);
    temp_check_2 = read_temp(2,AVG);
    pressure_check = read_pressure(AVG);

    //check temperature sensor 1
    if(temp_check_1 < -60)
    {
      Serial.println("ERROR: temp.sensor 1 SHORTED!!");
      setup_error++;
    }else if(temp_check_1 > 120)
    {
      Serial.println("ERROR: temp.sensor 1 OPEN!!");
      setup_error++;
    }else{
      if(!silent)
      {
        Serial.print("temp.sensor 1: ");
        Serial.print(temp_check_1, DEC);
        Serial.print(" degC");
        Serial.println(" (OK)");
      }
    }


     //check temperature sensor 2
     if(temp_check_2 < -60)
    {
      Serial.println("ERROR: temp.sensor 2 SHORTED!!");
      setup_error++;
    }else if(temp_check_2 > 120)
    {
      Serial.println("ERROR: temp.sensor 2 OPEN!!");
      setup_error++;
    }else{
      if(!silent)
      {
        Serial.print("temp.sensor 2: ");
        Serial.print(temp_check_2, DEC);
        Serial.print(" degC");
        Serial.println(" (OK)");
      }
    }

    // enable/disable the temperature test LED and error LED
    if(setup_error)
    {
      digitalWrite(7,HIGH); //error LED
    }else{
      digitalWrite(11,HIGH);  //temp test ok LED
    }

    //check pressure sensor
    if(pressure_check < 0)
    {
      Serial.println("\nERROR: N2O pressure sensor sanity check FAILED!!");
      Serial.print("Sensor reads: ");
      Serial.print(pressure_check, DEC);
      Serial.println(" bar");
      Serial.println("check wiring to sensor\n");
      setup_error++;
    }else{
      if(!silent)
      {
        Serial.print("N2O tank pressure: ");
        Serial.print(pressure_check, DEC);
        Serial.println(" (OK)");
      }
    }


    //see if any errors occured and either switch on error LED or self test passed LED
    if(setup_error)
    {
      digitalWrite(7,HIGH);
      digitalWrite(12,LOW); 
      if(!silent)Serial.print("\nself test FAILED with ");
      if(!silent)Serial.print(setup_error, DEC);
      if(!silent)Serial.println(" ERROR(S)!! \n");
    }else{
      digitalWrite(12,HIGH);  
      digitalWrite(7,LOW);
      if(!silent)Serial.println("\nself test PASSED \n");
    }
    return setup_error;
}
