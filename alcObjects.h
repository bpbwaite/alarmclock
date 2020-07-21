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
    void downMinute(); // can refactor these^
    timeUnit();
    ~timeUnit();

protected:
    unsigned int hour;
    unsigned int minute;
};
class alckAbstract {
public:
    bool alarmIsSet; // no modifications; alarm is false
    timeUnit wakeTargetOffset;
    virtual void runNow();
    alckAbstract();
    ~alckAbstract();

protected: // nothing is static, however; no more than one object should be made
    unsigned short displayClockPin;
    unsigned short displayDataIOPin;
    unsigned short button_A_setter;
    unsigned short button_B_hour;
    unsigned short button_C_minute;
    unsigned short buzzerPin;
    unsigned short defaultBrightness;
    bool militaryTimeMode;
    float time_scale;
    unsigned long msAtLastInteraction;
    unsigned int timeReadyToShow;
    unsigned long tinyDelay;
    unsigned long smallDelay;
    unsigned long mediumDelay;
    unsigned long largeDelay;
    unsigned long ventiDelay;
    unsigned long hugeDelay;
    TM1637Display *clockDisplay;
    timeUnit Offset;
    unsigned int outputTimeAsNumber(timeUnit);
    unsigned int qTime();
    void timingFunction();
    void alarmingFunction();
    void buttonInputHandler();
    unsigned long lastInteraction();
    bool noInputsAreOn();
    virtual void flashRapidWhileSetup();
};
class alckAdvanced : public alckAbstract {
public:
    void runNow(); // extends virtual
    bool useTempRoutine;
    bool dynamicBrightness;
    alckAdvanced();
    ~alckAdvanced();

protected:
    unsigned short t_sensorPin;
    DHT *temperatureSensor;
    void temperatureRoutine();
    void flashRapidWhileSetup(); // extends virtual
private:
    bool debugMode;
    unsigned short maskClip(unsigned short);
    void brightnessHandlerRoutine();
};
#endif
