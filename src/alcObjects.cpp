/*
    by Bradyn Braithwaite, 2020
*/
#include "alcObjects.h"
// TIME UNIT OBJECT DEFINITIONS
timeUnit::timeUnit() {
    this->hour   = 0x0;
    this->minute = 0x0;
}
timeUnit::~timeUnit() {}
void timeUnit::setHour(unsigned int h_offset) {
    this->hour = h_offset;
}
void timeUnit::setMinute(unsigned int m_offset) {
    this->minute = m_offset;
}
unsigned int timeUnit::getHour() {
    return this->hour;
}
unsigned int timeUnit::getMinute() {
    return this->minute;
}
void timeUnit::upHour(int amount) {
    this->hour += amount;
}
void timeUnit::upMinute(int amount) {
    this->minute += amount;
}
// ABSTRACT ALCK DEFINITIONS
alckAbstract::alckAbstract() {
    displayClockPin   = 2;
    displayDataIOPin  = 3;
    button_A_setter   = 8;
    button_B_hour     = 4;
    button_C_minute   = 5;
    buzzerPin         = 9;
    defaultBrightness = 5;
    tinyDelay         = 3;
    smallDelay        = 125;
    mediumDelay       = 500;
    largeDelay        = 1500;
    ventiDelay        = 5000;
    hugeDelay         = 24000;
    clockDisplay      = new TM1637Display(displayClockPin, displayDataIOPin);
    pinMode(button_A_setter, INPUT_PULLUP);
    pinMode(button_B_hour, INPUT_PULLUP);
    pinMode(button_C_minute, INPUT_PULLUP);
    pinMode(buzzerPin, OUTPUT);
    analogWrite(buzzerPin, LOW);
    clockDisplay->setBrightness(defaultBrightness);
    clockDisplay->clear();
    militaryTimeMode = false;
    alarmIsSet       = false;
    Offset.setHour(12);
    Offset.setMinute(0);
    wakeTargetOffset.setHour(12); // start up at noon
    wakeTargetOffset.setMinute(0);
    msAtLastInteraction = 0;
}
alckAbstract::~alckAbstract() {}
bool alckAbstract::noInputsAreOn() {
    return !(!digitalRead(button_B_hour) || !digitalRead(button_C_minute) || !digitalRead(button_A_setter)); // DeMorgan would be disappointed
}
unsigned long alckAbstract::lastInteraction() {
    return millis() - msAtLastInteraction;
}
unsigned int alckAbstract::outputTimeAsNumber(timeUnit t_offset) {
    static unsigned int hourComponent, minuteComponent, minute_true;
    unsigned long sec = millis() / 1000;
    minute_true       = (sec / 60 + Offset.getMinute());
    minuteComponent   = minute_true % 60;
    hourComponent     = (minute_true / 60 + Offset.getHour()) % 24;
    return 100 * hourComponent + minuteComponent;
}
unsigned int alckAbstract::qTime() {
    return outputTimeAsNumber(this->Offset);
}
void alckAbstract::timingFunction() {
    const byte mask             = 64; // colon mask. note some chips require 64
    static byte ColonController = mask;
    timeReadyToShow             = qTime();
    if (!militaryTimeMode) {
        int augmentHour = qTime() / 100;
        if (augmentHour == 0) {
            timeReadyToShow += 1200;
        }
        else if (augmentHour > 12) {
            timeReadyToShow -= 1200;
        }
    }
    if (!alarmIsSet) {
        ColonController |= mask; // the colon flashes only when alarm is set.
    }
    else if (millis() % 2000 > 1000 && !militaryTimeMode) {
        ColonController |= mask;
    }
    else {
        ColonController = 0;
    }
    clockDisplay->showNumberDecEx(timeReadyToShow, ColonController, (timeReadyToShow < 99) || militaryTimeMode, 4, 0);
}
void alckAbstract::flashRapidWhileSetup() {
    if (millis() % 1000 > 500) {
        clockDisplay->setBrightness(2);
    }
    else {
        clockDisplay->setBrightness(6);
    }
}
void alckAbstract::buttonInputHandler() { // needs work
    if (!digitalRead(button_A_setter) || !digitalRead(button_B_hour) || !digitalRead(button_C_minute)) {
        msAtLastInteraction = millis();
    }
    if (!digitalRead(button_B_hour) && !digitalRead(button_C_minute)) { // press both hour and minute to toggle
        alarmIsSet = !alarmIsSet;                                       // for toggling alarm
        delay(mediumDelay);
    }
    if (!digitalRead(button_A_setter)) { // for setting alarm
        delay(largeDelay);
        while (!digitalRead(button_A_setter)) {
            flashRapidWhileSetup();
            militaryTimeMode = true;
            clockDisplay->showNumberDecEx(100 * (wakeTargetOffset.getHour() % 24) + wakeTargetOffset.getMinute() % 60, 64, true, 4, 0); // show the alarm clock setting
            militaryTimeMode = false;
            if (!digitalRead(button_B_hour)) {
                wakeTargetOffset.upHour(1);
                delay(smallDelay);
            }
            if (!digitalRead(button_C_minute)) {
                wakeTargetOffset.upMinute(1);
                delay(smallDelay);
            }
        }
        while (digitalRead(button_A_setter)) { // for setting time
            flashRapidWhileSetup();
            militaryTimeMode = true;
            clockDisplay->showNumberDecEx(qTime(), 64, true, 4, 0); // show the time setting
            militaryTimeMode = false;
            if (!digitalRead(button_B_hour)) {
                Offset.upHour(1);
                delay(smallDelay);
            }
            if (!digitalRead(button_C_minute)) {
                Offset.upMinute(1);
                delay(smallDelay);
            }
        }
        delay(mediumDelay);
    }
}
void alckAbstract::alarmingFunction() {
    const float loudnessScale = 0.95;
    static bool markedToRun   = true;
    static bool sounding      = false;
    if (((lastInteraction() > ventiDelay) && sounding) || (alarmIsSet && (qTime() == 100U * (wakeTargetOffset.getHour() % 24) + wakeTargetOffset.getMinute() % 60) && markedToRun)) {
        sounding = true;
        while (noInputsAreOn()) {
            timingFunction(); // update time while beeping
            if (millis() % 400 > 200) {
                analogWrite(buzzerPin, 0xFF * loudnessScale);
                delay(tinyDelay);
            }
            else {
                digitalWrite(buzzerPin, LOW);
                delay(tinyDelay);
            }
        }
        while (!noInputsAreOn()) { // means a button is being pushed
            sounding    = false;
            markedToRun = false;
            digitalWrite(buzzerPin, LOW); // halt buzzer
            delay(largeDelay);            // hopefully not change anything by accident
        }
    }
    else if (!markedToRun && qTime() != (100U * wakeTargetOffset.getHour() + wakeTargetOffset.getMinute())) {
        markedToRun = true;
    }
}
void alckAbstract::runNow() {
    do {
        timingFunction();
        alarmingFunction();
        buttonInputHandler();
        delay(tinyDelay); // limiter
    } while (true);
}
// ADVANCED ALCK DEFINITIONS
alckAdvanced::alckAdvanced() {
    dynamicBrightness = false;
    useTempRoutine    = true;
    debugMode         = false;
    t_sensorPin       = 7;
    temperatureSensor = new DHT(t_sensorPin, 11); // mine are mode 11
    temperatureSensor->begin();
}
alckAdvanced::~alckAdvanced() {}
void alckAdvanced::flashRapidWhileSetup() {
    if (millis() % 750 > 375) {
        clockDisplay->setBrightness(1);
    }
    else {
        clockDisplay->setBrightness(7);
    }
}
void alckAdvanced::temperatureRoutine() {
    const unsigned int intervalOfService = 5; //interval; todo: public
    static bool markedToRun              = true;
    static float temperature_F;
    static float temperature_C;
    if (!useTempRoutine) {
        // skip
    }
    else {                                                                                                                                         // deprecated
        if (lastInteraction() > hugeDelay && qTime() != 1200 && qTime() % intervalOfService == 0 && markedToRun && digitalRead(button_C_minute)) { // note: button_C_minute: prevents showing temp while passing the trigger when setting time
            clockDisplay->clear();
            if (militaryTimeMode) {                                        // legacy switch for taking reading:
                temperature_C = temperatureSensor->readTemperature(false); // false means celsius
                if (temperature_C < 100) {
                    clockDisplay->showNumberDec(temperature_C, false, 2, 1);
                    clockDisplay->showNumberHexEx(0xC, 0, false, 1, 3);
                }
            }
            else {
                temperature_F = temperatureSensor->readTemperature(true); // true means fahrenheit
                if (temperature_F < 100) {
                    clockDisplay->showNumberDec(temperature_F, false, 2, 1);
                    clockDisplay->showNumberHexEx(0xF, 0, false, 1, 3);
                }
            }
            delay(ventiDelay);
            markedToRun = false;
        }
        else if (qTime() % intervalOfService != 0) {
            markedToRun = true;
        }
    }
}
unsigned short alckAdvanced::maskClip(unsigned short inputC) {
    return inputC < 0 ? 0 : (inputC > 7 ? 7 : (inputC));
}
void alckAdvanced::brightnessHandlerRoutine() {
    float t_dec = (qTime() / 100.0);
    clockDisplay->setBrightness(maskClip(ceil(.0003 * pow(t_dec, 4) - 0.0182 * pow(t_dec, 3) + 0.316 * pow(t_dec, 2) - 1.212 * t_dec + 0.972))); // much better
}
void alckAdvanced::runNow() {
    do {
        brightnessHandlerRoutine();
        timingFunction();
        alarmingFunction();
        buttonInputHandler();
        temperatureRoutine();
        if (debugMode) {
        }
        delay(tinyDelay);
    } while (true);
}
