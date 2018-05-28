#define CMD_START           '$'
#define CMD_DELIMITER       ','
#define CMD_FINAL           ':'
#define CMD_GO              "GO"
#define CMD_ALERT           "A"
#define CMD_RESPONSE        "D"
#define CMD_COMPUTE         "C"
#define CMD_DIR_E           "E"
#define CMD_DIR_W           "W"
#define CMD_DIR_S           "S"
#define CMD_DIR_N           "N"
#define CMD_DIR_SE          "SE"
#define CMD_DIR_NE          "NE"
#define CMD_DIR_SW          "SW"
#define CMD_DIR_NW          "NW"
#define CMD_DIR_K           "K"
#define CMD_END_P           "$END:"
#define CMD_FORWARD_P       "$F:"
#define CMD_RIGHT_P         "$R:"
#define CMD_LEFT_P          "$L:"
#define CMD_SENSE_P         "$S:"
#define CMD_PICK_P          "$P:"
#define CMD_MAX_LENGTH      32
#define ALERT_MAX_LENGTH    32
#define COMPUTE_MAX_LENGTH  32

enum direction
{
    DIR_E,
    DIR_W,
    DIR_S,
    DIR_N,
    DIR_SE,
    DIR_NE,
    DIR_SW,
    DIR_NW,
    DIR_K
};

enum cellinfo
{
    CI_FREE,
    CI_OBSTACLE,
    CI_KEY
};

enum motiontype
{
    MT_FORWARD,
    MT_RIGHT,
    MT_LEFT,
    MT_SENSE
};

enum cmdtype
{
    /* server commands */
    CT_GO,
    CT_RESPONSE,
    CT_ALERT,

    /* client commands */
    CT_END,
    CT_MOTION,
    CT_PICK,
    CT_COMPUTE
};

struct cmdobject_in
{
    union
    {
        struct
        {
            /* blank */
        } go;
        struct
        {
            char id;
            char n[ALERT_MAX_LENGTH];
        } alert;
        struct
        {
            char posx, posy, rot;
            enum cellinfo top, right, bottom, left;
            enum direction dir;
        } response;
    } cmd;

    enum cmdtype active;
};

struct cmdobject_out {
    union
    {
        struct
        {
            /* blank */
        } end;
        struct
        {
            enum motiontype mt;
        } motion;
        struct
        {
            /* blank */
        } pick;
        struct
        {
            char id;
            char n[COMPUTE_MAX_LENGTH];
        } compute;
    } cmd;

    enum cmdtype active;
};

enum cellinfo _char_to_ci(char c)
{
    if (c == '0')
        return CI_FREE;
    else if (c == '1')
        return CI_OBSTACLE;

    return CI_KEY;
}

int
_str_to_int(char *buf, ssize_t nbytes)
{
    int result;
    ssize_t i;

    result = 0;

    for (i = 0; i < nbytes; i++)
        result = result * 10 + buf[i] - '0';

    return result;
}

int
_str_find(char *buf, char target, ssize_t nbytes)
{
    ssize_t i;

    for (i = 0; i < nbytes; i++)
        if (buf[i] == target)
            return i;

    return -1;
}

int
_str_ndiff(char *str1, char *str2, ssize_t nbytes)
{
    ssize_t i = 0;

    for (i = 0; i < nbytes; i++)
        if (str1[i] != str2[i])
            return 1;

    return 0;
}

void
_str_ncopy(char *src, char *dest, ssize_t nbytes)
{
    ssize_t i = 0;

    for (i = 0; i < nbytes; i++)
        dest[i] = src[i];
}

void
_str_putchar(char *buf, char c)
{
    *buf = c;
}

void
cmdobject_frombuffer(char *buf, struct cmdobject_in *co)
{
    int length;

    buf++;

    if (!_str_ndiff(buf, CMD_GO, 2))
    {
        co->active = CT_GO;
    }
    else if (!_str_ndiff(buf, CMD_RESPONSE, 1))
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

        co->cmd.response.top    = _char_to_ci(buf[0]);
        co->cmd.response.right  = _char_to_ci(buf[1]);
        co->cmd.response.bottom = _char_to_ci(buf[2]);
        co->cmd.response.left   = _char_to_ci(buf[3]);

        buf += 4; /* skip comma */

        length = _str_find(buf, CMD_FINAL, CMD_MAX_LENGTH);

        if (!_str_ndiff(buf, CMD_DIR_E,  length))  { co->cmd.response.dir = DIR_E;  return; }
        if (!_str_ndiff(buf, CMD_DIR_W,  length))  { co->cmd.response.dir = DIR_W;  return; }
        if (!_str_ndiff(buf, CMD_DIR_S,  length))  { co->cmd.response.dir = DIR_S;  return; }
        if (!_str_ndiff(buf, CMD_DIR_N,  length))  { co->cmd.response.dir = DIR_N;  return; }
        if (!_str_ndiff(buf, CMD_DIR_SE, length))  { co->cmd.response.dir = DIR_SE; return; }
        if (!_str_ndiff(buf, CMD_DIR_NE, length))  { co->cmd.response.dir = DIR_NE; return; }
        if (!_str_ndiff(buf, CMD_DIR_SW, length))  { co->cmd.response.dir = DIR_SW; return; }
        if (!_str_ndiff(buf, CMD_DIR_NW, length))  { co->cmd.response.dir = DIR_NW; return; }
        if (!_str_ndiff(buf, CMD_DIR_K,  length))  { co->cmd.response.dir = DIR_K;  return; }
    }
    else if (!_str_ndiff(buf, CMD_ALERT, 1))
    {
        co->active = CT_ALERT;

        buf++;

        length = _str_find(buf, CMD_DELIMITER, CMD_MAX_LENGTH);
        co->cmd.alert.id = _str_to_int(buf, length);
        buf += length + 1;
        
        length = _str_find(buf, CMD_FINAL, CMD_MAX_LENGTH);
        _str_ncopy(buf, co->cmd.alert.n, length);
    }
}

void
cmdobject_tobuffer(char *buf, struct cmdobject_out co)
{
    int length;

    switch (co.active)
    {
        case CT_END:
            _str_ncopy(CMD_END_P, buf, sizeof(CMD_END_P));

            break;

        case CT_MOTION:
            switch (co.cmd.motion.mt)
            {
                case MT_FORWARD:
                    _str_ncopy(CMD_FORWARD_P, buf, sizeof(CMD_FORWARD_P));

                    break;

                case MT_RIGHT:
                    _str_ncopy(CMD_RIGHT_P, buf, sizeof(CMD_RIGHT_P));

                    break;

                case MT_LEFT:
                    _str_ncopy(CMD_LEFT_P, buf, sizeof(CMD_LEFT_P));

                    break;

                default: /* MT_SENSE */
                    _str_ncopy(CMD_SENSE_P, buf, sizeof(CMD_SENSE_P));

                    break;
            }

            break;

        case CT_PICK:
            _str_ncopy(CMD_PICK_P, buf, sizeof(CMD_PICK_P));

            break;

        default: /* CT_COMPUTE */
            _str_putchar(buf, '$');
            _str_putchar(buf + 1, 'C');
            _str_putchar(buf + 2, '0' + co.cmd.compute.id);
            _str_putchar(buf + 3, ',');

            buf += 4;

            length = _str_find(co.cmd.compute.n, '\0', COMPUTE_MAX_LENGTH);
            _str_ncopy(co.cmd.compute.n, buf, length);

            buf += length;

            _str_putchar(buf, ':');
            _str_putchar(buf + 1, '\0');

            break;
    }
}
