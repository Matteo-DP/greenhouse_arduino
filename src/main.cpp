#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Wire.h>
#include <SPI.h>
#include <config.h>
#include <logger.h>
#include <functions.h>
#include <BH1750.h>

/*
------------------ OVERVIEW ------------------
- See config.h for configuration (pins, delays, etc.)
- See logger.h for logger class
- /rpi/ contains python scripts for the Raspberry Pi
------------------ END OVERVIEW ------------------ 

------------------ TODO: ------------------
- Check time on RTC and Arduino, values are offset by 1 hour
- Wire the light sensor
- Test and change the light sensor threshold
- Test new serial communication protocol
------------------ END TODO ------------------ 
*/ 

// ------------------ CLASSES ------------------
// ------------------ IP ------------------
class Ip {
  private:
    // ------------------ LCD ------------------
    class Lcd {
      public:
        LiquidCrystal_I2C _lcd;
        Lcd() : _lcd(LCD_I2C_ADDRESS, 16, 2) {};
        int init() {
            _lcd.init();
            _lcd.clear();
            _lcd.backlight();
            _lcd.setCursor(0, 0);
            logger.log("LCD initialised", INFO);
            return 0;
        };
        int update(int value) {
            _lcd.clear();
            _lcd.setCursor(0, 0);
            _lcd.print("Soil: ");
            _lcd.print(value);
            _lcd.print("%");
            return 0;
        };
    } lcd;
    // ------------------ END LCD ------------------
    // ------------------ MOSFET ------------------
    class Mosfet {
      public:
        unsigned long currentMillis;
        unsigned long wateringStartMillis;
        unsigned long wateringPeriodMillis = PUMP_WATERING_PERIOD_MILLIS;
        unsigned long wateringTimeoutMillis = PUMP_WATERING_TIMEOUT_MILLIS;
        unsigned long wateringEndMillis;
        int initialWatering = 0;

        int pumpState = ch1[2]; // Initialse default pump state

        int lampsChanged = 0; // Used to check if the lamps config has changed

        // { pin, pin mode, default pin state }
        // Set all default lamp states to off, ask for which lamps to turn on on init
        const int ch1 [3] = {MOSFET_CH1_PIN, OUTPUT, LOW}; // PUMP
        const int ch2 [3] = {MOSFET_CH2_PIN, OUTPUT, LOW}; // INFRARED
        const int ch3 [3] = {MOSFET_CH3_PIN, OUTPUT, LOW}; // COLD WHITE
        const int ch4 [3] = {MOSFET_CH4_PIN, OUTPUT, LOW}; // BLOOMING

        // Configurable lamp states
        int useInfrared = ch2[2];
        int useColdWhite = ch3[2];
        int useBlooming = ch4[2];
        int useLightSensor = 0; // Default to not using it

        int init() {
          pinMode(ch1[0], ch1[1]);
          pinMode(ch2[0], ch2[1]);
          pinMode(ch3[0], ch3[1]);
          pinMode(ch4[0], ch4[1]);
          digitalWrite(ch1[0], ch1[2]);
          digitalWrite(ch2[0], ch2[2]);
          digitalWrite(ch3[0], ch3[2]);
          digitalWrite(ch4[0], ch4[2]);
          logger.askWhichLamps();
          logger.log("Mosfet initialised", INFO);
          return 0;
      };
      int printLampsConfig() {
        logger.log("Lamps config: infrared: " + String(useInfrared) + ", cold white: " + String(useColdWhite) + ", blooming: " + String(useBlooming) + ", light: " + String(useLightSensor), INFO);
        return 0;
      };
      int write(int channel, int value) {
          // if(value != (HIGH || LOW)) { logger.log("Unexpected mosfet write value", ERROR); return 1; }
          switch(channel) {
            case PUMP:
                logger.log("Turning pump " + String(value), INFO);
                digitalWrite(ch1[0], value);
                if(value == HIGH) {
                  wateringStartMillis = millis();
                  logger.log("Watering start millis: " + String(wateringStartMillis), DEBUG);
                  pumpState = 1;
                } else {
                  wateringEndMillis = millis();
                  logger.log("Watering end millis: " + String(wateringEndMillis), DEBUG);
                  pumpState = 0;
                }
                break;
            case INFRARED:
                logger.log("Turning infrared " + String(value), INFO);
                digitalWrite(ch2[0], value);
                break;
            case COLD_WHITE:
                logger.log("Turning cold white " + String(value), INFO);
                digitalWrite(ch3[0], value);
                break;
            case BLOOMING:
                logger.log("Turning blooming " + String(value), INFO);
                digitalWrite(ch4[0], value);
                break;
            case ALL_LAMPS:
                logger.log("Turning all available lamps " + String(value), INFO);
                switch(value) {
                  case LOW:
                    // Dont check for states, just turn off everything to be sure
                    digitalWrite(ch2[0], LOW);
                    digitalWrite(ch3[0], LOW);
                    digitalWrite(ch4[0], LOW);
                  case HIGH:
                    digitalWrite(ch2[0], useInfrared);
                    digitalWrite(ch3[0], useColdWhite);
                    digitalWrite(ch4[0], useBlooming);
                  break;
                }
                break;
            default:
                logger.log("Invalid channel: " + String(channel), ERROR);
                break;
          }
          return 0;
      };
      bool checkLampOn() {
        const int lamp1 = digitalRead(ch2[0]);
        const int lamp2 = digitalRead(ch3[0]);
        const int lamp3 = digitalRead(ch4[0]);
        if(lamp1 == HIGH || lamp2 == HIGH || lamp3 == HIGH) {
          return true;
        } else {
          return false;
        }
      }
      int checkWateringMillis() {
        if(pumpState == 1 && ((currentMillis - wateringStartMillis) > (wateringPeriodMillis))) {
          logger.log("Turning off pump, elapsed: " + String(currentMillis - wateringStartMillis), INFO);
          write(PUMP, LOW);
        } else if(pumpState == 1) {
          logger.log("Pump is on. Millis since watering: " + String(currentMillis - wateringStartMillis) + " total watering time: " + wateringPeriodMillis, DEBUG);
        };
        return 0;
      };
    } mosfet;
    // ------------------ END MOSFET ------------------
    // ------------------ SOIL MOISTURE ------------------
    class Soil_Moisture {
      public:
        const int pin [2] = {MOISTURE_PIN, INPUT};
        
        int percentage;
        int value;

        const int airValue = 598;
        const int waterValue = 45;

        int init() {
          pinMode(pin[0], pin[1]);
          logger.log("Soil moisture sensor initialised", INFO);
          return 0;
        };
        int read() {
          value = analogRead(pin[0]);
          percentage = map(value, airValue, waterValue, 0, 100);
          logger.log("Soil: " + String(value) + ", " + String(percentage) + "%", SENSOR);
          return 0;
        };
    } soil_moisture;
    // ------------------ END SOIL MOISTURE ------------------
    // ------------------ LIGHT SENSOR ------------------
    class Light {
      public:
        BH1750 GY30;
        float value;
        int init() {
          GY30.begin(); // Initialize the sensor object
          logger.log("Light sensor initialised", INFO);
          return 0;
        };
        int read() {
          value = GY30.readLightLevel();
          logger.log("Light: " + String(value), SENSOR);
          return 0;
        };
    } light;
    // ------------------ END LIGHT SENSOR ------------------
    // ------------------ RTC ------------------
    class RTC {
      public:
        RTC_DS1307 _rtc; // Default I2C address at 0x68
        DateTime now;
        int lampStartHour = RTC_START_HOUR;
        int lampEndHour = RTC_END_HOUR;
        String getTimeString() {
          return String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
        }
        String getDateTimeString() {
          return String(now.day()) + "/" + String(now.month()) + "/" + String(now.year()) + " " + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
        }
        int update() {
          now = _rtc.now();
          logger.log("RTC time: " + getDateTimeString(), DEBUG);
          return 0;
        };
        int init() {
          if(!_rtc.begin()) {
            logger.log("Couldn't find RTC", ERROR);
            return 1;
          };
          if (!_rtc.isrunning()) {
            logger.log("RTC is not running.", ERROR);
          };
          _rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // set RTC to compile time
          logger.log("RTC initialised: " + getDateTimeString(), INFO);
          return 0;
        };
    } rtc;
    // ------------------ END RTC ------------------
    // ------------------ PRIVATE IP VARIABLES ------------------
    int cycle = 110;
    // ------------------ END IP VARIABLES ------------------
    // ------------------ IP FUNCTIONS ------------------
    // Functions that require multiple nested classes' functionality
    // Alternative: pass a reference of the class to the function
    int lcdDisplayTimeString() {
      lcd._lcd.setCursor(0, 1);
      lcd._lcd.print(rtc.getTimeString());
      return 0;
    };
    int checkMoisture();
    int checkLamp();
    // ------------------ END IP FUNCTIONS ------------------
  public:
    // ------------------ PUBLIC IP VARIABLES ------------------
    unsigned int cycleDelay = CYCLE_DELAY_MS;
    // ------------------ END PUBLIC IP VARIABLES ------------------
    // ------------------ PUBLIC IP FUNCTIONS ------------------
    int init();
    int update();
    int readSerial();
    // ------------------ END PUBLIC IP FUNCTIONS ------------------
} ip;
// ------------------ END IP ------------------
// ------------------ IP CLASS FUNCTIONALITY ------------------
int Ip::checkMoisture() {
  // Check if watering is needed
  // Percentage should be under threshhold, pump should be off and millis since last watering should be over timeout to prevent constant watering due to water not being absorbed yet
  if(soil_moisture.percentage < PERCENTAGE_MOISTURE_THRESHOLD && mosfet.pumpState == 0 && (((mosfet.currentMillis - mosfet.wateringEndMillis) > (mosfet.wateringTimeoutMillis)) || mosfet.initialWatering == 0)) {
    logger.log("Turning on pump for " + String(mosfet.wateringPeriodMillis) + "millis", INFO);
    mosfet.write(PUMP, HIGH);
    if(mosfet.initialWatering == 0) mosfet.initialWatering = 1;
  } else {
    logger.log("Watering not needed", DEBUG);
  }
  return 0;
};
int Ip::checkLamp() {
  switch(mosfet.useLightSensor) {
    case 0:
      // Check if the current time is within the day cycle interval
      if (rtc.now.hour() >= rtc.lampStartHour && rtc.now.hour() < rtc.lampEndHour) {
        // Current time is within the day cycle interval
        if(mosfet.checkLampOn() == false && !(mosfet.useLightSensor == 0 && mosfet.useBlooming == 0 && mosfet.useInfrared == 0 && mosfet.useColdWhite == 0)) {
          mosfet.write(ALL_LAMPS, HIGH);
        } else if(mosfet.lampsChanged == 1) {
          mosfet.write(ALL_LAMPS, HIGH);
          mosfet.lampsChanged = 0;
        }
      } else {
        // Current time is outside the day cycle interval
        if(mosfet.checkLampOn() == true) {
          //mosfet.write(ALL_LAMPS, LOW);
          // Idk why this doesnt work, better safe than sorry
          digitalWrite(mosfet.ch2[0], LOW);
          digitalWrite(mosfet.ch3[0], LOW);
          digitalWrite(mosfet.ch4[0], LOW);
        }
      }
      break;
    case 1:
      if(light.value > LIGHT_THRESHOLD) {
        // Natural light, turn off lamps if they're on and return
        if(mosfet.checkLampOn() == true) {
          logger.log("Natural light detected, turning off lamps", INFO);
          //mosfet.write(ALL_LAMPS, LOW);
          digitalWrite(mosfet.ch2[0], LOW);
          digitalWrite(mosfet.ch3[0], LOW);
          digitalWrite(mosfet.ch4[0], LOW);
          
          return 0;
        };
        return 0;
      } else {
        // Check if the current time is within the day cycle interval
        if (rtc.now.hour() >= rtc.lampStartHour && rtc.now.hour() < rtc.lampEndHour) {
          // Current time is within the day cycle interval
          if(mosfet.checkLampOn() == false && !(mosfet.useLightSensor == 0 && mosfet.useBlooming == 0 && mosfet.useInfrared == 0 && mosfet.useColdWhite == 0)) {
            mosfet.write(ALL_LAMPS, HIGH);
          } else if(mosfet.lampsChanged == 1) {
            mosfet.write(ALL_LAMPS, HIGH);
            mosfet.lampsChanged = 0;
          }
        }
      }
      break;
  }
  return 0;
};
int Ip::init() {
  mosfet.currentMillis = millis();
  mosfet.init();
  lcd.init();
  soil_moisture.init();
  rtc.init();
  light.init();
  return 0;
};
int Ip::readSerial() {
  static char message[MAX_SERIAL_BUFFER_SIZE];
  static int messageIndex = 0;
  while(Serial.available() > 0) {
    char inChar = Serial.read();
    if(inChar != '<' && inChar != '>') {
      message[messageIndex] = inChar;
      messageIndex++;
    }
    if(inChar == '>') {
      message[messageIndex] = '\0';
      String messageString = String(message);
      messageIndex = 0;
      logger.log("Serial received: " + messageString, INFO);

      // Parse message
      String split[5]; // 5 elements max, 1 for each lamp, 1 for config declaration, 1 for light sensor
      splitString(messageString, ' ', split, 5);
      if(!split[0].equals("C")) return 1; // Not config message, no need to parse it here
      for(int i = 0; i < 5; i++) { // Works in any order
        if(split[i].startsWith("I")) {
          split[i].remove(0, 1); // Remove 1 character at index 0
          mosfet.useInfrared = split[i].toInt();
          mosfet.lampsChanged = 1;
          logger.log("Use infrared: " + String(mosfet.useInfrared), INFO);
        } else if(split[i].startsWith("W")) {
          split[i].remove(0, 1);
          mosfet.useColdWhite = split[i].toInt();
          mosfet.lampsChanged = 1;
          logger.log("Use cold white: " + String(mosfet.useColdWhite), INFO);
        } else if(split[i].startsWith("B")) {
          split[i].remove(0, 1);
          mosfet.useBlooming = split[i].toInt();
          mosfet.lampsChanged = 1;
          logger.log("Use blooming: " + String(mosfet.useBlooming), INFO);
        } else if(split[i].startsWith("L")) {
          split[i].remove(0, 1);
          mosfet.useLightSensor = split[i].toInt();
          logger.log("Use light sensor: " + String(mosfet.useLightSensor), INFO);
        }
      };

      // Empty the message array
      memset(message, 0, sizeof(message));
    }
  }
  return 0;
}
// ------------------ MAIN LOOP ------------------
int Ip::update() {
  cycle++;
  mosfet.currentMillis = millis();
  logger.log(String(mosfet.currentMillis), DEBUG);
  rtc.update(); // Update RTC time variable every cycle

  light.read();
  soil_moisture.read();
  lcd.update(soil_moisture.percentage);
  lcdDisplayTimeString();
  checkLamp();

  logger.log("Cycle: " + String(cycle), DEBUG);

  mosfet.checkWateringMillis();
  if(cycle == CYCLES_PER_ACTION) {
    checkMoisture();
    logger.log("Saving data. Cycle:" + String(cycle), DEBUG);
    logger.serialToRpiDb(SOIL_MOISTURE, soil_moisture.percentage);
    logger.serialToRpiDb(LIGHT_SENSOR, light.value);
    cycle = 0;
  };

  return 0;
};
// ------------------ END MAIN LOOP ------------------
// ------------------ END IP CLASS FUNCTIONALITY ------------------

// ------------------ END CLASSES ------------------

// ------------------ MAIN ------------------
void setup() {
  Wire.begin();
  Serial.begin(9600);
  logger.log("Setup ...", INFO);
  ip.init();
  logger.log("Setup finished", INFO);
};

void loop() {
  ip.readSerial();
  ip.update();
  delay(ip.cycleDelay);
};
// ------------------ END MAIN ------------------