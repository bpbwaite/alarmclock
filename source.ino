/*
    by Bradyn Braithwaite, 2020
*/

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <TM1637Display.h>
#include <pins_arduino.h>

// GLOBAL VARIABLES AND OBJECTS
static const unsigned short displayClockPin       = 2;
static const unsigned short displayDataIOPin      = 3;
static const unsigned short button_A_setter       = 8;
static const unsigned short button_B_hour         = 4;  // hour, pullup
static const unsigned short button_C_minute       = 5;  // minute, pullup
static const unsigned short militaryPin           = 6;  // for debugging
static const unsigned short buzzerPin             = 9;  // any pin with PWM
static const unsigned short lightSensorAnalogPin  = 18; // analog input A0
static const unsigned short humidAndTempSensorPin = 7;
class timeUnit {
public:
    int hour;
    int minute;
};
TM1637Display myAlarmclock(displayClockPin, displayDataIOPin);
DHT dht11Sensor(humidAndTempSensorPin, 11); // set to mode 11
timeUnit Offset;                            // as described by button presses
timeUnit wakeTargetOffset;                  // as described by setting the alarm
byte ColonController;
unsigned int timeReadyToShow;
unsigned short brightnessValue           = 2;
bool militaryTimeMode                    = false;
bool alarmIsSet                          = false;
unsigned long millisWhenButtonLastPushed = 0;

// FUNCTIONS
int outputTimeAsNumber(timeUnit t_offset) {
    const double time_scale = 1.0;
    static unsigned int hourComponent, minuteComponent, minute_true;
    unsigned long sec = time_scale * millis() / 1000;
    minute_true       = (sec / 60 + Offset.minute);
    minuteComponent   = minute_true % 60;
    hourComponent     = (minute_true / 60 + Offset.hour) % 24;
    return 100 * hourComponent + minuteComponent;
}

int qTime() {
    return outputTimeAsNumber(Offset);
}

bool noButtonsAreBeingPushed() {
    return !(!digitalRead(button_B_hour) || !digitalRead(button_C_minute) || !digitalRead(button_A_setter));
    // DeMorgan would be disappointed in me
}

unsigned long timeSincelastButtonPush() {
    return millis() - millisWhenButtonLastPushed;
}

void alarmingFunction() {
    static const float loudnessScale = 0.85;
    static bool markedToRun          = true;
    static bool sounding             = false;
    if (sounding || (alarmIsSet && (qTime() == 100 * (wakeTargetOffset.hour % 24) + wakeTargetOffset.minute % 60) && markedToRun)) {
        sounding = true;
        while (noButtonsAreBeingPushed()) {
            if (millis() % 400 > 200) {
                digitalWrite(buzzerPin, HIGH);
                // analogWrite(buzzerPin, 0xFF * loudnessScale); // optional
            }
            else {
                digitalWrite(buzzerPin, LOW);
            }
        }
        while (!noButtonsAreBeingPushed()) { // means a button is being pushed
            sounding    = false;
            markedToRun = false;
            digitalWrite(buzzerPin, LOW); // halt buzzer
            delay(560);                   // not change anything by accident
        }
    }
    else if (!markedToRun && qTime() != (100 * wakeTargetOffset.hour + wakeTargetOffset.minute)) {
        markedToRun = true;
    }
}

void temperatureFunction() {
    static const unsigned int intervalOfService = 5; // minutes between showing the temperature
    const unsigned int persistentDelay          = 5000;
    static bool markedToRun                     = true;
    static const unsigned int requiredDelay     = 10000;
    static float temperature_F;
    static float temperature_C;
    bool Celsius = militaryTimeMode;                                                                                                                       // deprecated
    if (timeSincelastButtonPush() > requiredDelay && qTime() != 1200 && qTime() % intervalOfService == 0 && markedToRun && digitalRead(button_C_minute)) { // note: button_C_minute: prevents showing temp while passing the trigger when setting time
        myAlarmclock.clear();
        // Take reading:
        if (Celsius) {
            temperature_C = dht11Sensor.readTemperature(false); // false means celsius
            if (temperature_C < 100) {
                myAlarmclock.showNumberDec(temperature_C, false, 2, 1);
                myAlarmclock.showNumberHexEx(0xC, 0, false, 1, 3);
            }
        }
        else {
            temperature_F = dht11Sensor.readTemperature(true); // true means fahrenheit
            if (temperature_F < 100) {
                myAlarmclock.showNumberDec(temperature_F, false, 2, 1);
                myAlarmclock.showNumberHexEx(0xF, 0, false, 1, 3);
            }
        }
        delay(persistentDelay); // TODO: fix blocking
        markedToRun = false;
    }
    else if (qTime() % intervalOfService != 0) {
        markedToRun = true;
    }
}

void timingFunction() {
    timeReadyToShow = qTime();
    if (!militaryTimeMode) {
        int augmentHour = qTime() / 100;
        if (augmentHour == 0) {
            timeReadyToShow += 1200;
        }
        else if (augmentHour > 12) {
            timeReadyToShow -= 1200;
        }
    }
    if (millis() % 2000 > 1000 && !militaryTimeMode) {
        ColonController |= 255; // mask all bits to show a colon. Some chips require mask = 64
    }
    else {
        ColonController = 0;
    }
    myAlarmclock.showNumberDecEx(timeReadyToShow, ColonController, (timeReadyToShow < 99) || militaryTimeMode, 4, 0);
}

void lightSensorandBrightnessHandler() {
    static bool markedToRun    = true;
    const bool dynamicLighting = false;                            // a future chassis designed may allow for dynamic brightness adjustments
    const bool immediateChange = false;                            // the change is usually spread over a couple of minutes
    const short thresholds[4]  = {140, 250, 400, 750};             // temporary uncalibrated values
    unsigned int lightLevels   = analogRead(lightSensorAnalogPin); // use +5v and 15kOhm. ensure correct pull orientation
    if (dynamicLighting && !immediateChange) {
        if ((qTime() % 2 == 1) && markedToRun) {
            markedToRun = false;
            // proceed
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
        short linBrite = floor(0.0065525317 * lightLevels + 0.23986);
        myAlarmclock.setBrightness(linBrite < 8 && linBrite >= 0 ? linBrite : 4);
        // values computed by linear regression with fallback
    }
}

void flashRapidWhileSetup() {
    if (millis() % 1000 > 450) {
        myAlarmclock.setBrightness(1);
    }
    else {
        myAlarmclock.setBrightness(7);
    }
}

void buttonInputHandler() {
    // for checking releases; such as with alternate displays
    if (!digitalRead(button_A_setter) || !digitalRead(button_B_hour) || !digitalRead(button_C_minute)) {
        millisWhenButtonLastPushed = millis();
    }
    // for toggling alarm
    if (!digitalRead(button_A_setter) && !digitalRead(button_B_hour) && !digitalRead(button_C_minute)) { // roll from right to left to enable
        alarmIsSet = !alarmIsSet;
        if (alarmIsSet) {
            for (short x = 1; x < 75; x++) {
                flashRapidWhileSetup();
                delay(10);
            }
        }
    }
    // for setting alarm
    if (!digitalRead(button_A_setter)) {
        delay(1000);
        while (!digitalRead(button_A_setter)) {
            flashRapidWhileSetup();
            militaryTimeMode = true;
            myAlarmclock.showNumberDecEx(100 * (wakeTargetOffset.hour % 24) + wakeTargetOffset.minute % 60); // show the alarm clock setting
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
        // for setting time
        while (digitalRead(button_A_setter)) {
            flashRapidWhileSetup();
            militaryTimeMode = true;
            qTime(); // show the time setting
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
    }
}

void setup() {
    myAlarmclock.clear();
    dht11Sensor.begin();
    myAlarmclock.setBrightness(brightnessValue);
    pinMode(button_A_setter, INPUT_PULLUP);
    pinMode(button_B_hour, INPUT_PULLUP);
    pinMode(button_C_minute, INPUT_PULLUP);
    pinMode(militaryPin, INPUT_PULLUP);
    pinMode(buzzerPin, OUTPUT);
    digitalWrite(buzzerPin, LOW);
    // all other required pins set by object constructors
    Offset.hour             = 12; // start up at noon
    Offset.minute           = 0;
    alarmIsSet              = true;
    wakeTargetOffset.hour   = 12; // enter time here (24 hr)
    wakeTargetOffset.minute = 0;
}

void loop() {
    lightSensorandBrightnessHandler();
    timingFunction();
    alarmingFunction();
    buttonInputHandler();
    temperatureFunction();
}
