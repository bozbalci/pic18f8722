// This is the cleanest code any student could come up with.
// It should be included as a complete listing in Steve McConnell's
// Code Complete 2, it should be taught in the SE course Ceng350
// as a prime example of readable and understandable code.
//
// Yet it doesn't work.
//
// It has a very simple task of updating the robot state and updating
// the command object. The frequency of this task is every 10 commands,
// because any rotation or forward motion will take 10 commands. It uses
// mutual exclusion to prevent other tasks from fiddling with cin and cout
// while doing its job. Perfect

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

        if (GetResource(1) == E_OK)
        {
            robotstate_dispatch(&rs, &cout);

            ReleaseResource(1);
        }
    }
}
