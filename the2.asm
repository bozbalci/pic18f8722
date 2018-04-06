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
t0_times              RES 1
move_ball             RES 1
wreg_ctx              RES 1
p1_score              RES 1
p2_score              RES 1
paddle                RES 1
p1_paddle             RES 1
p2_paddle             RES 1
p1uc                  RES 1
p1dc                  RES 1
p2uc                  RES 1
p2dc                  RES 1
ball_direction        RES 1
handle_collision      RES 1
ball_a                RES 1
ball_b                RES 1
ball_c                RES 1
ball_d                RES 1
ball_e                RES 1
ball_f                RES 1
reinitialize_promise  RES 1


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

    CLRF    p1_score
    CLRF    p2_score

    CLRF    p1uc
    CLRF    p1dc
    CLRF    p2uc
    CLRF    p2dc
    CLRF    reinitialize_promise
    COMF    reinitialize_promise,F

    CALL    reinitialize_game

    CLRF    handle_collision

    ; Enable interrupts!
    BSF     INTCON,TMR0IE
    BSF     INTCON,GIE


main:
    CALL    do_move_ball
    CALL    do_handle_collision
    CALL    do_end_game

    CALL    reinitialize_game

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


do_move_ball:
    BTFSS   move_ball,0
    RETURN

    CLRF    move_ball

    TSTFSZ  ball_direction
    BRA     _move_right
    BRA     _move_left
_move_left:
    TSTFSZ  ball_b
    COMF    handle_collision,F
    CALL    ball_left
    ; TODO: Up/down
    RETURN
_move_right:
    TSTFSZ  ball_e
    COMF    handle_collision,F
    CALL    ball_right
    ; TODO: Up/down
    RETURN

ball_left:
    MOVF    ball_b,W
    CLRF    ball_b
    IORWF   ball_a,F
    MOVF    ball_c,W
    CLRF    ball_c
    IORWF   ball_b,F
    MOVF    ball_d,W
    CLRF    ball_d
    IORWF   ball_c,F
    MOVF    ball_e,W
    CLRF    ball_e
    IORWF   ball_d,F
    MOVF    ball_f,W
    CLRF    ball_f
    IORWF   ball_e,F
    RETURN

ball_right:
    MOVF    ball_e,W
    CLRF    ball_e
    IORWF   ball_f,F
    MOVF    ball_d,W
    CLRF    ball_d
    IORWF   ball_e,F
    MOVF    ball_c,W
    CLRF    ball_c
    IORWF   ball_d,F
    MOVF    ball_b,W
    CLRF    ball_b
    IORWF   ball_c,F
    MOVF    ball_a,W
    CLRF    ball_a
    IORWF   ball_b,F
    RETURN


do_handle_collision:
    BTFSS   handle_collision,0
    RETURN

    CLRF    handle_collision

    TSTFSZ  ball_direction
    BRA     _right_check
    BRA     _left_check
_left_check:
    MOVF    p1_paddle,W
    ANDWF   ball_a,W
    TSTFSZ  WREG
    BRA     _ball_collided
    BRA     _p2_scored
_right_check:
    MOVF    p2_paddle,W
    ANDWF   ball_f,W
    TSTFSZ  WREG
    BRA     _ball_collided
    BRA     _p1_scored
_p1_scored:
    INCF    p1_score,F
    COMF    reinitialize_promise,F
    RETURN
_p2_scored:
    INCF    p2_score,F
    COMF    reinitialize_promise,F
    RETURN
_ball_collided:
    COMF    ball_direction,F
    RETURN


reinitialize_game:
    BTFSS   reinitialize_promise,0
    RETURN

    CLRF    reinitialize_promise

    MOVLW   b'00011100'
    MOVWF   p1_paddle
    MOVWF   p2_paddle

    ; Initialize the ball
    CLRF    ball_a
    CLRF    ball_b
    CLRF    ball_c
    CLRF    ball_d
    CLRF    ball_e
    CLRF    ball_f
    BSF     ball_d,3

    ; 0x00 -> left, 0xFF -> right
    CLRF    ball_direction
    RETURN


do_end_game:
    MOVLW   d'5'
    CPFSEQ  p1_score
    BRA     _check_p2_score
    BRA     _end_game
_check_p2_score:
    CPFSEQ  p2_score
    RETURN
    BRA     _end_game
_end_game:
    ; halt and catch fire
    BCF     INTCON,GIE
    CALL    update_game_view
_end_loop:
    CALL    update_score_view
    GOTO    _end_loop


update_game_view:
    CLRF    PORTA
    CLRF    PORTB
    CLRF    PORTC
    CLRF    PORTD
    CLRF    PORTE
    CLRF    PORTF

    MOVF    p1_paddle,W
    IORWF   PORTA,F
    MOVF    p2_paddle,W
    IORWF   PORTF,F

    MOVF    ball_a,W
    IORWF   PORTA,F
    MOVF    ball_b,W
    IORWF   PORTB,F
    MOVF    ball_c,W
    IORWF   PORTC,F
    MOVF    ball_d,W
    IORWF   PORTD,F
    MOVF    ball_e,W
    IORWF   PORTE,F
    MOVF    ball_f,W
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
    MOVF    wreg_ctx,W
    RETFIE
_compl_move_ball:
    MOVLW   0xFF
    MOVWF   move_ball
    MOVLW   T0_TIMES_DEFAULT
    MOVWF   t0_times
    MOVF    wreg_ctx,W
    RETFIE

 END
