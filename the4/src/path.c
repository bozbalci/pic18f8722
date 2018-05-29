#include "path.h"

void
robotstate_update(struct robotstate *rs, struct cmdobject_in co)
{
    if (co.active != CT_RESPONSE)
        return;

    rs->posx = co.cmd.response.posx;
    rs->posy = co.cmd.response.posy;
    rs->rot = co.cmd.response.rot;

    rs->can_pickup_key = (co.cmd.response.dir == DIR_K);

    // Top right corner is the location of the key, fixed
    rs->reached_door = (rs->posx == MAP_WIDTH) && (rs->posy == 0);

    if (co->cmd.response.front == CI_KEY)
    {
        rs->target_rot = rs->rot;
    }
    else if (co->cmd.response.right == CI_KEY)
    {
        rs->target_rot = rs->rot - 10;
    }
    else if (co->cmd.response.left == CI_KEY)
    {
        rs->target_rot = rs->rot + 10;
    }
    else if (co->cmd.response.back == CI_KEY)
    {
        rs->target_rot = rs->rot - 20;
    }

    rs->target_rot = 0;
}

void
robotstate_dispatch(struct robotstate *rs, struct cmdobject_out *co)
{
    sensor_ctr++;

    if (sensor_ctr == SENSOR_FREQ)
    {
        sensor_ctr = 0;

        co->active = CT_MOTION;
        co->cmd.motion.mt = MT_SENSE;

        return;
    }

    if (rs->have_key && rs->reached_door)
    {
        co->active = CT_END;

        return;
    }

    if (!rs->have_key && rs->can_pickup_key)
    {
        rs->have_key = 1;
        rs->can_pickup_key = 0;
        co->active = CT_PICK;

        return;
    }

    // Still rotating?
    if (target_rot != rot)
    {
        co->active = CT_MOTION;

        if (target_rot > rot)
            co->cmd.motion.mt = MT_LEFT;
        else
            co->cmd.motion.mt = MT_RIGHT;

        return;
    }
    else
    {
        co->active = CT_MOTION;
        co->cmd.motion.mt = MT_FORWARD;

        return
    }
}
