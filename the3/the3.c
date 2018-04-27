#pragma config OSC = HSPLL, FCMEN = OFF, IESO = OFF
#pragma config PWRT = OFF, BOREN = OFF, BORV = 3
#pragma config WDT = OFF, WDTPS = 32768
#pragma config MODE = MC, ADDRBW = ADDR20BIT, DATABW = DATA16BIT, WAIT = OFF
#pragma config CCP2MX = PORTC, ECCPMX = PORTE, LPT1OSC = OFF, MCLRE = ON
#pragma config STVREN = ON, LVP = OFF, BBSIZ = BB2K, XINST = OFF
#pragma config CP0 = OFF, CP1 = OFF, CP2 = OFF, CP3 = OFF, CP4 = OFF, CP5 = OFF
#pragma config CP6 = OFF, CP7 = OFF
#pragma config CPB = OFF, CPD = OFF
#pragma config WRT0 = OFF, WRT1 = OFF, WRT2 = OFF, WRT3 = OFF, WRT4 = OFF
#pragma config WRT5 = OFF, WRT6 = OFF, WRT7 = OFF
#pragma config WRTC = OFF, WRTB = OFF, WRTD = OFF
#pragma config EBTR0 = OFF, EBTR1 = OFF, EBTR2 = OFF, EBTR3 = OFF, EBTR4 = OFF
#pragma config EBTR5 = OFF, EBTR6 = OFF, EBTR7 = OFF
#pragma config EBTRB = OFF

#include "Includes.h"
#include "LCD.h"

#include <xc.h>
#include <stdint.h>

// TODO Uncomment the following line before submission!
#define DEBUG

#define T0_TIMES_DEFAULT 46

typedef enum
{
    PS_INITIAL,
    PS_RE1WAIT,
    PS_DELAY,
    PS_PINSETTING,
    PS_PINSET,
    PS_TEST,
    PS_TARPIT,
    PS_SUCCESS,
    PS_FAILURE
} ProgramState;

ProgramState state;

volatile uint8_t t0_times;
volatile uint8_t promise_change_digit;

/* [S]leep [F]or [S]even [S]egment [D]isplay [U]pdate */
void sfssdu(void)
{
    uint8_t i = 255;

    while (i)
    {
        i--;
    }
}

void zeg_dashes(void)
{
    PORTJ = 1 << 6;
    PORTH = 8;

    do
    {
        sfssdu();
    } while (PORTH >>= 1);
}

void init(void)
{
    // Configure RB7 and RB6 as inputs, the rest of the PORTB is unused.
    TRISB = 0b11000000;
    PORTB = 0;

    PORTE = 0;

    TRISE = 1 << 1;

    /* Configuration for the 7-segment display */
    TRISJ = 0;
    TRISH = 0;
    PORTJ = 0;
    PORTH = 0;

    INTCON = 0;
    INTCON2 = 0;

    TMR0L = 0;
    T0CON = 0;
    TMR0ON = 1;
    T08BIT = 1;
    T0CON |= 0b111;

    TMR0IE = 1;
    RBIE = 1;
    RBPU = 0;

    ei();
}

void interrupt isr(void)
{
    if (TMR0IF)
    {
        TMR0L = 0;
        TMR0IF = 0;
        t0_times++;
    }
    else if (RBIF)
    {
        /* This unorthodox method of reading PORTB is probably required to
           force a read on all of the bits in PORTB, therefore ending the
           mismatch condition and allow the RBIF bit to be cleared. */
        if (PORTB)
        {
            if (PORTBbits.RB6)
            {                
                promise_change_digit = 1;
            }
            else if (PORTBbits.RB7)
            {
                // TODO Handle this
            }
        }
        
        RBIF = 0;
    }
}

void delay_3s(void)
{
    uint8_t t1 = 0, t2 = 0, t3 = 0;
#asm
    MOVLW 0x99
    MOVWF delay_3s@t3
_loop3:
    MOVLW 0xFF
    MOVWF delay_3s@t2
_loop2:
    MOVLW 0xFF
    MOVWF delay_3s@t1
_loop1:
    DECFSZ delay_3s@t1
    BRA _loop1
    DECFSZ delay_3s@t2
    BRA _loop2
    DECFSZ delay_3s@t3
    BRA _loop3
#endasm
}

void test(void)
{
    while (!PORTB)
    {
        ;
    }
}

void main(void)
{
    int digits_entered;

    init();
    //InitLCD();

        test();

    state = PS_INITIAL;
    
    for (;;) {
        switch (state) {
            case PS_INITIAL:
                ClearLCDScreen();
                WriteCommandToLCD(0x80); // Goto to the beginning of the first line
                WriteStringToLCD(" $>Very  Safe<$ ");
                WriteCommandToLCD(0xC0); // Goto to the beginning of the second line
                WriteStringToLCD(" $$$$$$$$$$$$$$ ");

                state = PS_RE1WAIT;
                break;

            case PS_RE1WAIT:
#ifndef DEBUG
                while (!PORTEbits.RE1)
                    ;
                /* FIXME: Button release waiting */
                while (PORTEbits.RE1)
                    ;
#endif

                state = PS_DELAY;
                break;

            case PS_DELAY:
                delay_3s();

                state = PS_PINSETTING;
                break;

            case PS_PINSETTING:
                ClearLCDScreen();
                WriteCommandToLCD(0x80);
                WriteStringToLCD(" Set a pin:#### ");

                digits_entered = 0;

                while (digits_entered < 4)
                {
                    if (promise_change_digit)
                    {
                        /* Commit change_digit and release the promise.
                           PORTB change interrupt is disabled as it can
                           cause digits_entered to be incremented
                           repeatedly. */
                        RBIE = 0;

                        promise_change_digit = 0;
                        digits_entered++;

                        RBIE = 1;
                    }

                    zeg_dashes();
                }

                state = PS_PINSET;
                break;

            case PS_PINSET:
                ClearLCDScreen();
                WriteCommandToLCD(0x80);
                WriteStringToLCD("HASSAS TARTI");

                delay_3s();

                state = PS_TEST;
                break;

            case PS_TEST:
                break;

            case PS_TARPIT:
                break;

            case PS_SUCCESS:
                break;

            case PS_FAILURE:
                break;
        }
    }
}
