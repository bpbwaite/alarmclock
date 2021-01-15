/*
    by Bradyn Braithwaite, 2020
*/
#ifndef _ALCKOBJECTS_H
#define _ALCKOBJECTS_H

#include "Arduino.h"
#include <TM1637Display.h>
#include <math.h>
class timeUnit {
public:
  timeUnit();
  void setHour(int);
  void setMinute(int);
  //! @return hour offset of this timeUnit
  int getHour();
  //! @return minute offset of this timeUnit
  int getMinute();
  void upHour(int);
  void upMinute(int);

protected:
  int hour;
  int minute;
};
class alck {
public:
  alck(uint8_t *);
  bool alarmIsSet; // false by default
  bool brightnessRoutine;

  timeUnit wakeTargetOffset;
  void execute();

protected:
  static byte displayClockPin;
  static byte displayDataIOPin;
  static byte button_A_setter;
  static byte button_B_hour;
  static byte button_C_minute;
  static byte buzzerPin;

  byte defaultBrightness;

  unsigned long msAtLastInteraction;
  unsigned long debouncingDelay;
  int timeReadyToShow;
  bool m_mode;
  bool debugMode;

  TM1637Display *clockDisplay;
  timeUnit Offset;

  //! @return amount of time since offset
  unsigned int outputTimeAsNumber(timeUnit);
  //! @return four-digit time code
  unsigned int qTime();
  byte maskClip(int);
  void timingFunction();
  void alarmingFunction();
  void buttonInputHandler();
  //! @return milliseconds since last known button press
  unsigned long lastInteraction();
  //! @return if an interaction is taking place
  bool noInputsAreOn();
  void flashRapidWhileSetup();
  void brightnessHandlerRoutine();
};

#endif
