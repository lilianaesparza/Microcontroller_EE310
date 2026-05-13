#include <xc.h>
#include <stdint.h>
#include "PWM.h"
#include "configwords.h"

#define _XTAL_FREQ 4000000

#define BTN_MID     PORTBbits.RB5
#define BTN_CCW     PORTBbits.RB6
#define BTN_CW      PORTBbits.RB7

#define DUTY_LEFT_LIMIT    22
#define DUTY_MIDPOINT      51
#define DUTY_RIGHT_LIMIT   81

uint8_t dutyValue = DUTY_MIDPOINT;
uint8_t lastDutyValue = DUTY_MIDPOINT;
uint8_t updateDelay = 0;
_Bool pwmLevel;

void updateServoPosition(void)
{
    if(BTN_MID == 0)
    {
        dutyValue = DUTY_MIDPOINT;
    }
    else if((BTN_CCW == 0) && (dutyValue > DUTY_LEFT_LIMIT))
    {
        dutyValue--;
    }
    else if((BTN_CW == 0) && (dutyValue < DUTY_RIGHT_LIMIT))
    {
        dutyValue++;
    }

    if(dutyValue != lastDutyValue)
    {
        PWM2_LoadDutyValue(dutyValue);
        lastDutyValue = dutyValue;
    }
}

void main(void)
{
    OSCSTATbits.HFOR = 1;
    OSCFRQ = 0x02;              // 4 MHz clock

    ANSELB = 0x00;              // Set PORTB as digital I/O

    TRISBbits.TRISB2 = 0;       // Servo PWM output
    TRISBbits.TRISB5 = 1;       // Center button
    TRISBbits.TRISB6 = 1;       // Left button
    TRISBbits.TRISB7 = 1;       // Right button

    WPUBbits.WPUB5 = 1;
    WPUBbits.WPUB6 = 1;
    WPUBbits.WPUB7 = 1;

    TMR2_Initialize();
    T2PR = 155;                 // Approximately 20 ms period
    TMR2_StartTimer();

    PWM_Output_D8_Enable();
    PWM2_Initialize();
    PWM2_LoadDutyValue(dutyValue);

    while(1)
    {
        pwmLevel = PWM2_OutputStatusGet();
        PORTBbits.RB2 = pwmLevel;

        if(PIR4bits.TMR2IF)
        {
            PIR4bits.TMR2IF = 0;

            updateDelay++;

            if(updateDelay == 2)
            {
                updateDelay = 0;
                updateServoPosition();
            }
        }
    }
}
