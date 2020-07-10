#include "alcObjects.h"
alck myAlClock;
void setup() {
    myAlClock.alarmIsSet              = false;
    myAlClock.wakeTargetOffset.hour   = 10; // enter time here (24 hr)
    myAlClock.wakeTargetOffset.minute = 0;
    myAlClock.darkHoursStart          = 22;
    myAlClock.darkHoursEnd            = 7;
    myAlClock.obeyDimTime             = true;
}
void loop() {
    myAlClock.runNow(); // it loops anyways
}
