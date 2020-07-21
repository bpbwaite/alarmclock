/*
    by Bradyn Braithwaite, 2020
*/
#include "alcObjects.h"
alckAdvanced nanoAlck;
void setup() {
    nanoAlck.dynamicBrightness = true;
    nanoAlck.useTempRoutine    = true;
    nanoAlck.alarmIsSet        = false;
    nanoAlck.wakeTargetOffset.setHour(10);
    nanoAlck.wakeTargetOffset.setMinute(0x0);
}
void loop() {
    nanoAlck.runNow();
}
