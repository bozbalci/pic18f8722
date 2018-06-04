#include "common.h"

TASK(TASK1) 
{
    WaitEvent(GO_EVENT);
    ClearEvent(GO_EVENT);

    // Send the first sense command here.
    cout.cmd.motion.mt = MT_SENSE;
    cout.active = CT_MOTION;

    SetRelAlarm(ALARM_TSK1, 50, 50);

    while (1)
    {
        // TODO Wait for command ready signal as well!
        WaitEvent(ALARM_EVENT);
        ClearEvent(ALARM_EVENT);
        
        if (cout.active == CT_MOTION)
        {
            // Every 10 commands, instead of sending the appropriate response
            // for the motion, just send a SENSE command.
            rs.sensor_ctr++;

            if (rs.sensor_ctr == SENSOR_FREQ)
            {
                rs.sensor_ctr = 0;
                cout.cmd.motion.mt = MT_SENSE;
            }
        }

        if (GetResource(1) == E_OK)
        {
            cmdobject_tobuffer(send_buf, cout);

            ReleaseResource(1);
        }

        TXSTA1bits.TXEN = 1;
    }

    TerminateTask();
}
