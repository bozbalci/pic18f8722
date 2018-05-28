#include "common.h"
#include "cmd.h"

TASK(TASK0) 
{
    while (1)
    {
        WaitEvent(RECV_EVENT);
        ClearEvent(RECV_EVENT);

        // TODO Make sure this mutual exclusion works
        if (GetResource(0) == E_OK)
        {
            cmdobject_frombuffer(recv_buf, &cin);

            ReleaseResource(0);
        }

        switch (cin.active)
        {
            case CT_GO:
                SetEvent(TASK1_ID, GO_EVENT);
                break;

            case CT_RESPONSE:
                SetEvent(TASK3_ID, MOTION_EVENT);
                break;

            case CT_ALERT:
                SetEvent(TASK2_ID, ALERT_EVENT);
                break;
        }
    }
}
