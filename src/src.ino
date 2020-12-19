/*
    by Bradyn Braithwaite, 2020
*/
#include "Arduino.h"
#include "alcObjects.h"
alckAdvanced nanoAlck;
void setup() {
    nanoAlck.dynamicBrightness = true;
    nanoAlck.useTempRoutine    = false;
    nanoAlck.wakeTargetOffset.setHour(8);
    nanoAlck.wakeTargetOffset.setMinute(5);
}
void loop() {
    nanoAlck.runNow();
}
