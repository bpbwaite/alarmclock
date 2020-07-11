/*
    by Bradyn Braithwaite, 2020
*/
#include "alcObjects.h"
alck myAlClock;
void setup() {
    // Serial.begin(9600);
    myAlClock.alarmIsSet              = false;
    myAlClock.wakeTargetOffset.hour   = 10; // enter a time here (24 hr)
    myAlClock.wakeTargetOffset.minute = 0;
    myAlClock.obeyDimTime             = false;
    myAlClock.time_btest              = true;
    myAlClock.time_scale              = 1.0;
    myAlClock.debugMode               = false;
}
void loop() {
    myAlClock.runNow(); // it loops anyways
}
