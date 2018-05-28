#include "common.h"

TASK(TASK0) 
{
    LATBbits.LATB0 ^= 1;
    for (;;)
    {
        unsigned char i;
        WaitEvent(RECV_EVENT);
        ClearEvent(RECV_EVENT);

        for (i = 0; recv_buf[i] != ':'; i++)
        {
            unsigned char times = recv_buf[i] - '0';
            SetRelAlarm(ALARM_TSK0, 1000, 200);
            while (times)
            {
                WaitEvent(ALARM_EVENT);
                ClearEvent(ALARM_EVENT);
                LATBbits.LATB0 ^= 1;
                times--;
            }
            CancelAlarm(ALARM_TSK0);
        }

        SetEvent(TASK1_ID, GO_EVENT);
    }
}
