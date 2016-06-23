      // HyCOMET-1 Interface Board
      // configuration file


      #define serial_speed 9600     //serial interface (UART) baud rate [default: 9k6 Baud, 8 data bits, no parity bit, one stop bit]
      #define TIMEOUT 250           //serial interface timeout in ms (keep low for a quicker response)

      //How many ADC-samples are used to calculate one average reading [4,8,16,32]
      #define AVG 8

      //calibration data for temperature sensors
      #define CAL_MULT_1 1
      #define CAL_MULT_2 0.978
      #define CAL_OFFSET -3

      //calibration data for pressure sensor (1bar ~ 1mV)
      #define CAL_PRESSURE 9.3

      //servo motor controls
      #define OPEN 1700        //pulse lenght in µs for open valve divided by 64µs to calculate timer value
      #define CLOSED 1300     //pulse lenght in µs for closed valve divided by 64µs to calculate timer value
      
