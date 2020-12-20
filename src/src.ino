#include "alckObjects.h"
#include "alckutils.h"
alck arduinoAlck;
void setup() {
    //stringToAlarmTime(&arduinoAlck, "8:00 AM");
    arduinoAlck.wakeTargetOffset.setHour(8);
    arduinoAlck.wakeTargetOffset.setMinute(0);
    arduinoAlck.alarmIsSet        = false;
    arduinoAlck.brightnessRoutine = true;
}
void loop() {
    arduinoAlck.execute();
}
