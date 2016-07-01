      // HyCOMET-1 Interface Board
      // configuration file


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
      
