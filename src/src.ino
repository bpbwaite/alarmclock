#include "alckObjects.h"

#define SEEDUINO
// board type used to determine pin mappings
// Order: displayClockPin displayDataIOPin button_A_setter button_B_hour
// button_C_minute buzzerPin
#ifdef SEEDUINO
uint8_t pinMap[6] = {2, 3, 5, 4, 8, 9};
#endif
#ifdef ATTINY
uint8_t pinMap[6] = {0, 1, 2, 3, 4, 5};
#endif
alck arduinoAlck(pinMap);
void setup() {
    // stringToAlarmTime(&arduinoAlck, "8:00 AM");
    arduinoAlck.wakeTargetOffset.setHour(8);
    arduinoAlck.wakeTargetOffset.setMinute(0);
    arduinoAlck.alarmIsSet        = false;
    arduinoAlck.brightnessRoutine = true;
}
void loop() {
    arduinoAlck.execute();
}
