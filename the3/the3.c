/*
 * CENG336, Intro. to Embedded Systems Development
 * Spring 2018
 * Take Home Exam III, rev. 1.1, deadline: 2018-04-30 23:55
 *
 * Group 09
 * Yağmur Oymak 2171783
 * Berk Özbalcı 2171791
 *
 * TODO: Clear out TODOs!
 */

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

#define T0_5MS_INITIAL 61 // Required to produce a delay of 5ms
#define T1_50MS_INITIAL 0xBE3 // Required to produce a delay of 50ms

// This enumerated type holds the current state of the program. Each of these
// states are handled separately. Each state has its own rules for transitioning
// into different states.
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

volatile uint8_t t0_times; // Number of Timer0 interrupts handled
volatile uint8_t t1_times; // Number of Timer1 interrupts handled
volatile uint8_t adc_counter; // Seperate counter for ADC conversion in Timer0

// The Promise-Commit pattern is used in this code. Named by the authors, the
// "promise" is a flag that is set by an external subroutine and then "committed"
// in the main loop of the program. Once committed, the promise is said to be
// invalidated (set to zero). It is possible to extend this mechanism to multiple
// by using non-binary values, but in this program we chose not to.
volatile uint8_t promise_change_digit; // Promise given by RB6 event
volatile uint8_t promise_pin_confirmed; // Promise given by RB7 event

// promise_can_promise is a promise given by the main loop to the PORTB button
// handlers. When committed, promise_change_digit and promise_pin_confirmed can
// be dealt by their respective handlers.
volatile uint8_t promise_can_promise;

// These variables are set when either of the respective buttons are pressed.
// They are checked against the current value of the buttons to verify that
// they have been released.
volatile uint8_t rb6_pressed, rb7_pressed;

// ad_result is constantly updated on ADC conversion. It is the non-normalized
// value of the potentiometer, ranging between 0 and 1023.
volatile int16_t ad_result;

// pot_last is updated just before new user input by the potentiometer is
// required. It captures the value of ad_result at that instant, and is
// checked against the subsequent values of ad_result to determine if the
// user had provided new inputs.
int16_t pot_last;

// should_blink is set when no input has been given for the current digit of
// the pin, that is exactly when the pound symbol shall blink on the LCD.
// pound is a constantly alternating variable. When cleared, the LCD displays
// a whitespace character, and when set, the LCD displays a pound symbol.
uint8_t should_blink, pound;

// pin holds the PIN that was set by the user in PS_PINSETTING. input_pin holds
// the PINs that are entered by the user in the test stage, namely PS_TEST and
// PS_TARPIT.
uint8_t pin[4], input_pin[4];

int8_t countdown = 120; // Seconds left before program restart
int8_t attempts; // Number of attempts left
int8_t countdown_snapshot; // An instantaneous value of countdown

// This function compares the current potentiometer value to a previous value.
// If the difference is larger than the threshold value of 50, it returns 1.
// Otherwise, it returns 0.
uint8_t pot_updated(void)
{
    int16_t diff = pot_last - ad_result;

    return (diff >= 50) || (diff <= -50);
}

// [S]leep [F]or [S]even [S]egment [D]isplay [U]pdate. This horribly named
// function produces a delay that is sufficiently long to display outputs
// on the seven segment display with a tolerable level of flickering.
void sfssdu(void)
{ 
    // TODO Maybe this is better implemented as a preprocessor macro.
    uint16_t i = 1023;

    while (i)
    {
        i--;
    }
}

// zeg_clear clears the seven segment display. If you squint hard enough,
// the letter z looks like the digit 7, which is supposed to remind you that
// this function operates on the seven segment display. The following functions
// are named similarly.
void zeg_clear(void)
{
    PORTJ = 0;
    PORTH = 8;
    
    do
    {
        ;
    } while (PORTH >>= 1);
}

// zeg_dashes displays four dashes on the seven segment display.
void zeg_dashes(void)
{
    PORTJ = 1 << 6;
    PORTH = 8;

    do
    {
        sfssdu();
    } while (PORTH >>= 1);
}

// get_zeg_repr takes a digit as input and returns the corresponding segment
// display configuration. This value is then written to PORTJ.
int8_t get_zeg_repr(int8_t digit)
{
    switch (digit)
    {
        case 0:
            return 0b00111111;
        case 1:
            return 0b00000110;
        case 2:
            return 0b01011011;
        case 3:
            return 0b01001111;
        case 4:
            return 0b01100110;
        case 5:
            return 0b01101101;
        case 6:
            return 0b01111101;
        case 7:
            return 0b00000111;
        case 8:
            return 0b01111111;
        case 9:
            return 0b01100111;
    }
}

// zeg_number displays a positive number up to 127 on the seven segment display.
// The number is zero-padded to the length of the display (4).
void zeg_number(int8_t num)
{
    // The most significant digit is always zero in our specifications.
    PORTH = 1;
    PORTJ = get_zeg_repr(0);
    sfssdu();

    // The third digit is either 1 or 0, depending on how large the number is.
    PORTH = 2;
    if (num >= 100)
    {
        PORTJ = get_zeg_repr(1);
    }
    else
    {
        PORTJ = get_zeg_repr(0);
    }
    sfssdu();

    num %= 100;
    PORTH = 4;
    PORTJ = get_zeg_repr(num / 10);
    sfssdu();

    num %= 10;
    PORTH = 8;
    PORTJ = get_zeg_repr(num);
    sfssdu();
}

// delay_3s produces a three second delay. This function was implemented in
// PIC assembly on a previous assignment and was shamelessly copied verbatim
// into this one.
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

// normalize_ad returns a normalized value of the ADC output. According to the
// specifications, 0-99 maps to 0, 100-199 maps to 1, and so on, until 999-1023
// which maps to 9.
int8_t normalize_ad(int16_t ad)
{
    int8_t result;

    if (ad < 1000)
    {
        result = ad / 100;
        return result;
    }

    return 9;
}

// get_lcd_repr takes a digit and returns a character suitable for printing onto
// the LCD screen.
char get_lcd_repr(int8_t val)
{
    // TODO Maybe this is better implemented as a preprocessor macro.
    
    return '0' + val;
}

void init(void)
{
    // TODO Comment and organize this mess
    
    TRISA = 0b00111111;
    PORTA = 0;
    // Configure RB7 and RB6 as inputs, the rest of the PORTB is unused.
    TRISB = 0b11000000;
    PORTB = 0;

    PORTE = 0;

    TRISE = 1 << 1;

    TRISJ = 0;
    TRISH = 0b00010000;
    PORTJ = 0;
    PORTH = 0;

    INTCON = 0;
    INTCON2 = 0;

    /* Timer0 config */
    TMR0L = T0_5MS_INITIAL;
    T0CON = 0;
    TMR0ON = 1;
    T08BIT = 1;
    T0CON |= 0b111;

    /* Timer1 config */
    T1CON = 0b10110000;
    TMR1IE = 1;
    TMR1 = 0xBE3;

    TMR0IE = 1;
    RBPU = 0;

    /* Channel = 12, GO/DONE = 0, ADON = 1 */
    ADCON0 = 0b00110001;

    /* Voltage Reference = AVdd, AVss and PCFG = Analog */
    ADCON1 = 0b00000000;

    /* ADFM = Right justified, ACQT = 2 Tad, ADCS = 64 Tosc */
    ADCON2 = 0b10000010;

    ADIF = 0;
    ADIE = 1;

    PEIE = 1;
    ei();
}

// isr is the interrupt service routine.
void interrupt isr(void)
{
    if (TMR0IF)
    {
        // Timer0 interrupt.
        
        // Increment t0_times. As the desired number of Timer0 interrupts are different
        // in different states of the program, it is within responsibility of the
        // main loop to check this counter against a value and reset it afterwards.
        t0_times++;
        
        // Increment adc_counter. On every 20th increment, a period of 100 ms passes
        // and the counter is reset. Then, the GO bit of ADCON0 is set to trigger
        // ADC conversion.
        if (++adc_counter == 20)
        {
            ADCON0bits.GO = 1;
            adc_counter = 0;
        }
        
        TMR0L = T0_5MS_INITIAL;
        TMR0IF = 0;
    }
    else if (TMR1IF)
    {
        // Timer1 interrupt.
        
        // Increment t1_times. On every 20th increment, a period of 1 s passes and
        // the counter is reset, and countdown is decremented.
        if (++t1_times == 20) 
        {
            countdown--;
            t1_times = 0;
        }
        
        TMR1 = T1_50MS_INITIAL;
        TMR1IF = 0;
    }
    else if (ADIF)
    {
        // ADC interrupt.
        
        // Capture the output of the ADC module into ad_result and dispatch.
        ad_result = ADRES;
        
        ADIF = 0;
    }
    else if (RBIF)
    {
        // This unorthodox method of reading PORTB is probably required to
        // force a read on all of the bits in PORTB, therefore ending the
        // mismatch condition and allow the RBIF bit to be cleared.
        if (PORTB)
        {
#asm
            nop
#endasm
        }
        
        // If promise_can_promise is not given, then the release of RB6 or
        // RB7 should not yield a promise_change_digit or promise_pin_confirmed.
        // In this case we clear rb6_pressed and rb7_pressed, effectively discarding
        // button presses prior to the issuance of promise_can_promise.
        if (rb6_pressed && !promise_can_promise)
        {
            rb6_pressed = 0;
        }

        if (rb7_pressed && !promise_can_promise)
        {
            rb7_pressed = 0;
        }
        
        // When promise_can_promise is given, the following if-block, and its subsequent,
        // will capture a press and release and issue their respective promises.
        if (!PORTBbits.RB6)
        {
            rb6_pressed = 1;
        }
        else if (rb6_pressed && promise_can_promise)
        {
            promise_change_digit = 1;
            rb6_pressed = 0;
        }

        if (!PORTBbits.RB7)
        {
            rb7_pressed = 1;
        }
        else if (rb7_pressed && promise_can_promise)
        {
            promise_pin_confirmed = 1;
            rb7_pressed = 0;
        }
        
        RBIF = 0;
    }
}

void main(void)
{
    // TODO Comment and organize this
    
    uint8_t digits_entered;
    uint8_t pin_digit;
    uint8_t count = 0;

    init();
    InitLCD();

    state = PS_INITIAL;
    
    for (;;)
    {
main_loop_init:

        switch (state)
        {
            case PS_INITIAL:
                ClearLCDScreen();
                WriteCommandToLCD(0x80); // Goto to the beginning of the first line
                WriteStringToLCD(" $>Very  Safe<$ ");
                WriteCommandToLCD(0xC0); // Goto to the beginning of the second line
                WriteStringToLCD(" $$$$$$$$$$$$$$ ");

                state = PS_RE1WAIT;
                break;

            case PS_RE1WAIT:
                while (!PORTEbits.RE1)
                    ;
                /* FIXME: Button release waiting */
                while (PORTEbits.RE1)
                    ;

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
                RBIE = 1;

                should_blink = 1;
                pound = 1;

                pot_last = ad_result;

                while (digits_entered < 3)
                {
                    if (promise_change_digit && !should_blink)
                    {
                        promise_change_digit = 0;
                        promise_can_promise = 0;

                        should_blink = 1;
                        pound = 1;

                        pin[digits_entered++] = pin_digit;

                        continue;
                    }

                    if (pot_updated())
                    {
                        promise_can_promise = 1;

                        pin_digit = normalize_ad(ad_result);
                        should_blink = 0;
                        WriteCommandToLCD(0x80 + sizeof(" Set a pin:") - 1 + digits_entered);
                        WriteDataToLCD(get_lcd_repr(pin_digit));
                        pot_last = ad_result;
                    }

                    zeg_dashes();

                    if (t0_times == 50)
                    {
                        t0_times = 0;

                        WriteCommandToLCD(0x80 + sizeof(" Set a pin:") - 1 + digits_entered);

                        if (should_blink)
                        {
                            pound = !pound;

                            if (pound)
                            {
                                WriteStringToLCD("#");
                            }
                            else
                            {
                                WriteStringToLCD(" ");
                            }
                        }
                    }
                }

                pot_last = ad_result;
                promise_pin_confirmed = 0;

                while (digits_entered != 4)
                {
                    if (promise_pin_confirmed)
                    {
                        promise_pin_confirmed = 0;
                        pin[digits_entered++] = pin_digit;
                    }

                    if (pot_updated())
                    {
                        promise_can_promise = 1;

                        pin_digit = normalize_ad(ad_result);
                        should_blink = 0;
                        WriteCommandToLCD(0x80 + sizeof(" Set a pin:") - 1 + digits_entered);
                        WriteDataToLCD(get_lcd_repr(pin_digit));
                        pot_last = ad_result;
                    }

                    zeg_dashes();

                    if (t0_times == 50)
                    {
                        t0_times = 0;

                        WriteCommandToLCD(0x80 + sizeof(" Set a pin:") - 1 + digits_entered);

                        if (should_blink)
                        {
                            pound = !pound;

                            if (pound)
                            {
                                WriteStringToLCD("#");
                            }
                            else
                            {
                                WriteStringToLCD(" ");
                            }
                        }
                    }
                }

                RBIE = 0;

                zeg_clear();

                state = PS_PINSET;
                break;

            case PS_PINSET:
                t0_times = 0;
                
                while (count < 6)
                {
                    if (t0_times == 100)
                    {
                        t0_times = 0;
                        count++;

                        if (count % 2)
                        {
                            ClearLCDScreen();
                            WriteCommandToLCD(0x80);
                            WriteStringToLCD(" The new pin is ");
                            WriteCommandToLCD(0xC0);
                            WriteStringToLCD("   ---");
                            WriteDataToLCD(get_lcd_repr(pin[0]));
                            WriteDataToLCD(get_lcd_repr(pin[1]));
                            WriteDataToLCD(get_lcd_repr(pin[2]));
                            WriteDataToLCD(get_lcd_repr(pin[3]));
                            WriteStringToLCD("---   ");
                        }
                        else
                        {
                            ClearLCDScreen();
                        }
                    }
                }

                state = PS_TEST;
                break;

            case PS_TEST:
                T1CON |= 1;
                attempts = 2;

pintest_attempt:
                ClearLCDScreen();
                WriteCommandToLCD(0x80);
                WriteStringToLCD(" Enter pin:#### ");
                WriteCommandToLCD(0xC0);
                WriteStringToLCD("  Attempts:");
                WriteCommandToLCD(0xC0 + sizeof("  Attempts:") - 1);
                WriteDataToLCD(get_lcd_repr(attempts));
                WriteStringToLCD("    ");

                RBIE = 1;
                should_blink = 1;
                pound = 1;
                pot_last = ad_result;

                digits_entered = 0;

                promise_change_digit = 0;
                while (digits_entered < 3)
                {
                    if (countdown <= 0)
                    {
                        state = PS_FAILURE;

                        goto main_loop_init;
                    }

                    zeg_number(countdown);
                    if (promise_change_digit && !should_blink)
                    {
                        promise_change_digit = 0;
                        promise_can_promise = 0;

                        should_blink = 1;
                        pound = 1;

                        input_pin[digits_entered++] = pin_digit;

                        continue;
                    }

                    if (pot_updated())
                    {
                        promise_can_promise = 1;

                        pin_digit = normalize_ad(ad_result);
                        should_blink = 0;
                        WriteCommandToLCD(0x80 + sizeof(" Enter pin:") - 1 + digits_entered);
                        WriteDataToLCD(get_lcd_repr(pin_digit));
                        pot_last = ad_result;
                    }

                    if (t0_times == 50)
                    {
                        t0_times = 0;

                        WriteCommandToLCD(0x80 + sizeof(" Enter pin:") - 1 + digits_entered);

                        if (should_blink)
                        {
                            pound = !pound;

                            if (pound)
                            {
                                WriteStringToLCD("#");
                            }
                            else
                            {
                                WriteStringToLCD(" ");
                            }
                        }
                    }
                }

                pot_last = ad_result;
                promise_pin_confirmed = 0;

                while (digits_entered != 4)
                {
                    zeg_number(countdown);

                    if (countdown <= 0)
                    {
                        state = PS_FAILURE;

                        goto main_loop_init;
                    }

                    if (promise_pin_confirmed)
                    {
                        promise_pin_confirmed = 0;
                        input_pin[digits_entered++] = pin_digit;
                    }

                    if (pot_updated())
                    {
                        promise_can_promise = 1;

                        pin_digit = normalize_ad(ad_result);
                        should_blink = 0;
                        WriteCommandToLCD(0x80 + sizeof(" Enter pin:") - 1 + digits_entered);
                        WriteDataToLCD(get_lcd_repr(pin_digit));
                        pot_last = ad_result;
                    }

                    if (t0_times == 50)
                    {
                        t0_times = 0;

                        WriteCommandToLCD(0x80 + sizeof(" Enter pin:") - 1 + digits_entered);

                        if (should_blink)
                        {
                            pound = !pound;

                            if (pound)
                            {
                                WriteStringToLCD("#");
                            }
                            else
                            {
                                WriteStringToLCD(" ");
                            }
                        }
                    }
                }

                if (pin[0] == input_pin[0] &&
                    pin[1] == input_pin[1] &&
                    pin[2] == input_pin[2] &&
                    pin[3] == input_pin[3])
                {
                    RBIE = 0;
                    state = PS_SUCCESS;

                    continue;
                }
                else
                {
                    attempts--;

                    if (!attempts)
                    {
                        state = PS_TARPIT;
                        RBIE = 0;

                        continue;
                    }

                    WriteCommandToLCD(0x80);
                    WriteStringToLCD(" Enter pin:#### ");
                    WriteCommandToLCD(0xC0 + sizeof("  Attempts:") - 1);
                    WriteDataToLCD(get_lcd_repr(attempts));
                    WriteStringToLCD("    ");

                    goto pintest_attempt;
                }


            case PS_TARPIT:
                countdown_snapshot = countdown;

                ClearLCDScreen();
                WriteCommandToLCD(0x80);
                WriteStringToLCD(" Enter pin:XXXX ");
                WriteCommandToLCD(0xC0);
                WriteStringToLCD("Try after 20sec.");

                while (1)
                {
                    if (countdown <= 0)
                    {
                        state = PS_FAILURE;

                        goto main_loop_init;
                    }

                    if (countdown == countdown_snapshot - 20)
                    {
                        state = PS_TEST;

                        goto main_loop_init;
                    }

                    zeg_number(countdown);
                }

                break;

            case PS_SUCCESS:
                ClearLCDScreen();
                WriteCommandToLCD(0x80);
                WriteStringToLCD("Safe is opening!");
                WriteCommandToLCD(0xC0);
                WriteStringToLCD("$$$$$$$$$$$$$$$$");

                /* Disable Timer1 */
                T1CON &= ~1;

                while (1)
                {
                    zeg_number(countdown);
                }

                break;

            case PS_FAILURE:
                RESET();
                break;
        }
    }
}
