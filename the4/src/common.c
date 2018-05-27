#include "common.h"

char systemState = _WAITING;    // state of the system; _WAITING or _OPERATING
char transmitBuffer[256];       // holds the bytes to be transmitted
char transmitCount;             // index to the transmitBuffer array

unsigned char recv_buf[256];
unsigned char recv_idx = 0;

/* transmits data using serial communication */
void transmitData()
{
    if (transmitCount < 2) {
        TXREG1 = transmitBuffer[transmitCount];
        transmitCount++;
    }
    else {                  // all the bytes have been sent
        TXSTA1bits.TXEN = 0;// disable transmitter, will be enabled again in 250 msecs
    }
}

/* Invoked when receive interrupt occurs; meaning that data is received */
void data_recv()
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
