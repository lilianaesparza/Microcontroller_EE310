#ifndef CONFIGWORDS_H
#define CONFIGWORDS_H

#include <xc.h>

#pragma config FEXTOSC = OFF
#pragma config RSTOSC = HFINTOSC_1MHZ
#pragma config CLKOUTEN = OFF
#pragma config CSWEN = ON
#pragma config FCMEN = ON

#pragma config MCLRE = EXTMCLR
#pragma config PWRTS = PWRT_OFF
#pragma config BOREN = SBORDIS
#pragma config WDTE = OFF
#pragma config LVP = ON
#pragma config XINST = OFF
#pragma config DEBUG = OFF

#define _XTAL_FREQ 4000000
#define FCY (_XTAL_FREQ / 4)

#endif
