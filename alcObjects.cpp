/*
    by Bradyn Braithwaite, 2020
*/
#include "alcObjects.h"

alckBase::alckBase() { // reminder parent class constructor is called first
    time_scale   = 1.0;
    clockDisplay = new TM1637Display(displayClockPin, displayDataIOPin);
    pinMode(button_A_setter, INPUT_PULLUP);
    pinMode(button_B_hour, INPUT_PULLUP);
    pinMode(button_C_minute, INPUT_PULLUP);
    pinMode(buzzerPin, OUTPUT);
    digitalWrite(buzzerPin, LOW);
    brightnessValue = defaultBrightness;
    clockDisplay->setBrightness(brightnessValue);
    clockDisplay->clear();
    militaryTimeMode           = false;
    alarmIsSet                 = false;
    Offset.hour                = 12;
    Offset.minute              = 0;
    wakeTargetOffset.hour      = 0;
    wakeTargetOffset.minute    = 0;
    millisWhenButtonLastPushed = 0;
}
alckBase::~alckBase() {}
alckAdvanced::alckAdvanced() {
    temperatureSensor = new DHT(humidAndTempSensorPin, 11); // always set to mode 11
    temperatureSensor->begin();
    bright_computed = defaultBrightness;
    darkHoursStart  = 0;
    darkHoursEnd    = 0;
}
alckAdvanced::~alckAdvanced() {}
void alckBase::runNow() {
    do {
        timingFunction();
        alarmingFunction();
        buttonInputHandler();
        delay(5); // limiter
    } while (true);
}
void alckAdvanced::runNow() {
    do {
        lightSensorandBrightnessHandler();
        timingFunction();
        alarmingFunction();
        buttonInputHandler();
        temperatureFunction();
        if (debugMode) {
            //   Serial.println("time (int):"); // Serial is definitely identified, but alas
            //   Serial.println(qTime());
            //   Serial.println("brightness:");
            //   Serial.println(this->bright_computed);
            //   delay(100); // limit to 10 updates/sec max
        }
        delay(5); // limiter
    } while (true);
} // maybe these two above blocks can be refactored?
unsigned int alckBase::outputTimeAsNumber(timeUnit t_offset) {
    static unsigned int hourComponent, minuteComponent, minute_true;
    unsigned long sec = time_scale * millis() / 1000;
    minute_true       = (sec / 60 + Offset.minute);
    minuteComponent   = minute_true % 60;
    hourComponent     = (minute_true / 60 + Offset.hour) % 24;
    return 100 * hourComponent + minuteComponent;
}
unsigned int alckBase::qTime() {
    return outputTimeAsNumber(this->Offset);
}
bool alckBase::noButtonsAreBeingPushed() {
    return !(!digitalRead(button_B_hour) || !digitalRead(button_C_minute) || !digitalRead(button_A_setter)); // DeMorgan would be disappointed in me
}
unsigned long alckBase::timeSincelastButtonPush() {
    return millis() - millisWhenButtonLastPushed;
}
void alckBase::timingFunction() {
    const byte mask             = 64; // mask bits to show a colon. note some chips require mask = 64
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
void alckBase::flashRapidWhileSetup() {
    if ((int)floor(time_scale) * millis() % 1000 > 450) {
        clockDisplay->setBrightness(2);
    }
    else {
        clockDisplay->setBrightness(7);
    }
}
void alckBase::buttonInputHandler() {
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
            clockDisplay->showNumberDecEx(100 * (wakeTargetOffset.hour % 24) + wakeTargetOffset.minute % 60, 64, true, 4, 0); // show the alarm clock setting
            militaryTimeMode = false;
            if (!digitalRead(button_B_hour)) {
                wakeTargetOffset.hour++;
                delay(120);
            }
            if (!digitalRead(button_C_minute)) {
                wakeTargetOffset.minute++;
                delay(120);
            }
        }
        while (digitalRead(button_A_setter)) { // for setting time
            flashRapidWhileSetup();
            militaryTimeMode = true;
            clockDisplay->showNumberDecEx(qTime(), 64, true, 4, 0); // show the time setting
            militaryTimeMode = false;
            if (!digitalRead(button_B_hour)) {
                Offset.hour++;
                delay(120);
            }
            if (!digitalRead(button_C_minute)) {
                Offset.minute++;
                delay(120);
            }
        }
        delay(set_wait_debounce);
    }
}
void alckBase::alarmingFunction() {
    const float loudnessScale = 0.95;
    static bool markedToRun   = true;
    static bool sounding      = false;
    if (((timeSincelastButtonPush() > 5000) && sounding) || (alarmIsSet && (qTime() == 100U * (wakeTargetOffset.hour % 24) + wakeTargetOffset.minute % 60) && markedToRun)) {
        sounding = true;
        while (noButtonsAreBeingPushed()) {
            timingFunction();           // update time sometimes while beeping
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
    else if (!markedToRun && qTime() != (100U * wakeTargetOffset.hour + wakeTargetOffset.minute)) {
        markedToRun = true;
    }
}
void alckAdvanced::temperatureFunction() {
    const unsigned int intervalOfService    = 5 * time_scale; // minutes between showing the temperature, absolute
    const unsigned int persistentDelayratio = 5000;           // gets divided by timescale
    static bool markedToRun                 = true;
    const unsigned int requiredDelay        = 24000;
    static float temperature_F;
    static float temperature_C;
    bool Celsius = militaryTimeMode;
    if (timeSincelastButtonPush() > requiredDelay && qTime() != 1200 && qTime() % intervalOfService == 0 && markedToRun && digitalRead(button_C_minute)) { // note: button_C_minute: prevents showing temp while passing the trigger when setting time
        clockDisplay->clear();
        if (Celsius) {                                                 // Take reading:
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

void alckAdvanced::lightSensorandBrightnessHandler() {
    static bool markedToRun            = true;
    const unsigned short thresholds[4] = {140, 250, 400, 750}; // temporary, uncalibrated values
    static float t_dec                 = 0.0;
    if (time_btest) {
        t_dec           = (qTime() / 100.0);
        bright_computed = floor(.000462 * pow(t_dec, 4) - 0.0254 * pow(t_dec, 3) + 0.397 * pow(t_dec, 2) - 1.354 * t_dec + 0.654);
        clockDisplay->setBrightness(bright_computed > 7 ? 7 : (bright_computed < 0 ? 0 : bright_computed)); // fallback
    }
    else if (obeyDimTime && ((qTime() / 100.0 >= darkHoursStart) || (qTime() / 100.0 <= darkHoursEnd))) {
        clockDisplay->setBrightness(0);
    }
    else if (dynamicLighting && !immediateChange) {
        unsigned int lightLevels = analogRead(lightSensorAnalogPin); // use +5v and 15k
        if ((qTime() % 2 == 1) && markedToRun) {
            markedToRun = false; // proceed
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
                brightnessValue = defaultBrightness;
            }
            clockDisplay->setBrightness(brightnessValue);
        }
        else if (qTime() % 2 != 1) {
            markedToRun = true;
        }
        else if (dynamicLighting && immediateChange) {
            short linBrite = floor(0.00655 * lightLevels + 0.239); // lin reg with fallback
            clockDisplay->setBrightness(linBrite <= 7 && linBrite >= 0 ? linBrite : defaultBrightness);
        }
        else {
            clockDisplay->setBrightness(defaultBrightness);
        }
    }
}
