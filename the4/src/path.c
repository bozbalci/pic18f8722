#include "path.h"

// This routine is responsible for updating the struct robotstate using
// the struct response within the struct cmdobject_in. This newly set
// information includes position, some flags (can pickup key, reached door)
// and the target rotation.
void
robotstate_update(struct robotstate *rs, const struct cmdobject_in *co)
{
    if (co->active != CT_RESPONSE)
        return;

    rs->posx = co->cmd.response.posx;
    rs->posy = co->cmd.response.posy;
    rs->rot = co->cmd.response.rot;

    rs->can_pickup_key = (co->cmd.response.dir == DIR_K);

    // Top right corner is the location of the key, fixed
    rs->reached_door = (rs->posx == MAP_WIDTH) && (rs->posy == 0);

    // IDEALLY, this is where the pathfinding is done. Currently the robot
    // always aims for setting its angle to zero. In our test builds, even
    // this didn't work properly so we didn't bother writing a complete
    // pathfinding algorithm. But hey.
    rs->target_rot = 0;
}

// This routine is responsible for filling a struct cmdobject_out using
// the new robotstate. The END and PICK commands are fully implemented.
// If there is some rotation to do, this will do so. If there is no more
// rotation to do, it will send a FORWARD command.
// Note that the SENSE command isn't sent in here. It's handled in the
// task file.
void
robotstate_dispatch(struct robotstate *rs, struct cmdobject_out *co)
{
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
    if (rs->target_rot != rs->rot)
    {
        co->active = CT_MOTION;

        if (rs->target_rot > rs->rot)
        {
            rs->rot++;
            co->cmd.motion.mt = MT_LEFT;
        }
        else
        {
            rs->rot--;
            co->cmd.motion.mt = MT_RIGHT;
        }

        return;
    }
    else
    {
        co->active = CT_MOTION;
        co->cmd.motion.mt = MT_FORWARD;

        return;
    }
}
