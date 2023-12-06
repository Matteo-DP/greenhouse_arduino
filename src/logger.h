#include <config.h>
#include <Arduino.h>

class Logger {
  public:
    int log(String message, int importance = INFO) {
      if(importance <= logLevel) {
        Serial.println("LOG: [" + logLevelToString(importance) + "]" + ": " + message);
      }
      return 0;
    };
    int serialToRpiDb(int type, int value) {
      // Serial communication with Raspberry Pi
      // Line must begin with "RPI:" and end with "\n"
      // Format: RPI: <type> <value>
      Serial.println("RPI: " + String(type) + " " + String(value));
      return 0;
    }
    int askWhichLamps() {
      // Ask which lamps to turn on
      // Format: ASK: LAMPS
      Serial.println("ASK: LAMPS");
      return 0;
    }
  private:
    const int logLevel = LOG_LEVEL;
    String logLevelToString(int importance) {
      switch(importance) {
        case SENSOR:
          return "S";
        case DEBUG:
          return "D";
        case INFO:
          return "*";
        case ERROR:
          return "X";
        default:
          return "?";
      }
    }
} logger;