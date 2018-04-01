#include "p18f8722.inc"

#define RA4_BOUNCE 0x80
#define RB5_BOUNCE 0x80
#define POS_CHANGE_STATE1 0x02
#define POS_CHANGE_STATE2 0xFD

; CONFIG1H
  CONFIG  OSC = HSPLL           ; Oscillator Selection bits (HS oscillator, PLL enabled (Clock Frequency = 4 x FOSC1))
  CONFIG  FCMEN = OFF           ; Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
  CONFIG  IESO = OFF            ; Internal/External Oscillator Switchover bit (Two-Speed Start-up disabled)

; CONFIG2L
  CONFIG  PWRT = OFF            ; Power-up Timer Enable bit (PWRT disabled)
  CONFIG  BOREN = OFF           ; Brown-out Reset Enable bits (Brown-out Reset disabled in hardware and software)
  CONFIG  BORV = 3              ; Brown-out Voltage bits (Minimum setting)

; CONFIG2H
  CONFIG  WDT = OFF             ; Watchdog Timer (WDT disabled (control is placed on the SWDTEN bit))
  CONFIG  WDTPS = 32768         ; Watchdog Timer Postscale Select bits (1:32768)

; CONFIG3L
  CONFIG  MODE = MC             ; Processor Data Memory Mode Select bits (Microcontroller mode)
  CONFIG  ADDRBW = ADDR20BIT    ; Address Bus Width Select bits (20-bit Address Bus)
  CONFIG  DATABW = DATA16BIT    ; Data Bus Width Select bit (16-bit External Bus mode)
  CONFIG  WAIT = OFF            ; External Bus Data Wait Enable bit (Wait selections are unavailable for table reads and table writes)

; CONFIG3H
  CONFIG  CCP2MX = PORTC        ; CCP2 MUX bit (ECCP2 input/output is multiplexed with RC1)
  CONFIG  ECCPMX = PORTE        ; ECCP MUX bit (ECCP1/3 (P1B/P1C/P3B/P3C) are multiplexed onto RE6, RE5, RE4 and RE3 respectively)
  CONFIG  LPT1OSC = OFF         ; Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
  CONFIG  MCLRE = ON            ; MCLR Pin Enable bit (MCLR pin enabled; RG5 input pin disabled)

; CONFIG4L
  CONFIG  STVREN = ON           ; Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
  CONFIG  LVP = OFF             ; Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
  CONFIG  BBSIZ = BB2K          ; Boot Block Size Select bits (1K word (2 Kbytes) Boot Block size)
  CONFIG  XINST = OFF           ; Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

; CONFIG5L
  CONFIG  CP0 = OFF             ; Code Protection bit Block 0 (Block 0 (000800, 001000 or 002000-003FFFh) not code-protected)
  CONFIG  CP1 = OFF             ; Code Protection bit Block 1 (Block 1 (004000-007FFFh) not code-protected)
  CONFIG  CP2 = OFF             ; Code Protection bit Block 2 (Block 2 (008000-00BFFFh) not code-protected)
  CONFIG  CP3 = OFF             ; Code Protection bit Block 3 (Block 3 (00C000-00FFFFh) not code-protected)
  CONFIG  CP4 = OFF             ; Code Protection bit Block 4 (Block 4 (010000-013FFFh) not code-protected)
  CONFIG  CP5 = OFF             ; Code Protection bit Block 5 (Block 5 (014000-017FFFh) not code-protected)
  CONFIG  CP6 = OFF             ; Code Protection bit Block 6 (Block 6 (01BFFF-018000h) not code-protected)
  CONFIG  CP7 = OFF             ; Code Protection bit Block 7 (Block 7 (01C000-01FFFFh) not code-protected)

; CONFIG5H
  CONFIG  CPB = OFF             ; Boot Block Code Protection bit (Boot Block (000000-0007FFh) not code-protected)
  CONFIG  CPD = OFF             ; Data EEPROM Code Protection bit (Data EEPROM not code-protected)

; CONFIG6L
  CONFIG  WRT0 = OFF            ; Write Protection bit Block 0 (Block 0 (000800, 001000 or 002000-003FFFh) not write-protected)
  CONFIG  WRT1 = OFF            ; Write Protection bit Block 1 (Block 1 (004000-007FFFh) not write-protected)
  CONFIG  WRT2 = OFF            ; Write Protection bit Block 2 (Block 2 (008000-00BFFFh) not write-protected)
  CONFIG  WRT3 = OFF            ; Write Protection bit Block 3 (Block 3 (00C000-00FFFFh) not write-protected)
  CONFIG  WRT4 = OFF            ; Write Protection bit Block 4 (Block 4 (010000-013FFFh) not write-protected)
  CONFIG  WRT5 = OFF            ; Write Protection bit Block 5 (Block 5 (014000-017FFFh) not write-protected)
  CONFIG  WRT6 = OFF            ; Write Protection bit Block 6 (Block 6 (01BFFF-018000h) not write-protected)
  CONFIG  WRT7 = OFF            ; Write Protection bit Block 7 (Block 7 (01C000-01FFFFh) not write-protected)

; CONFIG6H
  CONFIG  WRTC = OFF            ; Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
  CONFIG  WRTB = OFF            ; Boot Block Write Protection bit (Boot Block (000000-007FFF, 000FFF or 001FFFh) not write-protected)
  CONFIG  WRTD = OFF            ; Data EEPROM Write Protection bit (Data EEPROM not write-protected)

; CONFIG7L
  CONFIG  EBTR0 = OFF           ; Table Read Protection bit Block 0 (Block 0 (000800, 001000 or 002000-003FFFh) not protected from table reads executed in other blocks)
  CONFIG  EBTR1 = OFF           ; Table Read Protection bit Block 1 (Block 1 (004000-007FFFh) not protected from table reads executed in other blocks)
  CONFIG  EBTR2 = OFF           ; Table Read Protection bit Block 2 (Block 2 (008000-00BFFFh) not protected from table reads executed in other blocks)
  CONFIG  EBTR3 = OFF           ; Table Read Protection bit Block 3 (Block 3 (00C000-00FFFFh) not protected from table reads executed in other blocks)
  CONFIG  EBTR4 = OFF           ; Table Read Protection bit Block 4 (Block 4 (010000-013FFFh) not protected from table reads executed in other blocks)
  CONFIG  EBTR5 = OFF           ; Table Read Protection bit Block 5 (Block 5 (014000-017FFFh) not protected from table reads executed in other blocks)
  CONFIG  EBTR6 = OFF           ; Table Read Protection bit Block 6 (Block 6 (018000-01BFFFh) not protected from table reads executed in other blocks)
  CONFIG  EBTR7 = OFF           ; Table Read Protection bit Block 7 (Block 7 (01C000-01FFFFh) not protected from table reads executed in other blocks)

; CONFIG7H
  CONFIG  EBTRB = OFF           ; Boot Block Table Read Protection bit (Boot Block (000000-007FFF, 000FFF or 001FFFh) not protected from table reads executed in other blocks)

  
INT_VAR UDATA_ACS
state res 1
pos res 1
pos_count1 res 1
pos_count2 res 1
pos_count3 res 1
ra4_count res 1
ra4_enabled res 1
rb5_count res 1
rb5_enabled res 1

TEMP_VAR UDATA_ACS
t1 res 1
t2 res 1
t3 res 1

  
RES_VECT CODE 0x0000            ; processor reset vector
    GOTO START                   ; go to beginning of program

    
MAIN_PROG CODE                      ; let linker place main program

 
START
    CALL INIT
    CALL DELAY2S


MAIN_LOOP
    CALL POS_TASK
    CALL STATE_TASK
    CALL RA4_TASK
    GOTO MAIN_LOOP


STATE_TASK
    CLRF WREG
    CPFSEQ state
    BRA _state2
_state1:
    CALL STATE1_TASK
    BRA _state_end
_state2:
    CALL STATE2_TASK
_state_end:
    RETURN
    

STATE1_TASK
    MOVLW 0x02
    SUBWF pos,0
    BZ _corner_state1
    MOVLW 0x05
    SUBWF pos,0
    BZ _corner_state1
    MOVLW 0x08
    SUBWF pos,0
    BZ _corner_state1
    MOVLW 0x0B
    SUBWF pos,0
    BZ _corner_state1
    ; Next pos or not
    INCFSZ pos_count1
    RETURN
    INCFSZ pos_count2
    RETURN
    INCF pos_count3
    MOVLW POS_CHANGE_STATE1
    CPFSLT pos_count3
    BRA _next_pos_state1
    RETURN
_corner_state1:
    CALL RB5_TASK
    BTFSS WREG,0
    RETURN
    MOVLW 0x0B
    CPFSEQ pos
    BRA _next_pos_state1
    ; pos = 0 again
    CLRF pos
    RETURN
_next_pos_state1:
    INCF pos
    CLRF pos_count3
    RETURN
    
    
    
STATE2_TASK
    MOVLW 0x00
    SUBWF pos,0
    BZ _corner_state2
    MOVLW 0x03
    SUBWF pos,0
    BZ _corner_state2
    MOVLW 0x06
    SUBWF pos,0
    BZ _corner_state2
    MOVLW 0x09
    SUBWF pos,0
    BZ _corner_state2
    ; Next pos or not
    INCF pos_count1
    MOVLW POS_CHANGE_STATE2
    CPFSGT pos_count1
    RETURN
    CLRF pos_count1
    INCF pos_count2
    BZ _next_pos_state2
    RETURN
_corner_state2:
    CALL RB5_TASK
    BTFSS WREG,0
    RETURN
    MOVLW 0x00
    CPFSEQ pos
    BRA _next_pos_state2
    ; pos = 0x0B
    MOVLW 0x0B
    MOVWF pos
    RETURN
_next_pos_state2:
    DECF pos
    RETURN


RA4_TASK
    BTFSS PORTA,4
    BRA _ra4_clr
    ; RB4 is set
    INCF ra4_count
    MOVLW RA4_BOUNCE
    CPFSGT ra4_count
    RETURN
    ; Consider pressed
    BTFSS ra4_enabled,0
    RETURN
    CLRF ra4_enabled
    COMF state
    RETURN
_ra4_clr:
    CLRF WREG
    CPFSEQ ra4_count
    BRA _ra4_count_reset
    ; enable reset
    MOVLW 0xFF
    MOVWF ra4_enabled
    RETURN
_ra4_count_reset:
    DECF ra4_count
    RETURN


RB5_TASK
    BTFSS PORTB,5
    BRA _rb5_clr
    ; RB5 is set
    INCF rb5_count
    MOVLW RB5_BOUNCE
    CPFSGT rb5_count
    RETLW 0x00
    ; Consider pressed
    BTFSS rb5_enabled,0
    RETLW 0x00
    CLRF rb5_enabled
    RETLW 0xFF
_rb5_clr:
    CLRF WREG
    CPFSEQ rb5_count
    BRA _rb5_count_reset
    ; enable reset
    MOVLW 0xFF
    MOVWF rb5_enabled
    RETLW 0x00
_rb5_count_reset:
    DECF rb5_count
    RETLW 0x00

    
POS_TASK
    MOVF pos,0
    RLNCF WREG
    RLNCF WREG
    ADDLW 0x08
    ADDWF PCL,0
    BC _carry
    MOVWF PCL
_carry:
    INCF PCLATH
    MOVWF PCL
    
    GOTO POS0_TASK
    GOTO POS1_TASK
    GOTO POS2_TASK
    GOTO POS3_TASK
    GOTO POS4_TASK
    GOTO POS5_TASK
    GOTO POS6_TASK
    GOTO POS7_TASK
    GOTO POS8_TASK
    GOTO POS9_TASK
    GOTO POSA_TASK
    GOTO POSB_TASK
    
    
POS0_TASK
    MOVLW 0x01
    MOVWF LATA
    MOVWF LATB
    CLRF LATC
    CLRF LATD
    RETURN
    
    
POS1_TASK
    CLRF LATA
    MOVLW 0x01
    MOVWF LATB
    MOVWF LATC
    CLRF LATD
    RETURN

    
POS2_TASK
    CLRF LATA
    CLRF LATB
    MOVLW 0x01
    MOVWF LATC
    MOVWF LATD
    RETURN

    
POS3_TASK
    CLRF LATA
    CLRF LATB
    CLRF LATC
    MOVLW 0x03
    MOVWF LATD
    RETURN

    
POS4_TASK
    CLRF LATA
    CLRF LATB
    CLRF LATC
    MOVLW 0x06
    MOVWF LATD
    RETURN

    
POS5_TASK
    CLRF LATA
    CLRF LATB
    CLRF LATC
    MOVLW 0x0C
    MOVWF LATD
    RETURN

    
POS6_TASK
    CLRF LATA
    CLRF LATB
    MOVLW 0x08
    MOVWF LATC
    MOVWF LATD
    RETURN

    
POS7_TASK
    CLRF LATA
    MOVLW 0x08
    MOVWF LATB
    MOVWF LATC
    CLRF LATD
    RETURN

    
POS8_TASK
    MOVLW 0x08
    MOVWF LATA
    MOVWF LATB
    CLRF LATC
    CLRF LATD
    RETURN

    
POS9_TASK
    MOVLW 0x0C
    MOVWF LATA
    CLRF LATB
    CLRF LATC
    CLRF LATD
    RETURN

    
POSA_TASK
    MOVLW 0x06
    MOVWF LATA
    CLRF LATB
    CLRF LATC
    CLRF LATD
    RETURN


POSB_TASK
    MOVLW 0x03
    MOVWF LATA
    CLRF LATB
    CLRF LATC
    CLRF LATD
    RETURN


INIT 
    ; PORTA
    ; Set as digital
    MOVLW 0x0F
    MOVWF ADCON1
    ; Set direction
    MOVLW 0x10
    MOVWF TRISA
    
    ; PORTB
    ; Set direction
    MOVLW 0x20
    MOVWF TRISB
    
    ; PORTC
    ; Set direction
    CLRF TRISC
    
    ; PORTD
    ; Set direction
    CLRF TRISD
    
    ; Turn on first four LEDs of all ports
    MOVLW 0x0F
    MOVWF LATA
    MOVWF LATB
    MOVWF LATC
    MOVWF LATD
    ; Clear the rest of the latches
    CLRF LATE
    CLRF LATF
    CLRF LATG
    CLRF LATH
    CLRF LATJ
    
    ; Variables
    CLRF state
    CLRF pos
    CLRF pos_count1
    CLRF pos_count2
    CLRF pos_count3
    CLRF ra4_count
    CLRF rb5_count
    MOVLW 0xFF
    MOVWF ra4_enabled
    MOVWF rb5_enabled
    
    RETURN
    
    
DELAY2S
    MOVLW 0x66
    MOVWF t3
_loop3:
        MOVLW 0xFF
        MOVWF t2
_loop2:
            MOVLW 0xFF
            MOVWF t1
_loop1:
                DECFSZ t1
                BRA _loop1
                DECFSZ t2
                BRA _loop2
                DECFSZ t3
                BRA _loop3
    RETURN
    
    
    END
