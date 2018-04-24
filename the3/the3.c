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

volatile int t0_times;
ProgramState state;

void init(void)
{

    TRISB = 0;
    LATB = 0;

    LATE = 0;
    TRISE = 1 << 1;

    INTCON = 0;
    INTCON2 = 0;

    TMR0L = 0;
    T0CON = 0;
    TMR0ON = 1;
    T08BIT = 1;
    T0CON |= 0b111;

    TMR0IE = 1;
    ei();
}

void __interrupt(high_priority) isr(void)
{
    TMR0L = 0;
    TMR0IF = 0;
    if (++t0_times == T0_TIMES_DEFAULT) {
        PORTBbits.RB0 = ~PORTBbits.RB0;
        t0_times = 0;
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

void main(void)
{
    init();

    state = PS_INITIAL;
    
    for (;;) {
        switch (state) {
            case PS_INITIAL:
                InitLCD();
                ClearLCDScreen();
                WriteCommandToLCD(0x80);   // Goto to the beginning of the first line
                WriteStringToLCD(" $>Very  Safe<$ ");
                WriteCommandToLCD(0xC0); // Goto to the beginning of the second line
                WriteStringToLCD(" $$$$$$$$$$$$$$ ");

                state = PS_RE1WAIT;
                break;

            case PS_RE1WAIT:
                while (!PORTEbits.RE1)
                    ;
                while (PORTEbits.RE1)
                    ;

                state = PS_DELAY;
                break;

            case PS_DELAY:
                delay_3s();

                state = PS_PINSETTING;
                break;

            case PS_PINSETTING:
                PORTBbits.RB1 = 1;
                break;

            case PS_PINSET:
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
