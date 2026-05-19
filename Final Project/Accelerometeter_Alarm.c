/*
 * Project: ADXL335 Alarm System with 16x2 LCD
 * Board: PIC18F47K42 Curiosity Nano
 * Compiler: XC8
 */
/* Youtube link: https://www.youtube.com/watch?v=YZSkxaTwrrQ 
*/
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define _XTAL_FREQ 64000000UL

#pragma config FEXTOSC = OFF
#pragma config RSTOSC = HFINTOSC_64MHZ
#pragma config CLKOUTEN = OFF
#pragma config WDTE = OFF
#pragma config LVP = ON
#pragma config MVECEN = OFF

#define BUTTON_PORT        PORTBbits.RB4

#define BUZZER_LAT         LATAbits.LATA4
#define BUZZER_TRIS        TRISAbits.TRISA4

#define LCD_RS_LAT         LATDbits.LATD0
#define LCD_E_LAT          LATDbits.LATD1
#define LCD_D4_LAT         LATDbits.LATD2
#define LCD_D5_LAT         LATDbits.LATD3
#define LCD_D6_LAT         LATDbits.LATD4
#define LCD_D7_LAT         LATDbits.LATD5

#define LCD_RS_TRIS        TRISDbits.TRISD0
#define LCD_E_TRIS         TRISDbits.TRISD1
#define LCD_D4_TRIS        TRISDbits.TRISD2
#define LCD_D5_TRIS        TRISDbits.TRISD3
#define LCD_D6_TRIS        TRISDbits.TRISD4
#define LCD_D7_TRIS        TRISDbits.TRISD5

#define ADC_CHANNEL_X      0x00
#define ADC_CHANNEL_Y      0x01
#define ADC_CHANNEL_Z      0x02

// Start here. Increase if still false-triggering.
#define MOTION_THRESHOLD_X     150
#define MOTION_THRESHOLD_Y     150
#define MOTION_THRESHOLD_Z     200

#define MOTION_HITS_REQUIRED   8
#define CALIBRATION_SAMPLES    64
#define DEBOUNCE_TICKS         5

// Ignore motion for 3 seconds after arming
#define ARM_IGNORE_TICKS       300

#define TIMER0_RELOAD_H        0xFD
#define TIMER0_RELOAD_L        0x8F

volatile bool systemArmed = false;
volatile bool alarmTriggered = false;
volatile bool buttonInterruptFlag = false;
volatile uint16_t systemTicks10ms = 0;

uint16_t baseX = 0;
uint16_t baseY = 0;
uint16_t baseZ = 0;
uint16_t armedTick = 0;

uint8_t lastDisplayState = 255;

void hardware_init(void);

void lcd_init(void);
void lcd_command(uint8_t cmd);
void lcd_data(uint8_t data);
void lcd_send_nibble(uint8_t nibble);
void lcd_pulse_enable(void);
void lcd_clear(void);
void lcd_set_cursor(uint8_t row, uint8_t col);
void lcd_print(const char *text);
void lcd_print_padded(const char *text);
void update_lcd_status(void);

uint16_t adc_read(uint8_t channel);
void read_accelerometer(uint16_t *x, uint16_t *y, uint16_t *z);
void read_accelerometer_avg(uint16_t *x, uint16_t *y, uint16_t *z);
void calibrate_accelerometer(void);
bool motion_detected(void);

void buzzer_on(void);
void buzzer_off(void);
void buzzer_tone_10ms(void);

void button_task(void);
void alarm_task(void);

void main(void)
{
    hardware_init();
    lcd_init();

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print_padded("ADXL335 Alarm");
    lcd_set_cursor(1, 0);
    lcd_print_padded("Starting...");
    __delay_ms(1000);

    calibrate_accelerometer();

    while (1)
    {
        button_task();

        if (systemArmed && !alarmTriggered)
        {
            if ((uint16_t)(systemTicks10ms - armedTick) > ARM_IGNORE_TICKS)
            {
                if (motion_detected())
                {
                    alarmTriggered = true;
                    lastDisplayState = 255;
                }
            }
        }

        alarm_task();
        update_lcd_status();
    }
}

void hardware_init(void)
{
    OSCFRQ = 0x08;

    ANSELB = 0x00;
    ANSELD = 0x00;

    ANSELA = 0x00;
    ANSELAbits.ANSELA0 = 1;
    ANSELAbits.ANSELA1 = 1;
    ANSELAbits.ANSELA2 = 1;

    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    TRISAbits.TRISA2 = 1;

    BUZZER_TRIS = 0;
    BUZZER_LAT = 0;
    ANSELAbits.ANSELA4 = 0;

    TRISBbits.TRISB4 = 1;
    ANSELBbits.ANSELB4 = 0;
    WPUBbits.WPUB4 = 1;

    LCD_RS_TRIS = 0;
    LCD_E_TRIS = 0;
    LCD_D4_TRIS = 0;
    LCD_D5_TRIS = 0;
    LCD_D6_TRIS = 0;
    LCD_D7_TRIS = 0;

    LCD_RS_LAT = 0;
    LCD_E_LAT = 0;
    LCD_D4_LAT = 0;
    LCD_D5_LAT = 0;
    LCD_D6_LAT = 0;
    LCD_D7_LAT = 0;

    ADCON0bits.ADON = 0;
    ADCON0bits.ADFM = 1;

    ADREFbits.ADPREF = 0;
    ADREFbits.ADNREF = 0;

    ADCLK = 0x3F;
    ADACQ = 20;

    ADCON0bits.ADON = 1;

    T0CON0bits.T0EN = 0;
    T0CON0bits.T016BIT = 1;
    T0CON1bits.T0CS = 0b010;
    T0CON1bits.T0CKPS = 0b1000;

    TMR0H = TIMER0_RELOAD_H;
    TMR0L = TIMER0_RELOAD_L;

    PIR3bits.TMR0IF = 0;
    PIE3bits.TMR0IE = 1;

    T0CON0bits.T0EN = 1;

    IOCBNbits.IOCBN4 = 1;
    IOCBPbits.IOCBP4 = 0;
    IOCBFbits.IOCBF4 = 0;

    PIR0bits.IOCIF = 0;
    PIE0bits.IOCIE = 1;

    INTCON0bits.GIE = 1;
}

void __interrupt() ISR(void)
{
    if (PIR3bits.TMR0IF)
    {
        PIR3bits.TMR0IF = 0;
        TMR0H = TIMER0_RELOAD_H;
        TMR0L = TIMER0_RELOAD_L;
        systemTicks10ms++;
    }

    if (PIR0bits.IOCIF)
    {
        if (IOCBFbits.IOCBF4)
        {
            IOCBFbits.IOCBF4 = 0;
            buttonInterruptFlag = true;
        }

        PIR0bits.IOCIF = 0;
    }
}

void lcd_pulse_enable(void)
{
    LCD_E_LAT = 1;
    __delay_us(2);
    LCD_E_LAT = 0;
    __delay_us(50);
}

void lcd_send_nibble(uint8_t nibble)
{
    LCD_D4_LAT = (nibble >> 0) & 0x01;
    LCD_D5_LAT = (nibble >> 1) & 0x01;
    LCD_D6_LAT = (nibble >> 2) & 0x01;
    LCD_D7_LAT = (nibble >> 3) & 0x01;

    lcd_pulse_enable();
}

void lcd_command(uint8_t cmd)
{
    LCD_RS_LAT = 0;
    lcd_send_nibble(cmd >> 4);
    lcd_send_nibble(cmd & 0x0F);
    __delay_ms(2);
}

void lcd_data(uint8_t data)
{
    LCD_RS_LAT = 1;
    lcd_send_nibble(data >> 4);
    lcd_send_nibble(data & 0x0F);
    __delay_us(50);
}

void lcd_init(void)
{
    LCD_RS_LAT = 0;
    LCD_E_LAT = 0;

    __delay_ms(50);

    lcd_send_nibble(0x03);
    __delay_ms(5);
    lcd_send_nibble(0x03);
    __delay_us(150);
    lcd_send_nibble(0x03);
    __delay_us(150);
    lcd_send_nibble(0x02);
    __delay_us(150);

    lcd_command(0x28);
    lcd_command(0x0C);
    lcd_command(0x06);
    lcd_command(0x01);

    __delay_ms(2);
}

void lcd_clear(void)
{
    lcd_command(0x01);
    __delay_ms(2);
}

void lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t address;

    if (row == 0)
        address = 0x00 + col;
    else
        address = 0x40 + col;

    lcd_command(0x80 | address);
}

void lcd_print(const char *text)
{
    while (*text)
    {
        lcd_data(*text);
        text++;
    }
}

void lcd_print_padded(const char *text)
{
    uint8_t count = 0;

    while (*text && count < 16)
    {
        lcd_data(*text);
        text++;
        count++;
    }

    while (count < 16)
    {
        lcd_data(' ');
        count++;
    }
}

uint16_t adc_read(uint8_t channel)
{
    ADPCH = channel;

    __delay_us(10);

    ADCON0bits.GO = 1;

    while (ADCON0bits.GO == 1)
    {
        ;
    }

    return ((uint16_t)ADRESH << 8) | ADRESL;
}

void read_accelerometer(uint16_t *x, uint16_t *y, uint16_t *z)
{
    *x = adc_read(ADC_CHANNEL_X);
    __delay_ms(2);

    *y = adc_read(ADC_CHANNEL_Y);
    __delay_ms(2);

    *z = adc_read(ADC_CHANNEL_Z);
    __delay_ms(2);
}

void read_accelerometer_avg(uint16_t *x, uint16_t *y, uint16_t *z)
{
    uint32_t sumX = 0;
    uint32_t sumY = 0;
    uint32_t sumZ = 0;

    uint16_t tempX;
    uint16_t tempY;
    uint16_t tempZ;

    for (uint8_t i = 0; i < 8; i++)
    {
        read_accelerometer(&tempX, &tempY, &tempZ);

        sumX += tempX;
        sumY += tempY;
        sumZ += tempZ;

        __delay_ms(2);
    }

    *x = (uint16_t)(sumX / 8);
    *y = (uint16_t)(sumY / 8);
    *z = (uint16_t)(sumZ / 8);
}

void calibrate_accelerometer(void)
{
    uint32_t sumX = 0;
    uint32_t sumY = 0;
    uint32_t sumZ = 0;

    uint16_t x;
    uint16_t y;
    uint16_t z;

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print_padded("Calibrating...");
    lcd_set_cursor(1, 0);
    lcd_print_padded("Keep Still");

    for (uint8_t i = 0; i < CALIBRATION_SAMPLES; i++)
    {
        read_accelerometer_avg(&x, &y, &z);

        sumX += x;
        sumY += y;
        sumZ += z;

        __delay_ms(10);
    }

    baseX = (uint16_t)(sumX / CALIBRATION_SAMPLES);
    baseY = (uint16_t)(sumY / CALIBRATION_SAMPLES);
    baseZ = (uint16_t)(sumZ / CALIBRATION_SAMPLES);

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print_padded("Calibration");
    lcd_set_cursor(1, 0);
    lcd_print_padded("Done");
    __delay_ms(800);

    lastDisplayState = 255;
}

bool motion_detected(void)
{
    static uint8_t motionCount = 0;

    uint16_t x;
    uint16_t y;
    uint16_t z;

    int16_t diffX;
    int16_t diffY;
    int16_t diffZ;

    read_accelerometer_avg(&x, &y, &z);

    diffX = abs((int16_t)x - (int16_t)baseX);
    diffY = abs((int16_t)y - (int16_t)baseY);
    diffZ = abs((int16_t)z - (int16_t)baseZ);

    if (diffX > MOTION_THRESHOLD_X ||
        diffY > MOTION_THRESHOLD_Y ||
        diffZ > MOTION_THRESHOLD_Z)
    {
        if (motionCount < 255)
        {
            motionCount++;
        }

        if (motionCount >= MOTION_HITS_REQUIRED)
        {
            motionCount = 0;
            return true;
        }
    }
    else
    {
        motionCount = 0;
    }

    return false;
}

void buzzer_on(void)
{
    BUZZER_LAT = 1;
}

void buzzer_off(void)
{
    BUZZER_LAT = 0;
}

void buzzer_tone_10ms(void)
{
    for (uint8_t i = 0; i < 20; i++)
    {
        BUZZER_LAT = 1;
        __delay_us(250);
        BUZZER_LAT = 0;
        __delay_us(250);
    }
}

void button_task(void)
{
    static uint16_t lastButtonTick = 0;

    if (buttonInterruptFlag)
    {
        buttonInterruptFlag = false;

        if ((uint16_t)(systemTicks10ms - lastButtonTick) >= DEBOUNCE_TICKS)
        {
            lastButtonTick = systemTicks10ms;

            if (BUTTON_PORT == 0)
            {
                systemArmed = !systemArmed;
                alarmTriggered = false;
                buzzer_off();
                lastDisplayState = 255;

                if (systemArmed)
                {
                    lcd_clear();
                    lcd_set_cursor(0, 0);
                    lcd_print_padded("Arming...");
                    lcd_set_cursor(1, 0);
                    lcd_print_padded("Do Not Touch");
                    __delay_ms(1500);

                    calibrate_accelerometer();

                    armedTick = systemTicks10ms;
                    lastDisplayState = 255;
                }
            }
        }
    }
}

void alarm_task(void)
{
    static uint16_t lastAlarmTick = 0;
    static bool buzzerState = false;

    if (!alarmTriggered)
    {
        buzzer_off();
        buzzerState = false;
        return;
    }

    if ((uint16_t)(systemTicks10ms - lastAlarmTick) >= 15)
    {
        lastAlarmTick = systemTicks10ms;
        buzzerState = !buzzerState;
    }

    if (buzzerState)
    {
        buzzer_tone_10ms();
    }
    else
    {
        buzzer_off();
    }
}

void update_lcd_status(void)
{
    uint8_t displayState;

    if (alarmTriggered)
        displayState = 2;
    else if (systemArmed)
        displayState = 1;
    else
        displayState = 0;

    if (displayState == lastDisplayState)
        return;

    lastDisplayState = displayState;

    lcd_clear();

    if (displayState == 0)
    {
        lcd_set_cursor(0, 0);
        lcd_print_padded("Status:");
        lcd_set_cursor(1, 0);
        lcd_print_padded("DISARMED");
    }
    else if (displayState == 1)
    {
        lcd_set_cursor(0, 0);
        lcd_print_padded("Status:");
        lcd_set_cursor(1, 0);
        lcd_print_padded("ARMED");
    }
    else
    {
        lcd_set_cursor(0, 0);
        lcd_print_padded("!!! WARNING !!!");
        lcd_set_cursor(1, 0);
        lcd_print_padded("MOTION DETECTED");
    }
}
