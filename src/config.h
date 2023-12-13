// ------------------ CONFIG ------------------

// ------------------ CLOCK CYCLES ------------------
#define CYCLE_DELAY_MS 500 // Main loop delay
#define CYCLES_PER_ACTION 120*5 // 5 minutes (functions: check lamp, save moisture data), Total time = CYCLE_DELAY_MS * CYCLES_PER_ACTION
// ------------------ END CLOCK CYCLES ------------------

// ------------------ LOGGING ------------------
// Log importance levels
#define SENSOR 4
#define DEBUG 3
#define INFO 2
#define ERROR 1

#define LOG_LEVEL INFO // Log level to display logs for, logs with a higher importance level will be displayed as well, with sensor being the lowest and error the highest
// ------------------ END LOGGING ------------------

// ------------------ SOIL MOISTURE ------------------
#define MOISTURE_PIN A0 // Analog pin
#define PERCENTAGE_MOISTURE_THRESHOLD 35 // Percentage of moisture at which the pump should be turned on
// ------------------ END SOIL MOISTURE ------------------

// ------------------ MOSFET ------------------
// Channel pins
#define MOSFET_CH1_PIN 10
#define MOSFET_CH2_PIN 11
#define MOSFET_CH3_PIN 12
#define MOSFET_CH4_PIN 13
// Channel functions, channel at which the functions are connected (1-4)
#define PUMP 1
#define INFRARED 2
#define COLD_WHITE 3
#define BLOOMING 4
#define ALL_LAMPS 5 // Edge case to select all lamps (channels 2-4)
// Pump timing
#define PUMP_WATERING_PERIOD_MILLIS 5000
#define PUMP_WATERING_TIMEOUT_MILLIS 60000*15 // 15 minutes
// ------------------ END MOSFET ------------------

// ------------------ RTC ------------------
// Simulated day cycle for lamp
#define RTC_START_HOUR 8 // 8:00
#define RTC_END_HOUR 18 // 18:00
// ------------------ END RTC ------------------

// ------------------ LCD ------------------
#define LCD_I2C_ADDRESS 0x27
// ------------------ END LCD ------------------


// ------------------ DATABASE TYPE MAP ------------------
// TODO: check type map
#define SOIL_MOISTURE 0
#define LIGHT_SENSOR 1
// ------------------ END DATABASE TYPE MAP ------------------

// ------------------ SERIAL FROM RPI ------------------
#define MAX_SERIAL_BUFFER_SIZE 32
// ------------------ END SERIAL FROM RPI ------------------

// ------------------ FLOW SENSOR ------------------
#define FLOW_SENSOR_PIN 2
// ------------------ END FLOW SENSOR ------------------


// ------------------ LIGHT SENSOR ------------------
#define LIGHT_PIN A1
#define LIGHT_THRESHOLD 375 // TODO: TEST AND CHANGE THIS VALUE
// ------------------ END LIGHT SENSOR ------------------

// ------------------ END CONFIG ------------------