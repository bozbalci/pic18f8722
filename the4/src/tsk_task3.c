#include "common.h"
#include "path.h"
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

        robotstate_update(&rs, &cin_copy);

        // TODO Implement motion planning

        if (GetResource(1) == E_OK)
        {
            robotstate_dispatch(&rs, &cout);

            ReleaseResource(1);
        }
    }
}
