#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "init.h"

void init_system(void);
uint16_t adc_read(uint8_t ch);
bool pr1_dark(void);
bool pr2_dark(void);

void display_blank(void);
void display_digit(uint8_t digit);

void motor_on(void);
void motor_off(void);
void buzzer_on(void);
void buzzer_off(void);
void buzz_error(void);
void emergency_melody(void);

uint8_t read_digit_from_pr(bool (*sensor_dark)(void));
bool code_correct(uint8_t d1, uint8_t d2);

#endif
