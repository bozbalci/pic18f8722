#include "common.h"

enum state_enum sim_state = STATE_IDLE;

unsigned char send_buf[256];       // holds the bytes to be transmitted
unsigned char send_idx = 0;

unsigned char recv_buf[256];
unsigned char recv_idx = 0;

/* transmits data using serial communication */
void data_send(void)
{
    static unsigned char i = 0;

    if (i < send_idx)
    {
        TXREG1 = send_buf[i++];
    }
    else
    {
        TXSTA1bits.TXEN = 0;
        send_idx = 0;
        i = 0;
    }
}

/* Invoked when receive interrupt occurs; meaning that data is received */
void data_recv(void)
{
    unsigned char rxbyte = RCREG1;

    recv_buf[recv_idx++] = rxbyte;

    if (rxbyte == ':')
    {
        SetEvent(TASK0_ID, RECV_EVENT);
        recv_buf[recv_idx] = '\0';
        recv_idx = 0;
    }
}
