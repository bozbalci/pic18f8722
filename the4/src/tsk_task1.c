#include "common.h"

TASK(TASK1) 
{
    WaitEvent(GO_EVENT);
    ClearEvent(GO_EVENT);

    SetRelAlarm(ALARM_TSK1, 50, 50);
    while(1)
    {
        WaitEvent(ALARM_EVENT);
        ClearEvent(ALARM_EVENT);

        send_buf[send_idx++] = '$';
        send_buf[send_idx++] = 'F';
        send_buf[send_idx++] = ':';
        TXSTA1bits.TXEN = 1;
    }

    TerminateTask();
}

#if 0
#include "common.h"

/**********************************************************************
 * ----------------------- LOCAL FUNCTIONS ----------------------------
 **********************************************************************/
/* A silly function for TASK1 */
void foo1()
{
    unsigned int n = 60;
    char i;
    for (i = 1; i >= 0; i--) {
        // transmit buffer is used to hold the resulting string
        // because this value will be transmitted to the PC
        transmitBuffer[i] = (n % 10) + '0';
        n /= 10;
    }
}


/**********************************************************************
 * ------------------------------ TASK1 -------------------------------
 *
 * Prepares the data to be transmitted
 *
 **********************************************************************/
TASK(TASK1) 
{
    unsigned char count = 0;

   // SetRelAlarm(ALARM_TSK1, 100, 200);
    while(1) {

        while(systemState != _STATE1);  // FIXME: Busy wait

        WaitEvent(ALARM_EVENT);
        ClearEvent(ALARM_EVENT);

        count++;
        if (count == 5) {       // 5 means 1 s time interval
            count = 0;
            transmitCount = 0;
            foo1();             // FIXME: I do bad things without mutual exclusion
            TXSTA1bits.TXEN = 1;// enable transmitter
        }
    }
    TerminateTask();
}

/* End of File : tsk_task1.c */
#endif
