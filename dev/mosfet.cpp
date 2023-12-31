#include <Arduino.h>

// MOSFET
const int mosfet_ch1_pin = 10;
const int mosfet_ch2_pin = 11;
const int mosfet_ch3_pin = 12;
const int mosfet_ch4_pin = 13;

void setup() {
  Serial.begin(9600);
  Serial.println("[*] Setup");

  pinMode(mosfet_ch1_pin, OUTPUT);
  pinMode(mosfet_ch2_pin, OUTPUT);
  pinMode(mosfet_ch3_pin, OUTPUT);
  pinMode(mosfet_ch4_pin, OUTPUT);

  // Set MOSFET off
  digitalWrite(mosfet_ch1_pin, LOW);
  digitalWrite(mosfet_ch2_pin, LOW);
  digitalWrite(mosfet_ch3_pin, LOW);
  digitalWrite(mosfet_ch4_pin, LOW);

  Serial.println("[*] Setup finished");
}

void loop() {
  Serial.println("Turning ON CH1");
  digitalWrite(mosfet_ch1_pin, HIGH);
  delay(1000); // 5 sec
  Serial.println("Turning OFF CH1");
  digitalWrite(mosfet_ch1_pin, LOW);
  delay(1000); // 5 sec
}