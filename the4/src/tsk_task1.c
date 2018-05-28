#include "common.h"

TASK(TASK1) 
{
    WaitEvent(GO_EVENT);
    ClearEvent(GO_EVENT);

    SetRelAlarm(ALARM_TSK1, 50, 50);

    while (1)
    {
        // TODO Wait for command ready signal as well!
        WaitEvent(ALARM_EVENT);
        ClearEvent(ALARM_EVENT);

        if (GetResource(1) == E_OK)
        {
            cmdobject_tobuffer(send_buf, cout);

            ReleaseResource(1);
        }

        TXSTA1bits.TXEN = 1;
    }

    TerminateTask();
}
