#include "common.h"

TASK(TASK1) 
{
    WaitEvent(GO_EVENT);
    ClearEvent(GO_EVENT);

    SetRelAlarm(ALARM_TSK1, 50, 50);

    while (1)
    {
        // TODO Fill cout or have it filled by some other task!

        WaitEvent(ALARM_EVENT);
        ClearEvent(ALARM_EVENT);

        cmdobject_tobuffer(send_buf, cout);
        TXSTA1bits.TXEN = 1;
    }

    TerminateTask();
}
