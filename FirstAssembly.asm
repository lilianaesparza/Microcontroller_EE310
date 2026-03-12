//-----------------------------
// Title: Temperature Control System Using PIC18F
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
// PORTD.2 → Connected to the COOLING system
// PORTD.1 → Connected to the HEATING system
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
    goto start	    ;jumps to main program

PSECT code
start:

;----------------
; Program Inputs
;----------------
#define measuredTempInput  -5
#define refTempInput       15
		    
;-------------
; Definitions
;-------------
#define SWITCH LATD,2
#define LED0   PORTD,0
#define LED1   PORTD,1 
   
; Program Constants
REG10   EQU     10h	;temp storage for refTemp conversion
REG11   EQU     11h	;temp storage for measuredTemp conversion
REG01   EQU     1h

; Main Code
;===========================
; Initialize PORTD
CLRF    TRISD          ; Clear TRISD register → set all PORTD pins as outputs
CLRF    LATD           ; Clear LATD register → ensure all output pins start LOW (LEDs OFF)

; Load temperature inputs into registers
MOVLW   refTempInput  
MOVWF   0x20           

MOVLW   measuredTempInput 
MOVWF   0x21            

; Compare measuredTemp and refTemp
MOVF    0x20, W        ; Move reference temperature (refTemp) into W register
SUBWF   0x21, W        ; W = measuredTemp - refTemp
                       ; Result determines comparison outcome

BZ	    TEMPEQUAL       ; Branch if result = 0 → temperatures are equal
BC	    measuredTempGREATER ; Branch if carry set → measuredTemp > refTemp
BRA	    measuredTempLESS    ; Otherwise → measuredTemp < refTemp


; Control Functions
TEMPEQUAL:             ; Case: measuredTemp == refTemp
                      ; No heating or cooling needed
CLRF    0x22         
BCF	    LATD,1        ; Turn OFF heating indicator LED (RD1)
BCF	    LATD,2        ; Turn OFF cooling indicator LED (RD2)
BRA	    CONTINUE      


measuredTempGREATER:  ; Case: measuredTemp > refTemp
                      ; Temperature too high → activate cooling
MOVLW   0x02
MOVWF   0x22         
BSF	    LATD,2        ; Turn ON cooling LED (RD2)
BCF	    LATD,1        
BRA	    CONTINUE


measuredTempLESS:     ; Case: measuredTemp < refTemp
                      ; Temperature too low → activate heating
MOVLW   0x01
MOVWF   0x22          
BSF	    LATD,1        ; Turn ON heating LED (RD1)
BCF	    LATD,2        ; Ensure cooling LED (RD2) is OFF
BRA	    CONTINUE

CONTINUE:
; HEX to Decimal Conversion
; Reference Temperature
CLRF    0x60          ; Clear ones digit storage
CLRF    0x61          ; Clear tens digit storage
CLRF    0x62          ; Clear hundreds digit storage (not used but reserved)

MOVF    0x20, W       
MOVWF   REG10         ; Copy value into working register REG10

TENLOOPREF:           ; Loop to count how many 10s are in the value
MOVLW   10            
SUBWF   REG10, F      
BNC	STOREREF       
INCF    0x61, F       
BRA	TENLOOPREF  


STOREREF:
MOVLW   10
ADDWF   REG10, F      

MOVF    REG10, W
MOVWF   0x60        

; HEX to Decimal Conversion
; Measured Temperature
CLRF    0x70          ; Clear ones digit storage
CLRF    0x71          ; Clear tens digit storage
CLRF    0x72          ; Clear hundreds digit storage (reserved)

MOVF    0x21, W       
MOVWF   REG11         

TENLOOPMEASURED:      ; Loop to extract tens digit
MOVLW   10
SUBWF   REG11, F     
BNC	STOREMEASUREDVALUE 
INCF    0x71, F       
BRA	TENLOOPMEASURED


STOREMEASUREDVALUE:
MOVLW   10
ADDWF   REG11, F      ; Restore last subtraction

MOVF    REG11, W
MOVWF   0x70          ; Store remaining value as ones digit

; End of Program (Infinite Loop)
ENDPROGRAM:
BRA	    ENDPROGRAM    

END
