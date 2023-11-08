#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

/*
TODO:
- Logger levels
- RTC module for lamp timing
- Photoresistor to save energy
- Water level sensor
*/

#define DELAY_MS 500

#define SENSOR 3
#define INFO 2
#define ERROR 1

class Logger {
  public:
    int log(String message, int importance = 0) {
      Serial.println("[" + String(importance) + "]" + ": " + message);
      return 0;
    };
} logger;

class Ip {
  private:
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
            lcd.print("Hello, world!");
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

    // MOSFET
    class Mosfet {
      public:
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
    } mosfet;

    // CAPACITIVE SOIL MOISTURE SENSOR
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
  soil_moisture.read();
  lcd.update(soil_moisture.percentage);
  return 0;
};

void setup() {
  Serial.println("[*] Setup");
  Serial.begin(9600);
  ip.init();
  Serial.println("[*] Setup finished");
};

void loop() {
  ip.update();
  delay(DELAY_MS);
};