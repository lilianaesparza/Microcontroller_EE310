//-----------------------------
// Title: Temperature Control System Using PIC18
//-----------------------------
// Purpose: 
// This program compares a measured temperature with a reference temperature.
// If the measured temperature is greater than the reference temperature,
// the cooling system is activated. If the measured temperature is less than
// the reference temperature, the heating system is activated. If both
// temperatures are equal, no system is activated. The program also converts
// both temperature values into their decimal digit equivalents and stores
// them in specific registers.
// Dependencies: NONE
// Compiler: MPLAB X IDE with XC8 / PIC-AS Assembler (PIC18 Simulator)
// Author: Liliana Esparza
// OUTPUTS: 
// PORTD.2 ? Connected to the COOLING system
// PORTD.1 ? Connected to the HEATING system
// INPUTS: 
// refTemp stored in register 0x20
// measuredTemp stored in register 0x21
// Versions:
//   V1.0: 03/09/2026 - First version
//   V1.2: TBD - Future improvements or bug fixes
//-----------------------------
 
#include <xc.inc>

PSECT resetVec,class=CODE,reloc=2
resetVec:
    goto start

PSECT code
start:

;----------------
; Program Inputs
;----------------
#define measuredTempInput  35
#define refTempInput       35

;-------------
; Definitions
;-------------
#define HEAT_LED LATD,1
#define COOL_LED LATD,2

; Registers
REG10   EQU     10h
REG11   EQU     11h

;===========================
; Initialize PORTD
CLRF    TRISD          ; PORTD outputs
CLRF    LATD           ; LEDs OFF

; Load temperature inputs
MOVLW   refTempInput
MOVWF   0x20           ; reference temp

MOVLW   measuredTempInput
MOVWF   0x21           ; measured temp

;===========================
; Compare temperatures
MOVF    0x20, W        ; W = refTemp
SUBWF   0x21, W        ; W = measured - ref

BZ      TEMPEQUAL
BN      measuredTempLESS
BRA     measuredTempGREATER

;===========================
; Control Logic

TEMPEQUAL:
CLRF    0x22
BCF     LATD,1
BCF     LATD,2
BRA     CONTINUE

measuredTempGREATER:
MOVLW   0x02
MOVWF   0x22
BSF     LATD,2
BCF     LATD,1
BRA     CONTINUE

measuredTempLESS:
MOVLW   0x01
MOVWF   0x22
BSF     LATD,1
BCF     LATD,2
BRA     CONTINUE

;===========================
; HEX ? DECIMAL (Reference Temp)

CONTINUE:

CLRF    0x60
CLRF    0x61

MOVF    0x20,W
MOVWF   REG10

TENLOOPREF:
MOVLW   10
SUBWF   REG10,F
BNC     STOREREF
INCF    0x61,F
BRA     TENLOOPREF

STOREREF:
MOVLW   10
ADDWF   REG10,F

MOVF    REG10,W
MOVWF   0x60

;===========================
; HEX ? DECIMAL (Measured Temp)

CLRF    0x70
CLRF    0x71

MOVF    0x21,W
MOVWF   REG11

TENLOOPMEASURED:
MOVLW   10
SUBWF   REG11,F
BNC     STOREMEASURED
INCF    0x71,F
BRA     TENLOOPMEASURED

STOREMEASURED:
MOVLW   10
ADDWF   REG11,F

MOVF    REG11,W
MOVWF   0x70

;===========================
; End Loop
ENDPROGRAM:
BRA ENDPROGRAM

END
