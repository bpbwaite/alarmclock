/*
    by Bradyn Braithwaite, 2020
*/
#include "alckObjects.h"

timeUnit::timeUnit() {
  this->hour = 0;
  this->minute = 0;
}
void timeUnit::setHour(int h_offset) { this->hour = h_offset % 24; }
void timeUnit::setMinute(int m_offset) { this->minute = m_offset % 60; }
int timeUnit::getHour() { return this->hour; }
int timeUnit::getMinute() { return this->minute; }
void timeUnit::upHour(int amount) { this->hour += amount; }
void timeUnit::upMinute(int amount) { this->minute += amount; }

// Beginning of ALCK method definitions:
alck::alck(uint8_t *pinMap) {
  displayClockPin = pinMap[0];
  displayDataIOPin = pinMap[1];
  button_A_setter = pinMap[2];
  button_B_hour = pinMap[3];
  button_C_minute = pinMap[4];
  buzzerPin = pinMap[5];
  pinMode(button_A_setter, INPUT_PULLUP);
  pinMode(button_B_hour, INPUT_PULLUP);
  pinMode(button_C_minute, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  analogWrite(buzzerPin, LOW);
  defaultBrightness = 0x5;
  clockDisplay = new TM1637Display(displayClockPin, displayDataIOPin);
  clockDisplay->setBrightness(defaultBrightness);
  clockDisplay->clear();

  msAtLastInteraction = 0U;
  debouncingDelay = 125;
  m_mode = false;
  alarmIsSet = false;
  brightnessRoutine = false;
  debugMode = false;

  Offset.setHour(12); // initialize time to noon
  Offset.setMinute(0);
  wakeTargetOffset.setHour(12); // initialize to noon alarm
  wakeTargetOffset.setMinute(0);
}
bool alck::noInputsAreOn() {
  return digitalRead(button_B_hour) && digitalRead(button_C_minute) &&
         digitalRead(button_A_setter);
}
unsigned long alck::lastInteraction() { return millis() - msAtLastInteraction; }
unsigned int alck::outputTimeAsNumber(timeUnit t_offset) {
  static long hourComponent, minuteComponent, minute_true;
  unsigned long sec = millis() / 1000;
  minute_true = (sec / 60 + Offset.getMinute());
  minuteComponent = minute_true % 60;
  hourComponent = (minute_true / 60 + Offset.getHour()) % 24;
  return 100 * hourComponent + minuteComponent;
}
unsigned int alck::qTime() { return outputTimeAsNumber(Offset); }
void alck::timingFunction() {
  const byte mask = 64; // colon mask. some chips require 64
  timeReadyToShow = qTime();
  if (!m_mode) {
    int augmentHour = timeReadyToShow / 100;
    if (augmentHour == 0) {
      timeReadyToShow += 1200;
    } else if (augmentHour > 12) {
      timeReadyToShow -= 1200;
    }
  }
  clockDisplay->showNumberDecEx(
      timeReadyToShow,
      !alarmIsSet || (millis() % 2000 > 1000 && !m_mode) ? mask : 0x0,
      (timeReadyToShow < 99U) || m_mode, 0x4, 0x0);
}
void alck::flashRapidWhileSetup() {
  if (millis() % 750 > 375) {
    clockDisplay->setBrightness(0x1);
  } else {
    clockDisplay->setBrightness(0x7);
  }
}
void alck::buttonInputHandler() { // needs work
  const unsigned int modeSwapDelay = 500;
  static unsigned int preventionDelay = 5 * debouncingDelay;
  if (!noInputsAreOn()) { // means a button is being pushed
    digitalWrite(buzzerPin,
                 LOW); // don't allow alarm to go when a button is pushed
  }
  if (!digitalRead(button_A_setter) || !digitalRead(button_B_hour) ||
      !digitalRead(button_C_minute)) {
    msAtLastInteraction = millis();
  }
  if (!digitalRead(button_B_hour) &&
      !digitalRead(button_C_minute)) { // press both hour and minute to toggle
    alarmIsSet = !alarmIsSet;          // for toggling alarm
    delay(preventionDelay);
  }
  if (!digitalRead(button_A_setter)) { // for setting alarm
    delay(modeSwapDelay);
    while (!digitalRead(button_A_setter)) {
      flashRapidWhileSetup();
      m_mode = true;
      clockDisplay->showNumberDecEx(100 * (wakeTargetOffset.getHour() % 24) +
                                        wakeTargetOffset.getMinute() % 60,
                                    64, true, 4,
                                    0); // show the alarm clock setting
      m_mode = false;
      if (!digitalRead(button_B_hour)) {
        wakeTargetOffset.upHour(1);
        delay(debouncingDelay);
      }
      if (!digitalRead(button_C_minute)) {
        wakeTargetOffset.upMinute(1);
        delay(debouncingDelay);
      }
    }
    while (digitalRead(button_A_setter)) { // for setting time
      flashRapidWhileSetup();
      m_mode = true;
      clockDisplay->showNumberDecEx(qTime(), 64, true, 4,
                                    0); // show the time setting
      m_mode = false;
      if (!digitalRead(button_B_hour)) {
        Offset.upHour(1);
        delay(debouncingDelay);
      }
      if (!digitalRead(button_C_minute)) {
        Offset.upMinute(1);
        delay(debouncingDelay);
      }
    } // could use another protected method
    delay(preventionDelay);
  }
}
void alck::alarmingFunction() { // needs work
  const double loudnessScale = 0.95;
  static bool markedToRun = true;
  static bool sounding = false;
  if (((lastInteraction() > (2 * debouncingDelay)) && sounding) ||
      (alarmIsSet &&
       (qTime() == 100U * (wakeTargetOffset.getHour() % 24) +
                       wakeTargetOffset.getMinute() % 60) &&
       markedToRun)) {
    sounding = true;
    do {
      timingFunction(); // update time while beeping
      if (sounding && millis() % 400 > 200) {
        analogWrite(buzzerPin, 0xFF * loudnessScale);
      } else {
        digitalWrite(buzzerPin, LOW);
      }
    } while (noInputsAreOn());
    // exiting allows any button press to halt the buzzer
    sounding = false;
    markedToRun = false;
    alarmIsSet = false;
    // Force alarm off. code is blocking and bad. rethink it
  } else if (!markedToRun && qTime() != (100U * wakeTargetOffset.getHour() +
                                         wakeTargetOffset.getMinute())) {
    markedToRun = true;
  }
}
byte alck::maskClip(int inputC) {
  return inputC < 0 ? 0x0 : (inputC > 7 ? 0x7 : (inputC));
}
void alck::brightnessHandlerRoutine() {
  double t_dec = (qTime() / 100.0);
  clockDisplay->setBrightness(
      maskClip(ceil(0.0003 * pow(t_dec, 4) - 0.0182 * pow(t_dec, 3) +
                    0.316 * pow(t_dec, 2) - 1.212 * t_dec + 0.972)));
}
void alck::execute() {
  timingFunction();
  alarmingFunction();
  buttonInputHandler();
  brightnessHandlerRoutine();
  if (debugMode) {
    // nothing here yet
  }
}
