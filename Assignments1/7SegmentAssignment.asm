//-----------------------------
// Title: 7-Segment Up/Down Counter Using PIC18
//-----------------------------
// Purpose: 
// This program controls a single 7-segment display using a PIC18 microcontroller.
// Two push buttons are used to increment or decrement the displayed value.
// The display cycles through hexadecimal values (0–F) using a lookup table.
// If both buttons are pressed at the same time, the display resets to 0.
// When incrementing past F, the counter wraps back to 0.
// When decrementing below 0, the counter wraps to F.
//
// Dependencies: NONE
// Compiler: MPLAB X IDE with XC8 / PIC-AS Assembler (PIC18 Simulator)
// Author: Liliana Esparza
//
// OUTPUTS: 
// PORTD  Connected to 7-segment display segments (a–g)
// INPUTS: 
// PORTB.0  Increment (BTN_UP)
// PORTB.1  Decrement (BTN_DOWN)
//
// INTERNALS:
// Lookup table stored from 0x0A to 0x19 for hex values (0–F)
// FSR0 used as pointer to traverse the table
//
// Versions:
//   V1.0: 04/02/2026 - Initial working version with up/down and reset functionality
//-----------------------------
#include "CONFIG.inc"
#include <xc.inc>

; Delay constants
innerLoop EQU 125
outerLoop EQU 100
thirdLoop EQU 12

; I/O definitions
#define BTN_UP      PORTB, 0
#define BTN_DOWN    PORTB, 1
#define SEG_OUT     LATD

; RAM locations
TABLE_START EQU 0x0A
TABLE_END   EQU 0x19
DLY1        EQU 0x31
DLY2        EQU 0x32
DLY3        EQU 0x33

    PSECT absdata,abs,ovrld
    ORG 0x30

;----------------------------------
; Initialization
;----------------------------------
Init:
    ; PortB: inputs (buttons)
    BANKSEL ANSELB
    CLRF ANSELB
    BANKSEL TRISB
    MOVLW 0x03
    MOVWF TRISB

    ; PortD: outputs (7-seg)
    BANKSEL ANSELD
    CLRF ANSELD
    BANKSEL TRISD
    CLRF TRISD
    BANKSEL LATD
    CLRF LATD

;----------------------------------
; Load 7-seg table (0–F)
;----------------------------------
    LFSR 0, TABLE_START

    MOVLW 0x3F ; 0
    MOVWF TABLE_START
    MOVLW 0x06 ; 1
    MOVWF 0x0B
    MOVLW 0x5B ; 2
    MOVWF 0x0C
    MOVLW 0x4F ; 3
    MOVWF 0x0D
    MOVLW 0x66 ; 4
    MOVWF 0x0E
    MOVLW 0x6D ; 5
    MOVWF 0x0F
    MOVLW 0x7D ; 6
    MOVWF 0x10
    MOVLW 0x07 ; 7
    MOVWF 0x11
    MOVLW 0x7F ; 8
    MOVWF 0x12
    MOVLW 0x6F ; 9
    MOVWF 0x13
    MOVLW 0x77 ; A
    MOVWF 0x14
    MOVLW 0x7C ; b
    MOVWF 0x15
    MOVLW 0x39 ; C
    MOVWF 0x16
    MOVLW 0x5E ; d
    MOVWF 0x17
    MOVLW 0x79 ; E
    MOVWF 0x18
    MOVLW 0x71 ; F
    MOVWF TABLE_END

    ; Start at 0
    LFSR 0, TABLE_START
    MOVF INDF0, W
    MOVWF SEG_OUT
    
; Main loop
Main:
    ; BTN_UP pressed?
    BTFSS BTN_UP
    GOTO CheckDownWithUp

    ; BTN_DOWN only?
    BTFSS BTN_DOWN
    GOTO StepDown

    GOTO Main

CheckDownWithUp:
    ; both pressed -> reset
    BTFSS BTN_DOWN
    GOTO ResetZero

    ; only up pressed
    GOTO StepUp

;----------------------------------
; Count control
;----------------------------------
StepUp:
    MOVF INDF0, W
    CPFSEQ TABLE_END
    GOTO DoIncrement
    GOTO ResetZero

StepDown:
    MOVF INDF0, W
    CPFSEQ TABLE_START
    GOTO DoDecrement
    GOTO LoadMax

;----------------------------------
; Pointer movement
;----------------------------------
DoIncrement:
    INCF FSR0L, F
    MOVF INDF0, W
    MOVWF SEG_OUT
    CALL DelayBlock
    GOTO Main

DoDecrement:
    DECF FSR0L, F
    MOVF INDF0, W
    MOVWF SEG_OUT
    CALL DelayBlock
    GOTO Main

LoadMax:
    LFSR 0, TABLE_END
    MOVF INDF0, W
    MOVWF SEG_OUT
    CALL DelayBlock
    GOTO Main

ResetZero:
    LFSR 0, TABLE_START
    MOVF INDF0, W
    MOVWF SEG_OUT
    CALL DelayBlock
    GOTO Main

;----------------------------------
; Delay routine
;----------------------------------
DelayBlock:
    MOVLW innerLoop
    MOVWF DLY1
    MOVLW outerLoop
    MOVWF DLY2
    MOVLW thirdLoop
    MOVWF DLY3

DelayLoop:
    DECF DLY1, F
    BNZ DelayLoop

    MOVLW innerLoop
    MOVWF DLY1

    DECF DLY2, F
    BNZ DelayLoop

    MOVLW outerLoop
    MOVWF DLY2

    DECF DLY3, F
    BNZ DelayLoop

    RETURN
