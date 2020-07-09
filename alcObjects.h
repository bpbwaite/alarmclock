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
    int hour;
    int minute;
};
class alck {
public:
    alck();
    ~alck();
    bool alarmIsSet;
    bool obeyDimTime;
    timeUnit wakeTargetOffset;
    unsigned short darkHoursStart;
    unsigned short darkHoursEnd;
    void runNow();

private:
    const unsigned short displayClockPin       = 2;
    const unsigned short displayDataIOPin      = 3;
    const unsigned short button_A_setter       = 8;
    const unsigned short button_B_hour         = 4;  // hour, pullup
    const unsigned short button_C_minute       = 5;  // minute, pullup
    const unsigned short militaryPin           = 6;  // for debugging
    const unsigned short buzzerPin             = 9;  // any pin with PWM
    const unsigned short lightSensorAnalogPin  = 18; // analog input A0
    const unsigned short humidAndTempSensorPin = 7;

    unsigned int timeReadyToShow             = 0;
    unsigned short brightnessValue           = 2;
    bool militaryTimeMode                    = false;
    unsigned long millisWhenButtonLastPushed = 0;
    TM1637Display *thisClock;
    DHT *temperatureSensor;
    timeUnit Offset;
    byte ColonController;
    unsigned int outputTimeAsNumber(timeUnit t_offset);
    unsigned int qTime();
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
