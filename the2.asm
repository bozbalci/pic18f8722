#include "p18f8722.inc"

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


 ORG 0x00
 GOTO init
 ORG 0x08
 GOTO timer0_handler


init:
    CLRF    INTCON
    CLRF    INTCON2

    ; prescale = 128
    ; using 16 bits (obviously)
    CLRF    T0CON
    BSF     T0CON,TMR0ON
    MOVLW   b'00000110'
    IORWF   T0CON,F
    CALL    timer0_reset

    BSF     INTCON,TMR0IE
    BSF     INTCON,GIE


main:
    GOTO    main


; timer0_reset moves the integer 41992 by breaking it into two 8-bit registers,
; and then returns.
timer0_reset:
    MOVLW   b'10100100'
    MOVWF   TMR0H
    MOVLW   b'00001000'
    MOVWF   TMR0L
    RETURN


timer0_handler:
    CALL    timer0_reset
    BCF     INTCON,TMR0IF
    RETFIE


 END
