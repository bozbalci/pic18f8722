#include "common.h"
#include "cmd.h"

static struct cmdobject_in cin_copy;
static unsigned char result[256];

void
compute_hash(unsigned char *inp, unsigned char *out);

TASK(TASK2) 
{
    while (1)
    {
        WaitEvent(ALERT_EVENT);
        ClearEvent(ALERT_EVENT);

        if (GetResource(0) == E_OK)
        {
            cin_copy = cin;

            ReleaseResource(0);
        }

        // TODO This function never returns
        compute_hash(cin_copy.cmd.alert.n, result);
        
        if (GetResource(1) == E_OK)
        {
            cout.active = CT_COMPUTE;
            cout.cmd.compute.id = cin_copy.cmd.alert.id;
            strcpy_ram2ram(cout.cmd.compute.n, result);

            ReleaseResource(1);
        }
    }
}
