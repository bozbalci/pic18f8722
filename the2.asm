#include "p18f8722.inc"

#define T0_TIMES_DEFAULT    d'46'

 CONFIG OSC = HSPLL, FCMEN = OFF, IESO = OFF
 CONFIG PWRT = OFF, BOREN = OFF, BORV = 3
 CONFIG WDT = OFF, WDTPS = 32768
 CONFIG MODE = MC, ADDRBW = ADDR20BIT, DATABW = DATA16BIT, WAIT = OFF
 CONFIG CCP2MX = PORTC, ECCPMX = PORTE, LPT1OSC = OFF, MCLRE = ON
 CONFIG STVREN = ON, LVP = OFF, BBSIZ = BB2K, XINST = OFF
 CONFIG CP0 = OFF, CP1 = OFF, CP2 = OFF, CP3 = OFF, CP4 = OFF, CP5 = OFF
 CONFIG CP6 = OFF, CP7 = OFF
 CONFIG CPB = OFF, CPD = OFF
 CONFIG WRT0 = OFF, WRT1 = OFF, WRT2 = OFF, WRT3 = OFF, WRT4 = OFF
 CONFIG WRT5 = OFF, WRT6 = OFF, WRT7 = OFF
 CONFIG WRTC = OFF, WRTB = OFF, WRTD = OFF
 CONFIG EBTR0 = OFF, EBTR1 = OFF, EBTR2 = OFF, EBTR3 = OFF, EBTR4 = OFF
 CONFIG EBTR5 = OFF, EBTR6 = OFF, EBTR7 = OFF
 CONFIG EBTRB = OFF

 UDATA_ACS
t0_times   RES 1
move_ball  RES 1


 ORG 0x00
 GOTO init
 ORG 0x08
 GOTO timer0_handler


init:
    ; Clear interrupt control registers.
    CLRF    INTCON
    CLRF    INTCON2

    ; Set Timer0 as 8-bits, set prescale=256.
    CLRF    T0CON
    BSF     T0CON,TMR0ON
    MOVLW   b'01000111'
    IORWF   T0CON,F

    ; Clear some files.
    CLRF    TMR0L
    CLRF    move_ball

    ; Initialize move_ball counter with 46. Timer0 interrupts every
    ; 6.55 ms, so we need 46 times that to achieve a delay of 300 ms.
    MOVLW   T0_TIMES_DEFAULT
    MOVWF   t0_times

    ; Enable interrupts!
    BSF     INTCON,TMR0IE
    BSF     INTCON,GIE


main:
    TSTFSZ  move_ball
    CLRF    move_ball
    GOTO    main


timer0_handler:
    CLRF    TMR0L
    BCF     INTCON,TMR0IF
    DCFSNZ  t0_times,F
    GOTO    compl_move_ball
    RETFIE
compl_move_ball:
    COMF    move_ball
    MOVLW   T0_TIMES_DEFAULT
    MOVWF   t0_times
    RETFIE

 END
