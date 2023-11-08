#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

/*
TODO:
- Logger levels
- RTC module for lamp timing
- Photoresistor to save energy
- Water level sensor
*/

// CYCLES
#define CYCLE_MS 500
#define CYCLES_PER_ACTION 120 // Saving, watering

// LOGGER
// Log importance levels
#define SENSOR 4
#define DEBUG 3
#define INFO 2
#define ERROR 1

#define LOG_LEVEL DEBUG

// SOIL MOISTURE
#define PERCENTAGE_MOISTURE_THRESHOLD 50

// MOSFET
// Channel functionss
#define PUMP 1
#define INFRARED 2
#define COLD_WHITE 3
#define BLOOMING 4

#define PUMP_WATERING_TIME 5000 // ms, needs to be divisible by CYCLE_MS so that watering_cycles can be calculated as a whole number

class Logger {
  public:
    Logger() {
      Serial.begin(9600);
    }
    int log(String message, int importance = INFO) {
      if(importance <= LOG_LEVEL) {
        Serial.println("[" + String(importance) + "]" + ": " + message);
      }
      return 0;
    };
    int save(int value) {
      // Do something
      log("Should save data, but no logic here yet", ERROR);
      return 0;
    }
} logger;

class Ip {
  private:
    int cycle = 0;
    // LCD
    class Lcd {
      public:
        LiquidCrystal_I2C lcd;
        Lcd() : lcd(0x27, 16, 2) {};
        int init() {
            lcd.init();
            lcd.clear();
            lcd.backlight();
            lcd.setCursor(0, 0);
            return 0;
        };
        int update(int value) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Soil: ");
            lcd.print(value);
            lcd.print("%");
            return 0;
        };
    } lcd;

    class Mosfet {
      public:
        const int watering_cycles = PUMP_WATERING_TIME / CYCLE_MS;
        int cycles_since_watering = 0;
        int pumpState = 0;

        const int ch1 [3] = {10, OUTPUT, LOW}; // PUMP
        const int ch2 [3] = {11, OUTPUT, HIGH}; // INFRARED
        const int ch3 [3] = {12, OUTPUT, HIGH}; // COLD WHITE
        const int ch4 [3] = {13, OUTPUT, HIGH}; // BLOOMING

        int init() {
            pinMode(ch1[0], ch1[1]);
            pinMode(ch2[0], ch2[1]);
            pinMode(ch3[0], ch3[1]);
            pinMode(ch4[0], ch4[1]);
            digitalWrite(ch1[0], ch1[2]);
            digitalWrite(ch2[0], ch2[2]);
            digitalWrite(ch3[0], ch3[2]);
            digitalWrite(ch4[0], ch4[2]);
            return 0;
        };

        int write(int channel, int value) {
            switch(channel) {
                case 1:
                    digitalWrite(ch1[0], value);
                    if(value == HIGH) {
                      pumpState = 1;
                    } else {
                      pumpState = 0;
                    }
                    break;
                case 2:
                    digitalWrite(ch2[0], value);
                    break;
                case 3:
                    digitalWrite(ch3[0], value);
                    break;
                case 4:
                    digitalWrite(ch4[0], value);
                    break;
                default:
                    logger.log("Invalid channel: " + String(channel), ERROR);
                    break;
            }
            return 0;
        };

      int checkWateringCycles() {
        if(pumpState == 1 && cycles_since_watering == watering_cycles) {
          logger.log("Turning off pump", INFO);
          write(PUMP, LOW);
          cycles_since_watering = 0;
        } else if(pumpState == 1) {
          cycles_since_watering++;
          logger.log("Pump is on. Cycles since watering: " + String(cycles_since_watering), DEBUG);
        };
        return 0;
      };

    } mosfet;

    class Soil_Moisture {
      public:
        int percentage;
        int value;
        const int pin [2] = {A0, INPUT};
        const int airValue = 598;
        const int waterValue = 45;

        int init() {
          pinMode(pin[0], pin[1]);
          return 0;
        };
        int read() {
          value = analogRead(pin[0]);
          percentage = map(value, airValue, waterValue, 0, 100);
          logger.log("Soil: " + String(value) + ", " + String(percentage) + "%", SENSOR);
          return 0;
        };
    } soil_moisture;

    int checkMoisture() {
      // Check if watering is needed
      if(soil_moisture.percentage < PERCENTAGE_MOISTURE_THRESHOLD && mosfet.pumpState == 0) {
        logger.log("Turning on pump for " + String(mosfet.watering_cycles) + "cycles", INFO);
        mosfet.write(PUMP, HIGH);
      }
      return 0;
    };

  public:
    int init();
    int update();
} ip;

int Ip::init() {
  mosfet.init();
  lcd.init();
  soil_moisture.init();
  return 0;
};

int Ip::update() {
  cycle++;

  soil_moisture.read();
  lcd.update(soil_moisture.percentage);

  if(cycle == CYCLES_PER_ACTION) {
    checkMoisture();
    logger.log("Saving data. Cycle:" + String(cycle), INFO);
    logger.save(soil_moisture.percentage);
    cycle = 0;
  }

  mosfet.checkWateringCycles();
  return 0;
};

void setup() {
  logger.log("Setup ...", INFO);
  ip.init();
  logger.log("Setup finished", INFO);
};

void loop() {
  ip.update();
  delay(CYCLE_MS);
};