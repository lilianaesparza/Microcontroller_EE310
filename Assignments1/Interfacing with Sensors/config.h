#ifndef CONFIG_H 
#define CONFIG_H 

#include <xc.h>

// CONFIG1L
#pragma config FEXTOSC = OFF
#pragma config RSTOSC = HFINTOSC_1MHZ

// CONFIG1H
#pragma config CLKOUTEN = OFF
#pragma config PR1WAY = ON
#pragma config CSWEN = ON
#pragma config FCMEN = OFF

// CONFIG2L
#pragma config MCLRE = EXTMCLR
#pragma config PWRTS = PWRT_OFF
#pragma config MVECEN = OFF
#pragma config IVT1WAY = ON
#pragma config LPBOREN = OFF
#pragma config BOREN = SBORDIS

// CONFIG2H
#pragma config BORV = VBOR_2P45
#pragma config ZCD = OFF
#pragma config PPS1WAY = ON
#pragma config STVREN = ON
#pragma config DEBUG = OFF
#pragma config XINST = OFF

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31
#pragma config WDTE = OFF

// CONFIG3H
#pragma config WDTCWS = WDTCWS_7
#pragma config WDTCCS = SC

// CONFIG4L
#endif
