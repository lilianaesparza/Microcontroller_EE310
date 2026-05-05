//-----------------------------
// Title: Sound Level Monitor with IOC Halt Interrupt
//-----------------------------
// Purpose:
// This program implements a sound level monitoring system using a PIC18F47K42
// microcontroller. A sound sensor connected to RA0 is read using the ADC, and
// the measured sound level is displayed on a 16x2 LCD.
//
// The system continuously samples the sound signal and calculates the peak-to-peak
// voltage level. Based on the measured level, the LCD displays a sound category:
// quiet, normal, loud, or obnoxious.
//
// An interrupt-on-change input button is used to temporarily halt ADC reading:
// - When the button is pressed, the system enters a HALT state
// - ADC sampling pauses for 10 seconds
// - A red LED blinks during the HALT state
// - After 10 seconds, normal sound monitoring resumes
//
// Dependencies:
// - xc.h
// - stdio.h
//
// Compiler:
// MPLAB X IDE with XC8 (PIC18F47K42)
//
// Author:
// Liliana Esparza
//
//-----------------------------
// INPUTS:
// RA0 (AN0)  -> Sound sensor analog output
// RC2        -> IOC interrupt button
//
// OUTPUTS:
// PORTB      -> LCD data bus (D0-D7)
// RD0        -> LCD RS control pin
// RD1        -> LCD EN control pin
// RE0        -> Red LED halt indicator
//-----------------------------
// SYSTEM BEHAVIOR:
// 1. Initializes LCD, ADC, and IOC interrupt
// 2. Continuously samples sound sensor on RA0
// 3. Calculates peak-to-peak ADC level
// 4. Converts ADC level to millivolts
// 5. Displays sound category and voltage level on LCD
// 6. Button interrupt on RC2:
//      - Sets halt request flag
//      - Pauses ADC reading for 10 seconds
//      - Displays HALT message on LCD
//      - Blinks red LED on RE0
// 7. After halt delay:
//      - Clears LCD
//      - Resumes ADC sound monitoring
//-----------------------------
// INTERNALS:
// - ADC reads analog sound signal from RA0 / AN0
// - Peak-to-peak calculation uses min/max ADC samples
// - IOC interrupt monitors falling edge on RC2
// - ISR only sets a halt flag; delay is handled in main loop
// - LCD uses 8-bit parallel mode through PORTB
//-----------------------------
// CONSTANTS:
// _XTAL_FREQ      -> System clock frequency
// RED_LED         -> Halt indicator LED output
// ADC thresholds  -> Sound category ranges in millivolts
//-----------------------------
// VERSIONS:
// V1.0: 05/05/2026 - Initial working version
//        - Sound sensor ADC reading
//        - LCD sound level display
//        - IOC interrupt button on RC2
//        - 10-second ADC halt state
//        - Red LED halt indicator on RE0
//-----------------------------


#include <xc.h>
#include <stdio.h>

#pragma config FEXTOSC = OFF
#pragma config RSTOSC = HFINTOSC_1MHZ
#pragma config WDTE = OFF
#pragma config LVP = OFF

#define _XTAL_FREQ 1000000

// LCD connections
#define RS LATD0
#define EN LATD1
#define ldata LATB

#define LCD_Port TRISB
#define LCD_Control TRISD

// LED on RE0
// Button on RC2
#define RED_LED LATE0

volatile unsigned char haltRequest = 0;

void LCD_Init(void);
void LCD_Command(char);
void LCD_Char(char);
void LCD_String(const char *);
void LCD_String_xy(char, char, const char *);
void MSdelay(unsigned int);

void ADC_Init(void);
unsigned int ADC_ReadSoundLevel(void);
void DisplaySound(unsigned int level);

void IOC_Init(void);
void Halt_10s(void);

void main(void)
{
    unsigned int level;

    ANSELA = 0x01;
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELD = 0x00;
    ANSELE = 0x00;

    TRISE0 = 0;
    TRISC2 = 1;

    RED_LED = 0;

    LCD_Init();
    ADC_Init();
    IOC_Init();

    while(1)
    {
        if(haltRequest)
        {
            haltRequest = 0;
            Halt_10s();
        }

        unsigned long sum = 0;
        unsigned char i;

        // Smoothing loop
        for(i = 0; i < 5; i++)
        {
            sum += ADC_ReadSoundLevel();
            __delay_ms(100);
        }

        level = sum / 5;

        DisplaySound(level);

        __delay_ms(300);
    }
}

/* ================= INTERRUPT ON RC2 ================= */

void IOC_Init(void)
{
    ANSELC = 0x00;
    ANSELE = 0x00;

    TRISE0 = 0;
    TRISC2 = 1;

    WPUCbits.WPUC2 = 1;

    IOCCNbits.IOCCN2 = 1;
    IOCCPbits.IOCCP2 = 0;

    IOCCFbits.IOCCF2 = 0;
    PIR0bits.IOCIF = 0;

    PIE0bits.IOCIE = 1;

    INTCON0bits.IPEN = 0;
    INTCON0bits.GIE = 1;
}

void __interrupt(irq(IOC), base(8)) IOC_ISR(void)
{
    if(IOCCFbits.IOCCF2)
    {
        haltRequest = 1;
        IOCCFbits.IOCCF2 = 0;
    }

    PIR0bits.IOCIF = 0;
}

/* ================= HALT ================= */

void Halt_10s(void)
{
    unsigned char i;

    LCD_String_xy(1, 0, "SYSTEM HALTED     ");
    LCD_String_xy(2, 0, "ADC paused 10 sec ");

    for(i = 0; i < 20; i++)
    {
        RED_LED = 1;
        __delay_ms(250);

        RED_LED = 0;
        __delay_ms(250);
    }

    RED_LED = 0;

    LCD_Command(0x01);
    __delay_ms(2);
}

/* ================= ADC ================= */

void ADC_Init(void)
{
    ANSELA = 0x01;
    TRISA0 = 1;

    ADPCH = 0x00;
    ADREF = 0x00;
    ADCLK = 0x3F;

    ADCON0 = 0x84;
}

unsigned int ADC_ReadSoundLevel(void)
{
    unsigned int min = 4095;
    unsigned int max = 0;
    unsigned int value;
    unsigned char i;

    for(i = 0; i < 200; i++)
    {
        __delay_us(200);

        ADCON0bits.GO = 1;
        while(ADCON0bits.GO);

        value = ((unsigned int)ADRESH << 8) | ADRESL;

        if(value < min)
            min = value;

        if(value > max)
            max = value;
    }

    return max - min;
}

/* ================= DISPLAY ================= */

void DisplaySound(unsigned int level)
{
    unsigned long millivolts;
    char line1[21];
    char line2[21];

    millivolts = ((unsigned long)level * 3300) / 4095;

    if(millivolts < 3)
        sprintf(line1, "Sound: quiet      ");
    else if(millivolts < 10)
        sprintf(line1, "Sound: normal     ");
    else if(millivolts < 25)
        sprintf(line1, "Sound: loud       ");
    else
        sprintf(line1, "Sound: obnoxious  ");

    sprintf(line2, "%lu mV Sound Level ", millivolts);

    LCD_String_xy(1, 0, line1);
    LCD_String_xy(2, 0, line2);
}

/* ================= LCD ================= */

void LCD_Init(void)
{
    MSdelay(20);

    LCD_Port = 0x00;
    LCD_Control = 0x00;

    LCD_Command(0x38);
    LCD_Command(0x0C);
    LCD_Command(0x06);
    LCD_Command(0x01);
    MSdelay(2);
}

void LCD_Command(char cmd)
{
    ldata = cmd;
    RS = 0;
    EN = 1;
    __delay_us(50);
    EN = 0;
    __delay_ms(2);
}

void LCD_Char(char data)
{
    ldata = data;
    RS = 1;
    EN = 1;
    __delay_us(50);
    EN = 0;
    __delay_ms(1);
}

void LCD_String(const char *msg)
{
    while(*msg)
        LCD_Char(*msg++);
}

void LCD_String_xy(char row, char pos, const char *msg)
{
    char location;

    if(row == 1)
        location = 0x80 + pos;
    else
        location = 0xC0 + pos;

    LCD_Command(location);
    LCD_String(msg);
}

void MSdelay(unsigned int val)
{
    unsigned int i, j;

    for(i = 0; i < val; i++)
    {
        for(j = 0; j < 165; j++);
    }
}
