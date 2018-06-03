/*
 * Berk Özbalcı, 2171791
 * Yağmur Oymak, 2171783
 * 
 * Task 0 is the receiver/parser. Whenever the interrupt handler reads the command
 * terminator character, ':', it fires up an event that wakes task 0 up. The task
 * then parses the command and fills input command descriptor structure
 * struct cmdobject_in (cin) accordingly. After filling the structure, it wakes up
 * the task related to the command by firing the appropriate event.
 *
 * Task 1 is the transmitter. It fills up the send buffers according to the output
 * command descriptor, struct cmdobject_out, cout. It wakes up every 50ms by an
 * alarm and does its work.
 *
 * Task 2 is the hash calculator. It is woken up by task 0 whenever an alert command
 * is received and calculates the hash in the background (with the lowest priority).
 * When it completes the calculation, it fills the cout object with the appropriate response.
 *
 * Task 3 is the motion planner/path finder. Whenever a sensors response is received,
 * it is woken up by task 0. Then, by using the API defined in path.h,
 * it interprets the sensor respons, plans a path for the robot and fills the cout
 * object with appropriate commands.
 */

#pragma config OSC = HSPLL, FCMEN = OFF, IESO = OFF, PWRT = OFF, BOREN = OFF, WDT = OFF, MCLRE = ON, LPT1OSC = OFF, LVP = OFF, XINST = OFF, DEBUG = OFF

#include "common.h"

/**********************************************************************
 * Definition dedicated to the local functions.
 **********************************************************************/
#define DEFAULT_MODE       0


/**********************************************************************
 * Function prototypes.
 **********************************************************************/
void main(void);
void Init(void);
void StartupHook(void);
void ShutdownHook(StatusType error);
void ErrorHook(StatusType error);
void PreTaskHook(void);
void PostTaskHook(void);

extern union Timers Tmr0;

AppModeType SelectedMode;

/**********************************************************************
 * -------------------------- main function ---------------------------
 *
 * Setup the different alarms and start the kernel.
 *
 **********************************************************************/
void main(void)
{
    STKPTR = 0;
    SelectedMode = DEFAULT_MODE;
    Init();

    while(1) {
        StartOS(SelectedMode);
    }
}

/**********************************************************************
 * Clear all RAM memory and set PORTB to output mode.
 *
 * @return void
 **********************************************************************/
void Init(void)
{
    FSR0H = 0;
    FSR0L = 0;

    /* User setting : actual PIC frequency */
    Tmr0.lt = _40MHZ;

    /* Timer OFF - Enabled by Kernel */
    T0CON = 0x08;
    TMR0H = Tmr0.bt[1];
    TMR0L = Tmr0.bt[0];

    /* configure I/O ports */
    TRISD = 0;			// set all ports of PortD as output
    TRISE = 0;			// set all ports of PortE as output
    PORTD = 0;			// clear PortD
    PORTE = 0;			// clear PortE

    /* configure USART transmitter/receiver */
    SPBRG1 = 21;		// for 40 MHz, to have 115200 baud rate, it should be 21
    TXSTA1 = 0x04;      // (= 00000100) 8-bit transmit, transmitter NOT enabled,
    // asynchronous, high speed mode
    RCSTA1 = 0x90;      // (= 10010000) 8-bit receiver, receiver enabled,
    // continuous receive, serial port enabled

    /* configure the interrupts */
    INTCON = 0;			// clear interrupt register completely
    PIE1bits.TX1IE = 1;	// enable USART transmit interrupt
    PIE1bits.RC1IE = 1;	// enable USART receive interrupt
    PIR1 = 0;			// clear all peripheral flags

    INTCONbits.PEIE = 1;// enable peripheral interrupts
    INTCONbits.GIE = 1;	// globally enable interrupts

    TRISB = 0;
    LATB = 0;
}

/**********************************************************************
 * Hook routine called just before entering in kernel.
 *
 * @param error      IN The new error stored in buffer
 * @return error     Error level
 **********************************************************************/
void StartupHook(void)
{
}

/**********************************************************************
 * Hook routine called just after leaving the kernel.
 *
 * @param error      IN The last error detected by OS
 * @return void
 **********************************************************************/
void ShutdownHook(StatusType error)
{
    LATBbits.LATB0 = 1;
}

/**********************************************************************
 * Store a new error in a global buffer keeping a track of the 
 * application history.
 *
 * @param error      IN The new error stored in buffer
 * @return void
 **********************************************************************/
void ErrorHook(StatusType error)
{
    //LATBbits.LATB0 = 1;
}

/**********************************************************************
 * Hook routine called just before entering in a task.
 *
 * @return void
 **********************************************************************/
void PreTaskHook(void)
{
    //LATBbits.LATB2 = 1;
}

/**********************************************************************
 * Hook routine called just after leaving a task.
 *
 * @return void
 **********************************************************************/
void PostTaskHook(void)
{
    //LATBbits.LATB1 = 1;
}

/* End of File : main.c */
