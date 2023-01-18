/*
    by B. Braithwaite, 2020
*/
#pragma once

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
    timeUnit wakeTargetOffset;
    void execute();

protected:
    byte displayClockPin;
    byte displayDataIOPin;
    byte button_A_setter;
    byte button_B_hour;
    byte button_C_minute;
    byte buzzerPin;

    byte defaultBrightness;

    unsigned long msAtLastInteraction;
    unsigned long debouncingDelay;
    int timeReadyToShow;
    bool m_mode;
    TM1637Display *clockDisplay;
    timeUnit Offset;

    //! @return amount of time since offset
    unsigned int outputTimeAsNumber(timeUnit);
    //! @return four-digit time code
    unsigned int qTime();
    inline byte maskClip(int);
    inline void timingFunction();
    inline void alarmingFunction();
    inline void buttonInputHandler();
    //! @return milliseconds since last known button press
    unsigned long lastInteraction();
    //! @return if an interaction is taking place
    void flashRapidWhileSetup();
    inline void brightnessHandlerRoutine();
};
