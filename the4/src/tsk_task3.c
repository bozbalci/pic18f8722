#include "common.h"
#include "cmd.h"

static struct cmdobject_in cin_copy;

TASK(TASK3) 
{
    while (1)
    {
        WaitEvent(MOTION_EVENT);
        ClearEvent(MOTION_EVENT);

        if (GetResource(0) == E_OK)
        {
            cin_copy = cin;

            ReleaseResource(0);
        }

        // TODO Process sensor response
        // TODO Implement motion planning
        // TODO Implement key picking logic
        // TODO Implement END command logic

        if (GetResource(1) == E_OK)
        {
            cout.active = CT_MOTION;
            cout.cmd.motion.mt = MT_FORWARD;

            ReleaseResource(1);
        }
    }
}
