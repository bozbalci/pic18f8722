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
paddle      RES 1
p1_paddle   RES 1
p2_paddle   RES 1
p1uc        RES 1
p1dc        RES 1
p2uc        RES 1
p2dc        RES 1


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

    MOVLW   03h
    MOVWF   p1_score
    MOVLW   04h
    MOVWF   p2_score

    CLRF    p1uc

    MOVLW   b'00011100'
    MOVWF   p1_paddle
    MOVWF   p2_paddle

    ; Enable interrupts!
    BSF     INTCON,TMR0IE
    BSF     INTCON,GIE


main:
    TSTFSZ  move_ball
    CLRF    move_ball

    CALL    update_score_view

    CALL    button_p1up
    CALL    button_p1up_commit

    CALL    button_p1down
    CALL    button_p1down_commit

    CALL    button_p2up
    CALL    button_p2up_commit

    CALL    button_p2down
    CALL    button_p2down_commit

    CALL    update_game_view

    GOTO    main


update_game_view:
    CLRF    PORTA
    CLRF    PORTF

    MOVF    p1_paddle,W
    IORWF   PORTA,F
    MOVF    p2_paddle,W
    IORWF   PORTF,F

    RETURN

update_score_view:
    CLRF    PORTH
    CLRF    PORTJ
    BSF     PORTH,1
    MOVF    p1_score,W
    CALL    get_score_repr
    CALL    sfssdu
    CLRF    PORTH
    CLRF    PORTJ
    BSF     PORTH,3
    MOVF    p2_score,W
    CALL    get_score_repr
    CALL    sfssdu
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

sfssdu:
    MOVLW   0xFF
_sfssdu_body:
    DECFSZ  WREG
    BRA     _sfssdu_body
    RETURN

button_p1up:
    BTFSS   PORTG,3
    RETURN
    MOVLW   0x1
    MOVWF   p1uc
    RETURN

button_p1down:
    BTFSS   PORTG,2
    RETURN
    MOVLW   0x1
    MOVWF   p1dc
    RETURN

button_p2up:
    BTFSS   PORTG,1
    RETURN
    MOVLW   0x1
    MOVWF   p2uc
    RETURN

button_p2down:
    BTFSS   PORTG,0
    RETURN
    MOVLW   0x1
    MOVWF   p2dc
    RETURN

button_p1up_commit:
    BTFSC   PORTG,3
    RETURN

    ; If we reached here, RG3 was either
    ;   a.) released
    ;   b.) never pressed
    ;
    ; So we check if it was pressed, by using a variable.
    ; Similar functions apply this Promise-Commit pattern, so we leave out
    ; the extra comments there.
    CLRF    WREG
    CPFSEQ  p1uc
    BRA     p1u_commit
    RETURN
p1u_commit:
    CLRF    p1uc
    MOVFF   p1_paddle,paddle
    CALL    paddle_up
    MOVFF   paddle,p1_paddle
    RETURN

button_p1down_commit:
    BTFSC   PORTG,2
    RETURN
    CLRF    WREG
    CPFSEQ  p1dc
    BRA     p1d_commit
    RETURN
p1d_commit:
    CLRF    p1dc
    MOVFF   p1_paddle,paddle
    CALL    paddle_down
    MOVFF   paddle,p1_paddle
    RETURN

button_p2up_commit:
    BTFSC   PORTG,1
    RETURN
    CLRF    WREG
    CPFSEQ  p2uc
    BRA     p2u_commit
    RETURN
p2u_commit:
    CLRF    p2uc
    MOVFF   p2_paddle,paddle
    CALL    paddle_up
    MOVFF   paddle,p2_paddle
    RETURN

button_p2down_commit:
    BTFSC   PORTG,0
    RETURN
    CLRF    WREG
    CPFSEQ  p2dc
    BRA     p2d_commit
    RETURN
p2d_commit:
    CLRF    p2dc
    MOVFF   p2_paddle,paddle
    CALL    paddle_down
    MOVFF   paddle,p2_paddle
    RETURN


paddle_up:
    MOVLW   b'00000111'
    CPFSEQ  paddle
    RRNCF   paddle
    RETURN
paddle_down:
    MOVLW   b'00111000'
    CPFSEQ  paddle
    RLNCF   paddle
    RETURN
    

timer0_handler:
    MOVWF   wreg_ctx
    MOVLW   0x00
    MOVWF   TMR0L
    BCF     INTCON,TMR0IF
    DCFSNZ  t0_times,F
    BRA     _compl_move_ball
    MOVF    wreg_ctx, W
    RETFIE
_compl_move_ball:
    MOVLW   0xFF
    MOVWF   move_ball
    MOVLW   T0_TIMES_DEFAULT
    MOVWF   t0_times
    MOVF    wreg_ctx, W
    RETFIE

 END