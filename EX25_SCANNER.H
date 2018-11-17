///////////////////////////////////////////////////////////////////////////////
//	Barcode scanner
// 	Date: 2018/10/31
///////////////////////////////////////////////////////////////////////////////

#include <18F26K22.h>
#include <string.h>
#include <stdint.h>


#FUSES NODEBUG				
#FUSES NOWDT				//No Watch Dog Timer
#FUSES WDT128				//Watch Dog Timer uses 1:128 Postscale
#FUSES INTRC_IO				//Internal RC OSC, no CLKOUT
#FUSES NOFCMEN				//Fail-safe clock monitor disabled
#FUSES NOIESO				//Internal External Switch Over mode disabled
#FUSES NOBROWNOUT			//No brownout reset
#FUSES WDT_SW				//No Watch Dog Timer, enabled in Software
#FUSES NOPBADEN				//PORTB PIN are configured as digital I/O on RESET
#FUSES NOSTVREN				//Stack full/underflow will not cause reset
#FUSES NOLVP				//No low voltage programming, B3(PIC16) or B5(PIC18) used for I/O
#FUSES NOXINST				//Extended set extension and Index Addressing mode disabled
#FUSES PROTECT				//Code protected from read

#use delay(clock=4000000)

#use rs232(uart1, baud=19200, parity=N, stop=1, xmit=PIN_C6, rcv=PIN_C7, bits=8, stream=TS02, ERRORS)
#use rs232(uart2, baud=57600, parity=N, stop=1, xmit=PIN_B6, rcv=PIN_B7, bits=8, stream=EX25, ERRORS)

//PORT.A
union ra_pin_union{
	struct ra_pin_map{
		boolean RTS_PIC			:1;		//RA0	1
		boolean CTS_PIC			:1;		//RA1	1
		boolean	RA2				:1;		//RA2	1
		boolean RA3				:1;		//RA3	1
		boolean SET3			:1;		//RA4	1
		boolean SET2			:1;		//RA5	1
		boolean	RA6				:1;		//RA6	0	Debug LED
		boolean	SET1			:1;		//RA7	1
	} RA;
	int8 si8_PORTA;
}PORTA;
union ra_pin_union LATA;
#byte PORTA	= 0x0F80
#byte LATA	= 0x0F89

#define def_TRIS_A		0b10111111
#define def_WPUA		0b00000000		//No pull-up

//PORT.B
union rb_pin_union{
	struct rb_pin_map{
		boolean SS2				:1;		//RB0	1
		boolean	SCK2			:1;		//RB1	1
		boolean BUTTON			:1;		//RB2	1
		boolean SDO2			:1;		//RB3	1
		boolean RB4				:1;		//RB4	1
		boolean RB5				:1;		//RB5	1
		boolean	PIC_TX2			:1;		//RB6	0	EX25_RX
		boolean	PIC_RX2			:1;		//RB7	1
	} RB;
	int8 si8_PORTB;
}PORTB;
union rb_pin_union LATB;
#byte PORTB	= 0x0F81
#byte LATB	= 0x0F8A

#define def_TRIS_B		0b10111111
#define def_WPUB		0b00000000		

//PORT.C
union rc_pin_union{
	struct rc_pin_map{
		boolean TS02E_WU		:1;		//RC0	1
		boolean TS02E_RDY		:1;		//RC1	1
		boolean BUZZER			:1;		//RC2	0
		boolean RC3				:1;		//RC3	1
		boolean RC4				:1;		//RC4	1
		boolean RC5				:1;		//RC5	1
		boolean	PIC_TX1			:1;		//RC6	0 	TS02_RX
		boolean	PIC_RX1			:1;		//RC7	1	
	} RC;
	int8 si8_PORTC;
}PORTC;
union rc_pin_union LATC;
#byte PORTC	= 0x0F82
#byte LATC	= 0x0F8B

#define def_TRIS_C		0b10111011
#define def_WPUC		0b00000000		//No pull-up

#use fast_io(A)
#use fast_io(B)
#use fast_io(C)

#byte 	TXSTA1 	= 0x0FAC
#bit 	TRMT	= TXSTA1.1
#bit 	BRGH	= TXSTA1.2
#byte 	BAUDCON1= 0x0FB8
#bit	BRG16	= BAUDCON1.3
#byte 	SPBRGH1 = 0x0FB0
#byte 	SPBRG1 	= 0x0FAF

#byte	RCSTA1	= 0x0FAB
#bit	FERR1	= RCSTA1.2
#bit	OERR1	= RCSTA1.1
#bit	CREN1	= RCSTA1.4
#byte	RCREG1	= 0x0FAE


#byte 	TXSTA2 	= 0x0F72
#bit 	TRMT2	= TXSTA2.1
#bit 	BRGH2	= TXSTA2.2
#byte 	BAUDCON2= 0x0F70
#bit	BRG16_2	= BAUDCON2.3
#byte 	SPBRGH2 = 0x0F76
#byte 	SPBRG2 	= 0x0F75

#byte	RCSTA2	= 0x0F71
#bit	FERR2	= RCSTA2.2
#bit	OERR2	= RCSTA2.1
#bit	CREN2	= RCSTA2.4
#byte	RCREG2	= 0x0F74

#define C6   237 
#define D6   211 
#define E6   188 
#define F6   177 
#define G6   158 
#define A6   141 
#define B6   125 
#define C7   118
#define D7   105 
#define E7   94 
#define F7   88 
#define G7   78 
#define A7   70 
#define B7   62 