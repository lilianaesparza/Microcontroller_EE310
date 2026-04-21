//-----------------------------
// Title: Dual PR Sensor Code Entry System with Emergency Interrupt
//-----------------------------
// Purpose:
// This program implements a secure code entry system using a PIC18 microcontroller.
// Two photo resistors (PR1 and PR2) are used to input a 2-digit code by counting
// interruptions (light-to-dark transitions). Each digit is displayed on a 7-segment display.
//
// After both digits are entered:
// - If the code matches the predefined secret code, a motor (relay) is activated.
// - If incorrect, an error tone is generated using a passive buzzer.
//
// An emergency input (INT0) can be triggered at any time:
// - Immediately stops the motor
// - Plays an emergency melody
// - Locks the system in a safe state
//
// Dependencies:
// - init.h (pin definitions, system constants)
// - function.h (system functions and drivers)
// - config.h (configuration bits)
//
// Compiler:
// MPLAB X IDE with XC8 (PIC18F47K42)
//
// Author:
// Liliana Esparza
//
//-----------------------------
// INPUTS:
// RA0 (AN0)  -> PR1 (Digit 1 input)
// RA1 (AN1)  -> PR2 (Digit 2 input)
// RB0        -> Emergency Interrupt (INT0)
// OUTPUTS:
// PORTD      -> 7-segment display (common cathode)
// RC3        -> System LED (status indicator)
// RC6        -> Passive buzzer
// RC7        -> Relay (motor control)
//-----------------------------
// SYSTEM BEHAVIOR:
// 1. Waits for user input via PR1 (digit 1)
// 2. Displays detected digit on 7-segment
// 3. Waits for user input via PR2 (digit 2)
// 4. Displays detected digit
// 5. If code is correct:
//      - Turns ON motor (relay)
//      - Runs until emergency trigger
// 6. If code is incorrect:
//      - Plays error tone
// 7. Emergency interrupt (INT0):
//      - Immediately stops motor
//      - Plays emergency melody
//      - System halts permanently
//-----------------------------
// INTERNALS:
// - ADC used to read analog values from PR sensors
// - Threshold comparison determines light/dark detection
// - Edge detection counts interruptions for digit entry
// - Timer-based delays used for debouncing and digit completion
//-----------------------------
// CONSTANTS:
// SECRET_DIGIT_1, SECRET_DIGIT_2 -> Required access code
// DARK_THRESHOLD                 -> Light detection threshold
// DEBOUNCE_MS                    -> Sensor debounce timing
// DIGIT_DONE_MS                  -> Digit completion timeout
//-----------------------------
// VERSIONS:
// V1.0: 04/21/2026 - Initial working version
//        - Dual PR input
//        - 7-segment display
//        - Motor control
//        - Emergency interrupt system
//-----------------------------

#include "config.h"
#include "init.h"
#include "functions.h"

volatile uint8_t emergency_flag = 0;

void main(void)
{
    uint8_t digit1, digit2;

    init_system();
    SYS_LED_LAT = 1;

    while (1)
    {
        if (emergency_flag)
        {
            motor_off();
            display_blank();

            while (1)
            {
                SYS_LED_LAT = 1;
            }
        }

        // First digit from PR1
        display_blank();
        digit1 = read_digit_from_pr(pr1_dark);
        if (emergency_flag) continue;

        display_digit(digit1);
        __delay_ms(1000);

        // Second digit from PR2
        display_blank();
        digit2 = read_digit_from_pr(pr2_dark);
        if (emergency_flag) continue;

        display_digit(digit2);
        __delay_ms(1000);

        // Check secret code
        if (code_correct(digit1, digit2))
        {
            motor_on();

            while (!emergency_flag)
            {
                SYS_LED_LAT = 1;
            }

            motor_off();
        }
        else
        {
            buzz_error();
        }

        display_blank();
        __delay_ms(500);
    }
}
