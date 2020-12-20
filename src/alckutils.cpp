#include "alckutils.h"
#include <WString.h>
#include <cstring>

bool haschari(char *str, char c) {
    for (int k = 0; k < strlen(str); k++) {
        if (tolower(str[k]) == c) {
            return true;
        }
    }
    return false;
}
bool stringToAlarmTime(alck *aptr, char *str) {
    short numsfound = 0;
    char timebuffer[5];
    for (int j = 0; j < strlen(str) && numsfound < 4; j++) {
        if (toascii(str[j]) >= 0x0 && toascii(str[j]) <= 0x9) {
            timebuffer[numsfound++] = str[j];
        }
    }
    if (toascii(timebuffer[0]) > 1) {
        for (int k = strlen(timebuffer); k > 0; k--) {
            timebuffer[k] = timebuffer[k - 1];
        }
        timebuffer[0] = toascii(0x0);
    }

    String(atoi(timebuffer) + haschari(str, 'P') ? 1200 : 0).toCharArray(timebuffer, 5);

    if (atoi(timebuffer) >= 0 && atoi(timebuffer) <= 2359) {
        aptr->wakeTargetOffset.setHour(atoi(timebuffer) / 100);
        aptr->wakeTargetOffset.setMinute(atoi(timebuffer) - (aptr->wakeTargetOffset.getHour() * 100));
        return true;
    }
    return false;
}
