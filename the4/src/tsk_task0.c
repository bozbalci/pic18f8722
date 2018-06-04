#include "common.h"
#include "cmd.h"
#include "path.h"

TASK(TASK0) 
{
    /* configure USART transmitter/receiver */
    SPBRG1 = 21;		// for 40 MHz, to have 115200 baud rate, it should be 21
    TXSTA1 = 0x04;      // (= 00000100) 8-bit transmit, transmitter NOT enabled,
    // asynchronous, high speed mode
    RCSTA1 = 0x90;      // (= 10010000) 8-bit receiver, receiver enabled,
    // continuous receive, serial port enabled

    /* configure the interrupts */
    PIE1bits.TX1IE = 1;	// enable USART transmit interrupt
    PIE1bits.RC1IE = 1;	// enable USART receive interrupt

    rs.to_compute = 1;

    LATBbits.LATB0 = 0;

    while (1)
    {
        WaitEvent(RECV_EVENT);
        ClearEvent(RECV_EVENT);

        if (GetResource(0) == E_OK)
        {
            cmdobject_frombuffer(recv_buf, &cin);

            ReleaseResource(0);
        }

        switch (cin.active)
        {
            case CT_GO:
                SetEvent(TASK1_ID, GO_EVENT);
                break;

            case CT_RESPONSE:
                SetEvent(TASK3_ID, MOTION_EVENT);
                break;

            case CT_ALERT:
                // Initiate the hash computing task only if the ID is valid
                if (rs.to_compute++ == cin.cmd.alert.id)
                    SetEvent(TASK2_ID, ALERT_EVENT);
                
                break;
        }
    }
}
