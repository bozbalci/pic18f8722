#include <stdio.h>

#define CMD_START           '$'
#define CMD_DELIMITER       ','
#define CMD_END             ':'
#define CMD_GO              "GO"
#define CMD_ALERT           "A"
#define CMD_RESPONSE        "D"
#define CMD_DIR_E           "E"
#define CMD_DIR_W           "W"
#define CMD_DIR_S           "S"
#define CMD_DIR_N           "N"
#define CMD_DIR_SE          "SE"
#define CMD_DIR_NE          "NE"
#define CMD_DIR_SW          "SW"
#define CMD_DIR_NW          "NW"
#define CMD_DIR_K           "K"
#define CMD_MAX_LENGTH      32
#define ALERT_MAX_LENGTH    32

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

enum cmdtype
{
    CT_GOCMD,
    CT_RESPONSECMD,
    CT_ALERTCMD
};

enum cellinfo
{
    CI_FREE,
    CI_OBSTACLE,
    CI_KEY
};

struct cmdobject {
    union {
        struct {
        } go;
        struct {
            char id;
            char n[ALERT_MAX_LENGTH];
        } alert;
        struct {
            char posx, posy, rot;
            enum cellinfo top, right, bottom, left;
            enum direction dir;
        } response;
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
parsecmd(char *buf, struct cmdobject *co)
{
    int length;

    buf++;

    if (!_str_ndiff(buf, CMD_GO, 2))
    {
        co->active = CT_GOCMD;
    }
    else if (!_str_ndiff(buf, CMD_RESPONSE, 1))
    {
        co->active = CT_RESPONSECMD;

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

        length = _str_find(buf, CMD_END, CMD_MAX_LENGTH);

        if (!_str_ndiff(buf, "E",  length))  { co->cmd.response.dir = DIR_E;  return; }
        if (!_str_ndiff(buf, "W",  length))  { co->cmd.response.dir = DIR_W;  return; }
        if (!_str_ndiff(buf, "S",  length))  { co->cmd.response.dir = DIR_S;  return; }
        if (!_str_ndiff(buf, "N",  length))  { co->cmd.response.dir = DIR_N;  return; }
        if (!_str_ndiff(buf, "SE", length))  { co->cmd.response.dir = DIR_SE; return; }
        if (!_str_ndiff(buf, "NE", length))  { co->cmd.response.dir = DIR_NE; return; }
        if (!_str_ndiff(buf, "SW", length))  { co->cmd.response.dir = DIR_SW; return; }
        if (!_str_ndiff(buf, "NW", length))  { co->cmd.response.dir = DIR_NW; return; }
        if (!_str_ndiff(buf, "K",  length))  { co->cmd.response.dir = DIR_K;  return; }
    }
    else if (!_str_ndiff(buf, CMD_ALERT, 1))
    {
        co->active = CT_ALERTCMD;

        buf++;

        length = _str_find(buf, CMD_DELIMITER, CMD_MAX_LENGTH);
        co->cmd.alert.id = _str_to_int(buf, length);
        buf += length + 1;
        
        length = _str_find(buf, CMD_END, CMD_MAX_LENGTH);
        _str_ncopy(buf, co->cmd.alert.n, length);
    }
}

int
main(int argc, char **argv)
{
    struct cmdobject co;
    
    parsecmd("$GO:", &co);
    printf("$GO:\n");
    printf("%d\n", co.active);
    printf("\n");

    parsecmd("$D3,5,14,1201NW:", &co);
    printf("$D3,5,14,1201NW:\n");
    printf("%d\n", co.active);
    printf("x = %d\n", co.cmd.response.posx);
    printf("y = %d\n", co.cmd.response.posy);
    printf("z = %d\n", co.cmd.response.rot);
    printf("a = %d\n", co.cmd.response.top);
    printf("b = %d\n", co.cmd.response.right);
    printf("c = %d\n", co.cmd.response.bottom);
    printf("d = %d\n", co.cmd.response.left);
    printf("dir = %d\n", co.cmd.response.dir);
    printf("\n");
    
    parsecmd("$A3,53413:", &co);
    printf("$A3,53413:\n");
    printf("%d\n", co.active);
    printf("x = %d\n", co.cmd.alert.id);
    printf("y = %s\n", co.cmd.alert.n);
    printf("\n");

    return 0;
}
