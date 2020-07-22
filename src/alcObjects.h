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
    void setHour(unsigned int);
    void setMinute(unsigned int);
    //! @return hour offset of this timeUnit
    unsigned int getHour();
    //! @return minute offset of this timeUnit
    unsigned int getMinute();
    void upHour(int);
    void upMinute(int);
    timeUnit();
    ~timeUnit();

protected:
    unsigned int hour;
    unsigned int minute;
};
class alckAbstract {
public:
    //! constructor makes false by default
    bool alarmIsSet;
    timeUnit wakeTargetOffset;
    virtual void runNow();
    alckAbstract();
    ~alckAbstract();

protected:
    unsigned short displayClockPin; // todo: improve static
    unsigned short displayDataIOPin;
    unsigned short button_A_setter;
    unsigned short button_B_hour;
    unsigned short button_C_minute;
    unsigned short buzzerPin;
    unsigned short defaultBrightness;
    bool militaryTimeMode;
    unsigned long msAtLastInteraction;
    unsigned int timeReadyToShow;
    unsigned long debouncingDelay;
    TM1637Display *clockDisplay;
    timeUnit Offset;
    //! @return amount of time since offset
    unsigned int outputTimeAsNumber(timeUnit);
    //! @return four-digit time code
    unsigned int qTime();
    void timingFunction();
    void alarmingFunction();
    void buttonInputHandler();
    //! @return milliseconds since last known button press
    unsigned long lastInteraction();
    //! @return if an interaction is taking place
    bool noInputsAreOn();
    virtual void flashRapidWhileSetup();
};
class alckAdvanced : public alckAbstract {
public:
    //! extends alck virtual
    void runNow();
    bool useTempRoutine;
    bool dynamicBrightness;
    alckAdvanced();
    ~alckAdvanced();

protected:
    unsigned short t_sensorPin;
    DHT *temperatureSensor;
    void temperatureRoutine();
    //! extends alck virtual
    void flashRapidWhileSetup();

private:
    bool debugMode;
    //! @return a number 0x0 to 0x7
    unsigned short maskClip(unsigned short);
    void brightnessHandlerRoutine();
};
#endif
