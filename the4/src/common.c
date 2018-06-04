#include "common.h"

// The following line is not for the faint of heart.
char send_buf[64]; // kim bilecek abi

char recv_buf[64];
unsigned char recv_idx = 0;

struct cmdobject_in cin;
struct cmdobject_out cout;
struct robotstate rs;

/* transmits data using serial communication */
void data_send(void)
{
    static unsigned char i = 0;

    // Read below for more information on this.
/*
    static unsigned char can_accept_start = 1;
    static unsigned char can_accept_end = 0;
*/

    if (send_buf[i])
    {
        TXREG1 = send_buf[i];
        
        // Since the transmission interrupts are very peculiar, the following state
        // machine unexpectedly reaches a state where either RB0 or RB1 is toggled.
        // This means that we are receiving interrupts BEFORE the character is
        // actually written to the output terminal. We tried to hack our way out by
        // adding a delay at the end of each transmission, but it didn't work for
        // some cases.
        //
        // :/
 /*
        if (send_buf[i] == '$' && !can_accept_start)
        {
            LATBbits.LATB0 = 1;
        }

        if (send_buf[i] == ':' && !can_accept_end)
        {
            LATBbits.LATB1 = 1;
        }

        if (send_buf[i] == '$')
        {
            can_accept_start = 0;
            can_accept_end = 1;
        }
        else if (send_buf[i] == ':')
        {
            can_accept_start = 1;
            can_accept_end = 0;
        }
*/

        i++;
    }
    else
    {
        i = 96;

        /* XXX
         * In the marvelous world of embedded systems development, there are
         * no temporary hacks. This is the pinnacle of engineering.
         *
         * What the fuck?
         */

        while (++i)
            ;

        TXSTA1bits.TXEN = 0;
    }
}

/* Invoked when receive interrupt occurs; meaning that data is received */
void data_recv(void)
{
    char rxbyte = RCREG1;

    recv_buf[recv_idx++] = rxbyte;

    if (rxbyte == ':')
    {
        SetEvent(TASK0_ID, RECV_EVENT);
        recv_buf[recv_idx] = '\0';
        recv_idx = 0;
    }
}
