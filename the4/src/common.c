#include "common.h"

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

    static unsigned char can_accept_start = 1;
    static unsigned char can_accept_end = 0;

    if (send_buf[i])
    {
        TXREG1 = send_buf[i];

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
