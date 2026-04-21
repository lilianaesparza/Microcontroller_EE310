#ifndef INIT_H
#define INIT_H

#include "config.h"
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#define _XTAL_FREQ 4000000UL

// --------------------
// Pin mapping
// --------------------
#define SYS_LED_LAT      LATCbits.LATC3
#define SYS_LED_TRIS     TRISCbits.TRISC3

#define BUZZER_LAT       LATCbits.LATC6
#define BUZZER_TRIS      TRISCbits.TRISC6

#define RELAY_LAT        LATCbits.LATC7
#define RELAY_TRIS       TRISCbits.TRISC7

#define EMG_TRIS         TRISBbits.TRISB0

#define SEG_LAT          LATD
#define SEG_TRIS         TRISD

#define PR1_TRIS         TRISAbits.TRISA0
#define PR2_TRIS         TRISAbits.TRISA1

// --------------------
// Secret code
// Example: 2 on PR1, then 3 on PR2
// --------------------
#define SECRET_DIGIT_1   2
#define SECRET_DIGIT_2   3

// --------------------
// Timing and threshold
// --------------------
#define DARK_THRESHOLD        800
#define DEBOUNCE_MS           250
#define DIGIT_DONE_MS         1500
#define ERROR_BUZZ_MS         2000

// Passive buzzer tones
#define BUZZER_ERR_FREQ_HZ    2000
#define BUZZER_EMG_FREQ1_HZ   1500
#define BUZZER_EMG_FREQ2_HZ   2500

extern volatile uint8_t emergency_flag;

#endif
