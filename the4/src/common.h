#ifndef _COMMON_H
#define COMMON_H

#include "device.h"
#include "cmd.h"
#include "str.h"

/***********************************************************************
 * ------------------------ Timer settings -----------------------------
 **********************************************************************/
#define _10MHZ	63320
#define _16MHZ	61768
#define _20MHZ	60768
#define _32MHZ	57768
#define _40MHZ 	55768

/***********************************************************************
 * ----------------------------- Events --------------------------------
 **********************************************************************/
#define ALARM_EVENT       0x80
#define RECV_EVENT        0x10
#define GO_EVENT          0x20

/***********************************************************************
 * ----------------------------- Task ID -------------------------------
 **********************************************************************/
/* Info about the tasks:
 * TASK0: USART
 * TASK1: USART
 */
#define TASK0_ID             1
#define TASK1_ID             2

/* Priorities of the tasks */
#define TASK0_PRIO           8
#define TASK1_PRIO           7

#define ALARM_TSK0           0
#define ALARM_TSK1           1

/**********************************************************************
 * ----------------------- GLOBAL DEFINITIONS -------------------------
 **********************************************************************/

/* System States */
#define _WAITING	0		// waiting state
#define _STATE0 	1		// operating state for task 0
#define _STATE1 	2		// operating state for task 1


/**********************************************************************
 * ----------------------- FUNCTION PROTOTYPES ------------------------
 **********************************************************************/
/* transmits data using serial communication */
void data_send(void);
/* Invoked when receive interrupt occurs; meaning that data is received */
void data_recv(void);


extern enum state_enum {
    STATE_IDLE,
    STATE_ACTIVE
} sim_state;

extern char send_buf[];	// holds the bytes to be transmitted/displayed. format: XXYYY

extern char recv_buf[];
extern unsigned char recv_idx;

extern struct cmdobject_in cin;
extern struct cmdobject_out cout;

#endif
