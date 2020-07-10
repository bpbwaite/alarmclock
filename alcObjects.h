/*
    by Bradyn Braithwaite, 2020
*/
#ifndef ALCOBJECTS_H
#define ALCOBJECTS_H
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <TM1637Display.h>
#include <pins_arduino.h>
class timeUnit {
public:
    unsigned int hour;
    unsigned int minute;
};
class alck {
public:
    alck();
    alck(unsigned short alckMappings[9], timeUnit time_in, timeUnit alarm_in, bool alarmIsSet, bool obeyDimTime, bool dynamicLighting, bool immediateChange, unsigned short brightnessValue);
    ~alck();
    bool alarmIsSet;
    bool obeyDimTime;
    unsigned short darkHoursStart;
    unsigned short darkHoursEnd;
    timeUnit wakeTargetOffset;
    void runNow();

private:
    const unsigned short displayClockPin       = 2;
    const unsigned short displayDataIOPin      = 3;
    const unsigned short button_A_setter       = 8;
    const unsigned short button_B_hour         = 4;
    const unsigned short button_C_minute       = 5;
    const unsigned short militaryPin           = 6;
    const unsigned short buzzerPin             = 9;
    const unsigned short lightSensorAnalogPin  = 18;
    const unsigned short humidAndTempSensorPin = 7;
    bool dynamicLighting; // a future chassis design may allow for dynamic brightness adjustments
    bool immediateChange; // the change is usually spread over a couple of minutes
    unsigned short brightnessValue;
    static const unsigned short defaultBrightness = 5;
    bool militaryTimeMode;
    unsigned long millisWhenButtonLastPushed;
    TM1637Display *thisClock;
    DHT *temperatureSensor;
    timeUnit Offset;
    unsigned int outputTimeAsNumber(timeUnit t_offset);
    unsigned int qTime();
    unsigned int timeReadyToShow;
    bool noButtonsAreBeingPushed();
    unsigned long timeSincelastButtonPush();
    void alarmingFunction();
    void temperatureFunction();
    void timingFunction();
    void lightSensorandBrightnessHandler();
    void flashRapidWhileSetup();
    void buttonInputHandler();
};
#endif
