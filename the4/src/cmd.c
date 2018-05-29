#include "cmd.h"
#include "str.h"

enum cellinfo _char_to_ci(char c)
{
    if (c == '0')
        return CI_FREE;
    else if (c == '1')
        return CI_OBSTACLE;

    return CI_KEY;
}

int
_str_to_int(const char *buf, char nbytes)
{
    int result;
    char i;

    result = 0;

    for (i = 0; i < nbytes; i++)
        result = result * 10 + buf[i] - '0';

    return result;
}

char
_str_find(const char *buf, char target, char nbytes)
{
    char i;

    for (i = 0; i < nbytes; i++)
        if (buf[i] == target)
            return i;

    return -1;
}

void
cmdobject_frombuffer(const char *buf, struct cmdobject_in *co)
{
    int length;

    buf++;

    if (!cstrncmp(buf, CMD_GO, 2))
    {
        co->active = CT_GO;
    }
    else if (!cstrncmp(buf, CMD_RESPONSE, 1))
    {
        co->active = CT_RESPONSE;

        buf++;

        length = _str_find(buf, CMD_DELIMITER, CMD_MAX_LENGTH);
        co->cmd.response.posx = _str_to_int(buf, length);
        buf += length + 1;

        length = _str_find(buf, CMD_DELIMITER, CMD_MAX_LENGTH);
        co->cmd.response.posy = _str_to_int(buf, length);
        buf += length + 1;

        length = _str_find(buf, CMD_DELIMITER, CMD_MAX_LENGTH);
        co->cmd.response.rot = _str_to_int(buf, length);
        buf += length + 1;

        co->cmd.response.front = _char_to_ci(buf[0]);
        co->cmd.response.right = _char_to_ci(buf[1]);
        co->cmd.response.back  = _char_to_ci(buf[2]);
        co->cmd.response.left  = _char_to_ci(buf[3]);

        buf += 4; /* skip comma */

        length = _str_find(buf, CMD_FINAL, CMD_MAX_LENGTH);

        if (!cstrncmp(buf, CMD_DIR_E,  length))
            { co->cmd.response.dir = DIR_E;  return; }
        if (!cstrncmp(buf, CMD_DIR_W,  length))
            { co->cmd.response.dir = DIR_W;  return; }
        if (!cstrncmp(buf, CMD_DIR_S,  length))
            { co->cmd.response.dir = DIR_S;  return; }
        if (!cstrncmp(buf, CMD_DIR_N,  length))
            { co->cmd.response.dir = DIR_N;  return; }
        if (!cstrncmp(buf, CMD_DIR_SE, length))
            { co->cmd.response.dir = DIR_SE; return; }
        if (!cstrncmp(buf, CMD_DIR_NE, length))
            { co->cmd.response.dir = DIR_NE; return; }
        if (!cstrncmp(buf, CMD_DIR_SW, length))
            { co->cmd.response.dir = DIR_SW; return; }
        if (!cstrncmp(buf, CMD_DIR_NW, length))
            { co->cmd.response.dir = DIR_NW; return; }
        if (!cstrncmp(buf, CMD_DIR_K,  length))
            { co->cmd.response.dir = DIR_K;  return; }
    }
    else if (!cstrncmp(buf, CMD_ALERT, 1))
    {
        co->active = CT_ALERT;

        buf++;
        co->cmd.alert.id = *buf - '0';
        buf += 2; /* skip comma */
        length = _str_find(buf, CMD_FINAL, CMD_MAX_LENGTH);
        strcpy_ram2ram(co->cmd.alert.n, buf);
        co->cmd.alert.n[length] = '\0';
    }
}

void
cmdobject_tobuffer(char *buf, struct cmdobject_out co)
{
    int length;

    switch (co.active)
    {
        case CT_END:
            strcpy_const2ram(buf, CMD_END_P);

            break;

        case CT_MOTION:
            switch (co.cmd.motion.mt)
            {
                case MT_FORWARD:
                    strcpy_const2ram(buf, CMD_FORWARD_P);

                    break;

                case MT_RIGHT:
                    strcpy_const2ram(buf, CMD_RIGHT_P);

                    break;

                case MT_LEFT:
                    strcpy_const2ram(buf, CMD_LEFT_P);

                    break;

                default: /* MT_SENSE */
                    strcpy_const2ram(buf, CMD_SENSE_P);

                    break;
            }

            break;

        case CT_PICK:
            strcpy_const2ram(buf, CMD_PICK_P);

            break;

        default: /* CT_COMPUTE */
            strcpy_const2ram(buf, "$C");
            buf += 2;
            *buf = '0' + co.cmd.compute.id;
            buf += 1;
            strcpy_const2ram(buf, ",");
            buf += 1;
            length = _str_find(co.cmd.compute.n, '\0', COMPUTE_MAX_LENGTH);
            strcpy_ram2ram(buf, co.cmd.compute.n);
            buf += length;
            strcpy_const2ram(buf, ":");

            break;
    }
}
