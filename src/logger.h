#include <config.h>
#include <Arduino.h>

class Logger {
  public:
    Logger() {
      Serial.begin(9600);
    }
    int log(String message, int importance = INFO) {
      if(importance <= logLevel) {
        Serial.println("[" + logLevelToString(importance) + "]" + ": " + message);
      }
      return 0;
    };
    int save(int value) {
      // Do something
      log("Should save data / , but no logic here yet", ERROR);
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
};