#ifndef CMD_H
#define CMD_H

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
            char unused;
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
            char unused;
        } end;
        struct
        {
            enum motiontype mt;
        } motion;
        struct
        {
            char unused;
        } pick;
        struct
        {
            char id;
            char n[COMPUTE_MAX_LENGTH];
        } compute;
    } cmd;

    enum cmdtype active;
};

void
cmdobject_frombuffer(const char *buf, struct cmdobject_in *co);

void
cmdobject_tobuffer(char *buf, struct cmdobject_out co);

#endif // CMD_H
