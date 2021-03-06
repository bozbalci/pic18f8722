/*
 * CENG336, Intro. to Embedded Systems Development
 * Spring 2018
 * Take Home Exam III, rev. 1.1, deadline: 2018-04-30 23:55
 *
 * Group 09
 * Yağmur Oymak 2171783
 * Berk Özbalcı 2171791
 *
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
    PS_ATTEMPT,
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

uint8_t digits_entered;
uint8_t pin_digit;
uint8_t pinset_blink_count;

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
    return '0' + val;
}

void init(void)
{
    // Set RA[5:0] as inputs, required for the ADC module
    TRISA = 0b00111111;
    PORTA = 0;

    // Set RB7 and RB6 as inputs, used as buttons in pin setting and test states
    TRISB = 0b11000000;
    PORTB = 0;

    // Set RE1 as input, used as a button in the initial state
    TRISE = 0b00000010;
    PORTE = 0;

    // Set PORTJ as output, required for 7-segment display
    TRISJ = 0;
    PORTJ = 0;

    // Set PORTH as output, except for RH4, which is an input to the ADC module.
    // The rest of PORTH is required to be outputs for the 7-segment display.
    TRISH = 0b00010000;
    PORTH = 0;

    // Interrupt configuration
    INTCON = 0;
    INTCON2 = 0;

    //  Timer0 configuration, period = 5 ms, prescale = 256, 8-bits
    TMR0L = T0_5MS_INITIAL;
    T0CON = 0;
    TMR0ON = 1;
    T08BIT = 1;
    T0CON |= 0b111;
    TMR0IE = 1;

    // Timer1 configuration, period = 50 ms, prescale = 8, 16-bits RW
    T1CON = 0b10110000;
    TMR1 = T1_50MS_INITIAL;
    TMR1IE = 1;

    // PORTB pull-up configuration
    RBPU = 0;

    // ADC module initialization
    ADCON0 = 0b00110001; // Channel = 12, GO/DONE = 0, ADON = 1
    ADCON1 = 0b00000000; // Voltage Reference = AVdd, AVss and PCFG = Analog
    ADCON2 = 0b10000010; // ADFM = Right justified, ACQT = 0 Tad, ADCS = 32 Tosc

    // Configure ADC interrupt
    ADIF = 0;
    ADIE = 1;

    // Enable all interrupts
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

// This function does not return until RE1 has been pressed and released.
void re1_wait(void)
{
    while (PORTEbits.RE1)
        ;
    while (!PORTEbits.RE1)
        ;
}

// Writes a digit to its appropriate position if the potentiometer value
// was changed since the last call of this function.
void handle_pot_update(void)
{
    if (pot_updated())
    {
        promise_can_promise = 1;

        pin_digit = normalize_ad(ad_result);
        should_blink = 0;
        WriteCommandToLCD(0x80 + sizeof(" Set a pin:") - 1 + digits_entered);
        WriteDataToLCD(get_lcd_repr(pin_digit));
        pot_last = ad_result;
    }
}

// Causes the pound symbol to blink every 250 ms while entering a pin and
// no digit has been set for the currently selected digit.
void handle_pound_blink(void)
{
    if (t0_times >= 50)
    {
        t0_times = 0;

        // Note that sizeof(" Set a pin:") == sizeof(" Enter pin:").
        // So this code is usable in PS_TEST as well.
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

// Initializes the pin setting state, by setting some variables and
// enabling the PORTB change interrupt.
void ps_pinsetting_init(void)
{
    digits_entered = 0;

    should_blink = 1;
    pound = 1;

    pot_last = ad_result;

    RBIE = 1;
}

// Finalizes the pin setting state.
void ps_pinsetting_exit(void)
{
    RBIE = 0;

    zeg_clear();
}

// Similar to ps_pinsetting_init, initializes the attempt state.
void ps_attempt_init(void)
{
    RBIE = 1;
    should_blink = 1;
    pound = 1;
    pot_last = ad_result;

    digits_entered = 0;

    promise_can_promise = 0;
    // Invalidating the promise is required in order to prevent the commit
    // of a dangling promise that is given incorrectly if
    // RB6 is pressed while setting the last digit.
    promise_change_digit = 0;
}

// Returns true if pin contains the same digits as input_pin, false
// otherwise.
int8_t pin_correct(void)
{
    return pin[0] == input_pin[0] &&
           pin[1] == input_pin[1] &&
           pin[2] == input_pin[2] &&
           pin[3] == input_pin[3];
}

void lcd_initial(void)
{
    ClearLCDScreen();
    WriteCommandToLCD(0x80);
    WriteStringToLCD(" $>Very  Safe<$ ");
    WriteCommandToLCD(0xC0);
    WriteStringToLCD(" $$$$$$$$$$$$$$ ");
}

void lcd_enter_pin(void)
{
    ClearLCDScreen();
    WriteCommandToLCD(0x80);
    WriteStringToLCD(" Set a pin:#### ");
}

void lcd_attempt(void)
{
    ClearLCDScreen();
    WriteCommandToLCD(0x80);
    WriteStringToLCD(" Enter pin:#### ");
    WriteCommandToLCD(0xC0);
    WriteStringToLCD("  Attempts:");
    WriteCommandToLCD(0xC0 + sizeof("  Attempts:") - 1);
    WriteDataToLCD(get_lcd_repr(attempts));
    WriteStringToLCD("    ");
}

void lcd_tarpit(void)
{
    ClearLCDScreen();
    WriteCommandToLCD(0x80);
    WriteStringToLCD(" Enter pin:XXXX ");
    WriteCommandToLCD(0xC0);
    WriteStringToLCD("Try after 20sec.");
}

void lcd_success(void)
{
    ClearLCDScreen();
    WriteCommandToLCD(0x80);
    WriteStringToLCD("Safe is opening!");
    WriteCommandToLCD(0xC0);
    WriteStringToLCD("$$$$$$$$$$$$$$$$");
}

void lcd_new_pin(void)
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

void lcd_clear(void)
{
    ClearLCDScreen();
}

void lcd_designers(void)
{
    ClearLCDScreen();

    WriteCommandToLCD(0x80);
    WriteStringToLCD("2171783");
    WriteCommandToLCD(0xC0);
    WriteStringToLCD("2171791");
}

void main(void)
{
    init();
    InitLCD();

    state = PS_INITIAL;
    
    for (;;)
    {
        // The main loop is a state machine. The initial state is PS_INITIAL,
        // and the final states are PS_SUCCESS and PS_FAILURE. Upon program
        // restart, we fall back to the initial state.
        switch (state)
        {
            // Displays the welcoming message on the LCD and proceeds to the
            // next state.
            case PS_INITIAL:
                lcd_initial();

                state = PS_RE1WAIT;
                break;

            // Waits for the press and release actions of RE1, and then
            // proceeds to the next state.
            case PS_RE1WAIT:
                re1_wait();

                state = PS_DELAY;
                break;

            // Produces a three second delay and then proceeds to the next state.
            case PS_DELAY:
                delay_3s();

                state = PS_PINSETTING;
                break;

            // Displays a message on the LCD indicating we are accepting a PIN.
            // Continuously updates as new values from the potentiometer is
            // delivered to ad_result, and waits for RB6 and RB7 depending on
            // how many digits have been inputted.
            case PS_PINSETTING:
                lcd_enter_pin();

                ps_pinsetting_init();

                while (digits_entered != 4)
                {
                    zeg_dashes();
                    handle_pot_update();
                    handle_pound_blink();

                    if (digits_entered < 3)
                    {
                        if (promise_change_digit)
                        {
                            promise_change_digit = 0;
                            promise_can_promise = 0;

                            should_blink = 1;
                            pound = 1;

                            pin[digits_entered++] = pin_digit;

                            pot_last = ad_result;
                            // Invalidating the promise is required in order to prevent the use
                            // of a dangling promise that is given incorrectly if
                            // RB7 is pressed while setting the first three digits.
                            promise_pin_confirmed = 0;
                        }
                    }
                    else if (promise_pin_confirmed) /* && digits_entered == 3 */
                    {
                        promise_pin_confirmed = 0;
                        pin[digits_entered++] = pin_digit;
                    }
                }

                ps_pinsetting_exit();

                state = PS_PINSET;
                break;

            case PS_PINSET:
                t0_times = 0;

                while (pinset_blink_count < 6)
                {
                    if (t0_times >= 100)
                    {
                        t0_times = 0;
                        pinset_blink_count++;

                        if (pinset_blink_count % 2)
                        {
                            lcd_new_pin();
                        }
                        else
                        {
                            lcd_clear();
                        }
                    }
                }

                state = PS_TEST;
                break;

            case PS_TEST:
                T1CON |= 1; // Start timer1, used for counting down the countdown variable.
                attempts = 2;

                state = PS_ATTEMPT;

                break;

            case PS_ATTEMPT:
                lcd_attempt();

                ps_attempt_init();

                while (countdown > 0 && digits_entered != 4)
                {
                    zeg_number(countdown);
                    handle_pot_update();
                    handle_pound_blink();

                    if (digits_entered < 3)
                    {
                        if (promise_change_digit)
                        {
                            promise_change_digit = 0;
                            promise_can_promise = 0;

                            should_blink = 1;
                            pound = 1;

                            input_pin[digits_entered++] = pin_digit;

                            pot_last = ad_result;
                            // Invalidating the promise is required in order to prevent the use
                            // of a dangling promise that is given incorrectly if
                            // RB7 is pressed while setting the first three digits.
                            promise_pin_confirmed = 0;
                        }
                    }
                    else if (promise_pin_confirmed) /* digits_entered == 3 */
                    {
                        promise_pin_confirmed = 0;
                        input_pin[digits_entered++] = pin_digit;
                    }
                }

                if (digits_entered == 4)
                {
                    // If the pin is entered correctly, go to the success state.
                    if (pin_correct())
                    {
                        RBIE = 0;
                        state = PS_SUCCESS;
                    }
                    // Otherwise, decrement attempts and go to the test state or
                    // the tarpit state as necessary.
                    else if (--attempts == 0)
                    {
                        state = PS_TARPIT;
                        RBIE = 0;
                    }
                }
                else /* if (countdown <= 0) */
                {
                    state = PS_FAILURE;
                }

                break;

            // The tarpit state discards all inputs from the user and counts down
            // from the tarpit period, which is specified as 20 seconds. During
            // this period, the global countdown may reach zero and the program
            // may enter the failure state (PS_FAILURE).
            case PS_TARPIT:
                countdown_snapshot = countdown;

                lcd_tarpit();

                while (state == PS_TARPIT)
                {
                    zeg_number(countdown);

                    if (countdown <= 0)
                    {
                        state = PS_FAILURE;
                    }
                    else if (countdown == countdown_snapshot - 20)
                    {
                        state = PS_TEST;
                    }
                }

                break;

            // The success state displays the success message and then enters
            // an infinite loop, while displaying the remaining seconds on the
            // 7-segment display.
            case PS_SUCCESS:
                lcd_success();

                /* Disable Timer1 */
                T1CON &= ~1;

                while (1)
                {
                    zeg_number(countdown);
                }

                break;

            // The failure state immediately resets the entire program.
            case PS_FAILURE:
                RESET();

                break;

            // Here's a little easter egg for you. Open up your debugger and
            // set the state to PS_FAILURE + 1.
            default:
                lcd_designers();
                delay_3s();
                RESET();

                break;

            // This is the end of the main loop. We have switched over a
            // exhaustive set of mutually disjoint states. There are no
            // other states, and well... we think there's nothing more to
            // say. What happens from now on? Well... we're really not
            // at liberty to.. uhm... say.
        }
    }
}
