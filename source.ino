/*
    The 'Bluino'
    Bradyn Braithwaite
    June 2020
*/
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <TM1637Display.h>
#include <pins_arduino.h>

#define pinD2asClock         2
#define pinD3asDataInOut     3
#define button_A_setter      8
#define button_B_hour        4  // hour, inverted, pullup.
#define button_C_minute      5  // minute, inverted, pullup.
#define militaryEnabler      6  // no longer in use
#define buzzerPin            9  // pin with PWM
#define photodiodePullPin    A0 // A0 = 18, analog input. defineception
#define daHumidTempSensorPin 7
class timeUnit {
public:
    int hour;
    int minute;
};

TM1637Display myAlarmclock(pinD2asClock, pinD3asDataInOut);
DHT dht11Sensor(daHumidTempSensorPin, 11);
timeUnit Offset;
timeUnit targetWake;
byte ColonController;
int ttbshown;
short brightnessValue                = 2; // default
bool militarized                     = false;
bool alarmIsSet                      = false;
unsigned long miliWhenlastButtonPush = 0;

int outTimeAsInteger(timeUnit t_offset)
{
    const int time_scale = 1;
    static unsigned int h, m, m_true;
    unsigned long sec = time_scale * millis() / 1000;
    m                 = (sec / 60 + Offset.minute) % 60;
    m_true            = (sec / 60 + Offset.minute);
    h                 = (m_true / 60 + Offset.hour) % 24;
    return 100 * h + m;
}
int qTime() // refactored
{
    return outTimeAsInteger(Offset);
}
bool noButtonsAreBeingPushed()
{
    return !(!digitalRead(button_B_hour) || !digitalRead(button_C_minute) || !digitalRead(button_A_setter));
    // yes i know demorgans theorem but this makes WAY more sense
}
unsigned long timeSincelastButtonPush()
{
    return millis() - miliWhenlastButtonPush;
}
void alarmingFunction()
{
    static bool markedToRun = true;
    static bool sounding    = false;
    if (sounding || (alarmIsSet && (qTime() == 100 * targetWake.hour + targetWake.minute) && markedToRun)) {
        sounding = true;
        while (noButtonsAreBeingPushed()) {
            if (millis() % 400 > 200) {
                //analogWrite(buzzerPin, 0xFF * 0.80); // how much volt to try give
                digitalWrite(buzzerPin, HIGH);
            }
            else {
                //analogWrite(buzzerPin, 0);
                digitalWrite(buzzerPin, LOW);
            }
        }
        while (!noButtonsAreBeingPushed()) { // meaning, some are being pooshed
            sounding    = false;
            markedToRun = false;
            digitalWrite(buzzerPin, LOW); // gotta stop yer beeper
            delay(560);                   // debounce as to not change the time by accident
        }
    }
    else if (!markedToRun && qTime() != (100 * targetWake.hour + targetWake.minute)) {
        markedToRun = true;
    }
}

void temperatureFunction()
{
    const int intervalOfService = 5; // minutes
    static bool markedToRun     = true;
    static float temperature_F;
    static float temperature_C;
    bool Celsius = militarized;
    if (timeSincelastButtonPush() > 10000 && qTime() != 1200 && qTime() % intervalOfService == 0 && markedToRun && digitalRead(button_C_minute)) { // button_C_minute prevents showing temp while passing the increment during set time
        myAlarmclock.clear();
        // take reading:
        if (Celsius) {
            temperature_C = dht11Sensor.readTemperature(false); // false==celsius
            if (temperature_F < 100) {
                myAlarmclock.showNumberDec(temperature_C, false, 2, 1);
                myAlarmclock.showNumberHexEx(0xC, 0, false, 1, 3);
            }
            else {
                myAlarmclock.showNumberDec(temperature_C, false, 3, 0);
                myAlarmclock.showNumberHexEx(0xC, 0, false, 1, 3);
            }
        }
        else {
            temperature_F = dht11Sensor.readTemperature(true); // true==fahrenheit
            if (temperature_F < 100) {
                myAlarmclock.showNumberDec(temperature_F, false, 2, 1);
                myAlarmclock.showNumberHexEx(0xF, 0, false, 1, 3);
            }
            else {
                myAlarmclock.showNumberDec(temperature_F, false, 3, 0);
                myAlarmclock.showNumberHexEx(0xF, 0, false, 1, 3);
            }
        }

        delay(5000); // kinda blocking. cant change modes while showing temperature.
        markedToRun = false;
    }
    else if (qTime() % intervalOfService != 0) {
        markedToRun = true;
    }
}

void timingFunction()
{
    ttbshown = qTime();
    if (!militarized) {
        int augmentHour = qTime() / 100;
        if (augmentHour == 0) {
            ttbshown += 1200;
        }
        else if (augmentHour > 12) {
            ttbshown -= 1200;
        }
    }
    if (millis() % 2000 > 1000 && !militarized) {
        ColonController |= 255; // mask all bits, because cheap
    }
    else {
        ColonController = 0;
    }

    myAlarmclock.showNumberDecEx(ttbshown, ColonController, (ttbshown < 99) || militarized, 4, 0);
}
void lightSensorandBrightnessHandler()
{
    static bool markedToRun     = true;
    static bool dynamicLighting = false; // must be true in order to change on the fly
    static bool immediateChange = false;
    const short thresholds[4]   = {140, 250, 400, 750};          // temporary values.
    int lightLevels             = analogRead(photodiodePullPin); // make sure pull is correct direction
    if (dynamicLighting && !immediateChange) {
        if ((qTime() % 2 == 1) && markedToRun) {
            markedToRun = false;
            // proceed, limited to one change per minute.
            if (lightLevels < thresholds[0]) {
                brightnessValue = 1;
            }
            else if (lightLevels < thresholds[1]) {
                brightnessValue = 2;
            }
            else if (lightLevels < thresholds[2]) {
                brightnessValue = 3;
            }
            else if (lightLevels < thresholds[3]) {
                brightnessValue = 5;
            }
            else {
                brightnessValue = 7;
            }
            myAlarmclock.setBrightness(brightnessValue);
        }
        else if (qTime() % 2 != 1) {
            markedToRun = true;
        }
    }
    else if (dynamicLighting && immediateChange) {
        short temp = floor(0.0065525317 * lightLevels + 0.23986);
        myAlarmclock.setBrightness(temp < 8 && temp >= 0 ? temp : 4);
    }
}

void inFieldSetupandButtonFeeler() // refactored for expandability
{
    if (!digitalRead(button_B_hour) && !digitalRead(button_A_setter)) {
        Offset.hour++;
        miliWhenlastButtonPush = millis();
        delay(100);
    }
    if (!digitalRead(button_C_minute) && !digitalRead(button_A_setter)) {
        Offset.minute++;
        miliWhenlastButtonPush = millis();
        delay(100);
    }
    //if (!digitalRead(militaryEnabler)) {
    //    militarized = !militarized;
    //    while (!digitalRead(militaryEnabler)) {
    //        delay(100);
    //    }
    //}
}
void photo15kCalibrationSequence()
{
    myAlarmclock.showNumberDec(analogRead(photodiodePullPin));
    delay(500);
}
void setup()
{
    dht11Sensor.begin();
    myAlarmclock.setBrightness(brightnessValue);
    myAlarmclock.clear();
    pinMode(button_A_setter, INPUT_PULLUP);
    pinMode(button_B_hour, INPUT_PULLUP);
    pinMode(button_C_minute, INPUT_PULLUP);
    pinMode(militaryEnabler, INPUT_PULLUP);
    pinMode(buzzerPin, OUTPUT);
    digitalWrite(buzzerPin, LOW);
    // display, analogOut, & sensor pinmode is auto set by a superclass.
    Offset.hour       = 12;
    Offset.minute     = 0; // start up at noon
    alarmIsSet        = true;
    targetWake.hour   = 10;
    targetWake.minute = 0; // 10:00 am
}

void loop()
{
    lightSensorandBrightnessHandler();
    timingFunction();
    alarmingFunction();
    inFieldSetupandButtonFeeler();
    temperatureFunction();
}
