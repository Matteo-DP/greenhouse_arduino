#include <Arduino.h>
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void splitString(String message, char delimiter, String words[], int maxWords) {
  int wordIndex = 0;
  int startIndex = 0;
  int endIndex = 0;

  while (wordIndex < maxWords && endIndex != -1) {
    endIndex = message.indexOf(delimiter, startIndex);
    if (endIndex == -1) {
      words[wordIndex] = message.substring(startIndex);
    } else {
      words[wordIndex] = message.substring(startIndex, endIndex);
      startIndex = endIndex + 1;
    }
    wordIndex++;
  }
}
// const int ch2 [3] = {MOSFET_CH2_PIN, OUTPUT, HIGH}; // INFRARED
// const int ch3 [3] = {MOSFET_CH3_PIN, OUTPUT, HIGH}; // COLD WHITE
// const int ch4 [3] = {MOSFET_CH4_PIN, OUTPUT, HIGH}; // BLOOMING
String lampIntToString(int value) {
  switch(value) {
    case 2:
      return "INFRARED";
    case 3:
      return "COLD_WHITE";
    case 4:
      return "BLOOMING";
    default:
      return "INVALID";
    break;
  }
};
int lampStringToInt(String lamp) {
  if(lamp == "INFRARED") {
    return 2;
  } else if(lamp == "COLD_WHITE") {
    return 3;
  } else if(lamp == "BLOOMING") {
    return 4;
  } else {
    return 0;
  }
};
#endif