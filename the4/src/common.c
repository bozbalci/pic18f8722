#include "common.h"

char send_buf[256];       // holds the bytes to be transmitted

char recv_buf[256];
unsigned char recv_idx = 0;

struct cmdobject_in cin;
struct cmdobject_out cout;
struct robotstate rs;

/* transmits data using serial communication */
void data_send(void)
{
    static unsigned char i = 0;

    if (send_buf[i])
    {
        TXREG1 = send_buf[i++];
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
