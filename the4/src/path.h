#ifndef PATH_H
#define PATH_H

#include "cmd.h"

#define MAP_WIDTH   15
#define MAP_HEIGHT  3
#define SENSOR_FREQ 10

struct robotstate
{
    char posx, posy, rot, target_rot;
    char can_pickup_key;

    char have_key;
    char reached_door;
    char sensor_ctr;

    char to_compute;
};

void
robotstate_update(struct robotstate *rs, const struct cmdobject_in *co);

void
robotstate_dispatch(struct robotstate *rs, struct cmdobject_out *co);

#endif // PATH_H
