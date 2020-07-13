/*
    by Bradyn Braithwaite, 2020
*/
#include "alcObjects.h"
alckAdvanced myAlClock; // beautiful initialization.
void setup() {
    myAlClock.alarmIsSet              = false;
    myAlClock.wakeTargetOffset.hour   = 10; // enter time in 24hr
    myAlClock.wakeTargetOffset.minute = 0;
}
void loop() {
    myAlClock.runNow(); // it loops
}
