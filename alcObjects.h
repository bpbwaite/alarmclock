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
class alckBase {
public:
    bool alarmIsSet;           // with no modifications, the device can have an alarm set to a time, and be ran from here
    timeUnit wakeTargetOffset; // alarmIsSet is false, by the base constructor
    virtual void runNow();     // the more advanced versions of the clock implement the method differently
    alckBase();
    ~alckBase();

protected:                                      // nothing static; but it is assumed only one object is made
    const unsigned short displayClockPin   = 2; // these pins will not be constant in the future
    const unsigned short displayDataIOPin  = 3;
    const unsigned short button_A_setter   = 8;
    const unsigned short button_B_hour     = 4;
    const unsigned short button_C_minute   = 5;
    const unsigned short buzzerPin         = 9;
    const unsigned short defaultBrightness = 5;
    unsigned short brightnessValue;
    bool militaryTimeMode;
    float time_scale; // make setter
    unsigned long millisWhenButtonLastPushed;
    unsigned int timeReadyToShow;
    TM1637Display *clockDisplay;
    timeUnit Offset;
    unsigned int outputTimeAsNumber(timeUnit t_offset);
    unsigned int qTime();
    unsigned long timeSincelastButtonPush();
    bool noButtonsAreBeingPushed();
    void alarmingFunction();
    void timingFunction();
    void flashRapidWhileSetup();
    void buttonInputHandler();
};
class alckAdvanced : public alckBase {
public:
    void runNow();
    alckAdvanced();
    ~alckAdvanced();

protected:
    const unsigned short lightSensorAnalogPin  = 18; // again, not constant in future
    const unsigned short humidAndTempSensorPin = 7;
    unsigned short darkHoursStart; // make a setter for 0-23
    unsigned short darkHoursEnd;
    short bright_computed;
    bool obeyDimTime     = false; // old way of determining brightness
    bool dynamicLighting = false; // a future chassis design may use a sensor
    bool immediateChange = false; // the change is spread over a couple of minutes
    bool debugMode       = false; // everything is set here so the constructors dont have to
    bool time_btest      = false; // have these been set in other contexts?
    DHT *temperatureSensor;
    void temperatureFunction();
    void lightSensorandBrightnessHandler();
};
#endif
