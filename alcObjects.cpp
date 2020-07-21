/*
    by Bradyn Braithwaite, 2020
*/
#include "alcObjects.h"
void timeUnit::upHour() {
    this->hour++;
}
void timeUnit::downHour() {
    this->hour--;
}
void timeUnit::upMinute() {
    this->minute--;
}
void timeUnit::downMinute() {
    this->minute--;
}
alckAbstract::alckAbstract() { // reminder: this constructor is always called
    time_scale   = 1.0;
    clockDisplay = new TM1637Display(displayClockPin, displayDataIOPin);
    pinMode(button_A_setter, INPUT_PULLUP);
    pinMode(button_B_hour, INPUT_PULLUP);
    pinMode(button_C_minute, INPUT_PULLUP);
    pinMode(buzzerPin, OUTPUT);
    digitalWrite(buzzerPin, LOW);
    clockDisplay->setBrightness(defaultBrightness); // no longer redundant
    clockDisplay->clear();
    militaryTimeMode = false;
    alarmIsSet       = false;
    Offset.setHour(12);
    Offset.setMinute();
    wakeTargetOffset.setHour(12); // defaults to noon if left on
    wakeTargetOffset.setMinute();
    millisWhenButtonLastPushed = 0;
}
alckAbstract::~alckAbstract() {}
alckAdvanced::alckAdvanced() {
    dynamicBrightness = false;
    useTempRoutine    = true;
    debugMode         = false;
    temperatureSensor = new DHT(humidAndTempSensorPin, 11); // always set to mode 11
    temperatureSensor->begin();
}
alckAdvanced::~alckAdvanced() {}
void alckAbstract::runNow() {
    do {
        timingFunction();
        alarmingFunction();
        buttonInputHandler();
        delay(5); // limiter
    } while (true);
}
void alckAdvanced::runNow() {
    do {
        brightnessHandlerRoutine();
        timingFunction();
        alarmingFunction();
        buttonInputHandler();
        temperatureFunction();
        if (debugMode) {
        }
        delay(5); // limiter
    } while (true);
}
void timeUnit::setHour(unsigned int h_offset = 0) { // try not to call these without arguments
    this->hour = h_offset;
}
void timeUnit::setMinute(unsigned int m_offset = 0) {
    this->minute = m_offset;
}
unsigned int timeUnit::getHour() {
    return this->hour;
}
unsigned int timeUnit::getMinute() {
    return this->minute;
}
unsigned int alckAbstract::outputTimeAsNumber(timeUnit t_offset) {
    static unsigned int hourComponent, minuteComponent, minute_true;
    unsigned long sec = time_scale * millis() / 1000;
    minute_true       = (sec / 60 + Offset.getMinute());
    minuteComponent   = minute_true % 60;
    hourComponent     = (minute_true / 60 + Offset.getHour()) % 24;
    return 100 * hourComponent + minuteComponent;
}
unsigned int alckAbstract::qTime() {
    return outputTimeAsNumber(this->Offset);
}
bool alckAbstract::noButtonsAreBeingPushed() {
    return !(!digitalRead(button_B_hour) || !digitalRead(button_C_minute) || !digitalRead(button_A_setter)); // DeMorgan would be disappointed in me
}
unsigned long alckAbstract::timeSincelastButtonPush() {
    return millis() - millisWhenButtonLastPushed;
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
    else if ((int)floor(time_scale) * millis() % 2000 > 1000 && !militaryTimeMode) {
        ColonController |= mask;
    }
    else {
        ColonController = 0;
    }
    clockDisplay->showNumberDecEx(timeReadyToShow, ColonController, (timeReadyToShow < 99) || militaryTimeMode, 4, 0);
}
unsigned short alckAdvanced::maskClip(unsigned short inputC) {
    return inputC < 0 ? 0 : (inputC > 7 ? 7 : (inputC));
}
void alckAbstract::flashRapidWhileSetup() {
    if ((int)floor(time_scale) * millis() % 1000 > 500) {
        clockDisplay->setBrightness(2);
    }
    else {
        clockDisplay->setBrightness(6);
    }
}
void alckAdvanced::flashRapidWhileSetup() {
    if ((int)floor(time_scale) * millis() % 750 > 375) {
        clockDisplay->setBrightness(1);
    }
    else {
        clockDisplay->setBrightness(7);
    }
}
void alckAbstract::buttonInputHandler() {       // needs work
    const unsigned int set_wait_debounce = 500; // for checking releases; such as with temperature display
    if (!digitalRead(button_A_setter) || !digitalRead(button_B_hour) || !digitalRead(button_C_minute)) {
        millisWhenButtonLastPushed = millis();
    }
    if (!digitalRead(button_B_hour) && !digitalRead(button_C_minute)) { // press both hour and minute to toggle
        alarmIsSet = !alarmIsSet;                                       // for toggling alarm
        delay(set_wait_debounce);
    }
    if (!digitalRead(button_A_setter)) { // for setting alarm
        delay(700);
        while (!digitalRead(button_A_setter)) {
            flashRapidWhileSetup();
            militaryTimeMode = true;
            clockDisplay->showNumberDecEx(100 * (wakeTargetOffset.getHour() % 24) + wakeTargetOffset.getMinute() % 60, 64, true, 4, 0); // show the alarm clock setting
            militaryTimeMode = false;
            if (!digitalRead(button_B_hour)) {
                wakeTargetOffset.upHour();
                delay(120);
            }
            if (!digitalRead(button_C_minute)) {
                wakeTargetOffset.upMinute();
                delay(120);
            }
        }
        while (digitalRead(button_A_setter)) { // for setting time
            flashRapidWhileSetup();
            militaryTimeMode = true;
            clockDisplay->showNumberDecEx(qTime(), 64, true, 4, 0); // show the time setting
            militaryTimeMode = false;
            if (!digitalRead(button_B_hour)) {
                Offset.upHour();
                delay(120);
            }
            if (!digitalRead(button_C_minute)) {
                Offset.upMinute();
                delay(120);
            }
        }
        delay(set_wait_debounce);
    }
}
void alckAbstract::alarmingFunction() {
    const float loudnessScale = 0.95;
    static bool markedToRun   = true;
    static bool sounding      = false;
    if (((timeSincelastButtonPush() > 5000) && sounding) || (alarmIsSet && (qTime() == 100U * (wakeTargetOffset.getHour() % 24) + wakeTargetOffset.getMinute() % 60) && markedToRun)) {
        sounding = true;
        while (noButtonsAreBeingPushed()) {
            timingFunction();           // update time while beeping
            if (millis() % 400 > 200) { // dont incorporate timescale into this one
                analogWrite(buzzerPin, 0xFF * loudnessScale);
                delay(2);
            }
            else {
                digitalWrite(buzzerPin, LOW);
                delay(2);
            }
        }
        while (!noButtonsAreBeingPushed()) { // means a button is being pushed
            sounding    = false;
            markedToRun = false;
            digitalWrite(buzzerPin, LOW); // halt buzzer
            delay(1000);                  // hopefully not change anything by accident
        }
    }
    else if (!markedToRun && qTime() != (100U * wakeTargetOffset.getHour() + wakeTargetOffset.getMinute())) {
        markedToRun = true;
    }
}
void alckAdvanced::temperatureFunction() {                    // want to make configurable
    const unsigned int intervalOfService    = 5 * time_scale; // absolute minutes between showing the temperature. todo, public
    const unsigned int persistentDelayratio = 5000;           // gets divided by timescale. need to unify with other debouncers
    static bool markedToRun                 = true;
    const unsigned int requiredDelay        = 24000;
    static float temperature_F;
    static float temperature_C;
    if (!useTempRoutine) {
        // skip
    }
    else {                                                                                                                                                     // deprecated
        if (timeSincelastButtonPush() > requiredDelay && qTime() != 1200 && qTime() % intervalOfService == 0 && markedToRun && digitalRead(button_C_minute)) { // note: button_C_minute: prevents showing temp while passing the trigger when setting time
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
            delay(persistentDelayratio / floor(time_scale));
            markedToRun = false;
        }
        else if (qTime() % intervalOfService != 0) {
            markedToRun = true;
        }
    }
}
void alckAdvanced::brightnessHandlerRoutine() {
    float t_dec = (qTime() / 100.0);
    clockDisplay->setBrightness(maskClip(ceil(.0003 * pow(t_dec, 4) - 0.0182 * pow(t_dec, 3) + 0.316 * pow(t_dec, 2) - 1.212 * t_dec + 0.972))); // much better
}
