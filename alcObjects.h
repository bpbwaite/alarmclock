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
    void setHour(unsigned int h_offset = 0);
    void setMinute(unsigned int m_offset = 0);
    unsigned int getHour();
    unsigned int getMinute();
    void upHour();
    void downHour();
    void upMinute();
    void downMinute(); // can refactor

protected:
    unsigned int hour;
    unsigned int minute;
};
class alckAbstract {
public:
    bool alarmIsSet;           // with no modifications, the device can have an alarm set to a time, and be ran from here
    timeUnit wakeTargetOffset; // alarmIsSet is false by default
    virtual void runNow();
    alckAbstract();
    ~alckAbstract();

protected:                                      // nothing is static, however; no more than one object should be made at a time.
    const unsigned short displayClockPin   = 2; // these pins will not be constants in the future
    const unsigned short displayDataIOPin  = 3;
    const unsigned short button_A_setter   = 8;
    const unsigned short button_B_hour     = 4;
    const unsigned short button_C_minute   = 5;
    const unsigned short buzzerPin         = 9;
    const unsigned short defaultBrightness = 5;
    bool militaryTimeMode;
    float time_scale;
    unsigned long millisWhenButtonLastPushed;
    unsigned int timeReadyToShow;
    TM1637Display *clockDisplay;
    timeUnit Offset;
    unsigned int outputTimeAsNumber(timeUnit);
    unsigned int qTime();
    unsigned long timeSincelastButtonPush();
    bool noButtonsAreBeingPushed();
    void alarmingFunction();
    void timingFunction();
    virtual void flashRapidWhileSetup();
    void buttonInputHandler();
};
class alckAdvanced : public alckAbstract {
public:
    void runNow(); // extends virtual
    bool useTempRoutine;
    bool dynamicBrightness;
    alckAdvanced();
    ~alckAdvanced();

protected:
    const unsigned short humidAndTempSensorPin = 7; // not for long
    bool debugMode;
    DHT *temperatureSensor;
    void temperatureFunction();
    void flashRapidWhileSetup(); // extends virtual
private:
    void brightnessHandlerRoutine();
    unsigned short maskClip(unsigned short);
};
#endif
