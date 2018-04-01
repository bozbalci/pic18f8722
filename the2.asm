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
t0_times    RES 1
move_ball   RES 1
wreg_ctx    RES 1
p1_score    RES 1
p2_score    RES 1


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

    ; Configure PORT[A-F] as digital outputs (LEDs).
    CLRF    PORTA
    CLRF    TRISA
    CLRF    PORTB
    CLRF    TRISB
    CLRF    PORTC
    CLRF    TRISC
    CLRF    PORTD
    CLRF    TRISD
    CLRF    PORTE
    CLRF    TRISE
    CLRF    PORTF
    CLRF    TRISF
    MOVLW   0x0F
    MOVWF   ADCON1

    ; Configure PORTG as input (used to move the paddles).
    CLRF    PORTG
    MOVLW   0x0F
    MOVWF   TRISG

    ; Configure PORTJ and PORTH as outputs (7-segment display).
    CLRF    PORTJ
    CLRF    PORTH
    CLRF    TRISJ
    CLRF    TRISH

    MOVLW   0xFF
    MOVWF   PORTJ
    MOVWF   PORTH

    MOVLW   b'01100111'
    MOVWF   PORTJ


    ; Enable interrupts!
    BSF     INTCON,TMR0IE
    BSF     INTCON,GIE


main:
    TSTFSZ  move_ball
    CLRF    move_ball
    GOTO    main

update_score_view:
    CLRF    PORTH
    BSF     PORTH,2
    MOVF    p1_score,W
    CALL    get_score_repr
    CLRF    PORTH
    BSF     PORTH,0
    MOVF    p1_score,W
    CALL    get_score_repr
    RETURN

get_score_repr:
    RLNCF   WREG
    ADDLW   0x08
    ADDWF   PCL,W
    BC      _carry
    MOVWF   PCL
_carry:
    INCF    PCLATH,F
    MOVWF   PCL
    BRA     display_0
    BRA     display_1
    BRA     display_2
    BRA     display_3
    BRA     display_4
    BRA     display_5

display_0:
    MOVLW   b'00111111'
    MOVWF   PORTJ
    RETURN
display_1:
    MOVLW   b'00000110'
    MOVWF   PORTJ
    RETURN
display_2:
    MOVLW   b'01011011'
    MOVWF   PORTJ
    RETURN
display_3:
    MOVLW   b'01001111'
    MOVWF   PORTJ
    RETURN
display_4:
    MOVLW   b'01100110'
    MOVWF   PORTJ
    RETURN
display_5:
    MOVLW   b'01101101'
    MOVWF   PORTJ
    RETURN

timer0_handler:
    MOVWF   wreg_ctx
    MOVLW   0x00
    MOVWF   TMR0L
    BCF     INTCON,TMR0IF
    DCFSNZ  t0_times,F
    BRA     compl_move_ball
    MOVF    wreg_ctx, W
    RETFIE
compl_move_ball:
    MOVLW   0xFF
    MOVWF   move_ball
    MOVLW   T0_TIMES_DEFAULT
    MOVWF   t0_times
    MOVF    wreg_ctx, W
    RETFIE

 END
