///////////////////////////////////////////////////////////////////////////////
//	Barcode scanner
// 	
//	MPLAB: Ver8.91
//	Complier: Ver5.048
//
//	Ver0001	2018/10/31	Create
//	Ver0002	2018/10/31	Rcv & xmit data
///////////////////////////////////////////////////////////////////////////////
#include "EX25_SCANNER.h"

#define def_TIMER1		0x3D90 //50ms

static int1 si1_232_1_ok = FALSE;
static int8 si8_rx1_cnt = 0;
static int1 si1_232_2_ok = FALSE;
static int8 si8_rx2_cnt = 0;
static int1 si1_TS02E_exist = FALSE;


#define def_BUF_MAX 255
#define def_TS02_SIZE 100

static int8 sai8_buffer_rx1[def_BUF_MAX+1];
int8 ai8_ts02_id[4];
int8 i8_ts02_id = 0;

static int8 sai8_buffer_rx2[def_BUF_MAX+1];

#include "EX25_SCANNER_FNC.c"
///////////////////////////////////////////////////////////////////////////////
void clear_232_1(){
	//Error handling
	volatile int temp;
	
	while(FERR1 != 0){
		temp = RCREG1;
	}
	if(OERR1){
		while(OERR1 != 0){
			CREN1 = 0;
		}
		CREN1 = 1;
	}
}
void clear_232_2(){
	//Error handling
	volatile int temp;
	
	while(FERR2 != 0){
		temp = RCREG2;
	}
	if(OERR2){
		while(OERR2 != 0){
			CREN2 = 0;
		}
		CREN2 = 1;
	}
}
#INT_RDA
void RDA_isr(void){
	int8 i8_temp;
	int16 i16_timeout = 0;
	
	si8_rx1_cnt = 0;
	
	while(TRUE){
		if(kbhit(TS02)){
		
			if(si8_rx1_cnt < def_BUF_MAX){
				sai8_buffer_rx1[si8_rx1_cnt] = fgetc(TS02);
				si8_rx1_cnt++;
			}
			else{
				i8_temp = fgetc(TS02);
			}
			i16_timeout = 0;
		}
		else{
			i16_timeout++;
			if(i16_timeout > 6451){
				break;
			}
		}
	}
	si1_232_1_ok = TRUE;	
	clear_232_1();
}
#INT_RDA2
void RDA2_isr(void){
	int8 i8_temp;
	int16 i16_timeout = 0;
	
	si8_rx2_cnt = 0;
	
	while(TRUE){
		if(kbhit(EX25)){
		
			if(si8_rx2_cnt < def_BUF_MAX){
				sai8_buffer_rx2[si8_rx2_cnt] = fgetc(EX25);
				si8_rx2_cnt++;
			}
			else{
				i8_temp = fgetc(EX25);
			}
			i16_timeout = 0;
		}
		else{
			i16_timeout++;
			if(i16_timeout > 6451){
				break;
			}
		}
	}
	si1_232_2_ok = TRUE;	
	clear_232_2();
}
///////////////////////////////////////////////////////////////////////////////
#INT_TIMER1
void TIMER1_isr(){
	set_timer1(def_TIMER1);	//50ms
	LATA.RA.RA6 ^= 1;
}	
///////////////////////////////////////////////////////////////////////////////
void main()
{
	setup_oscillator(OSC_4MHZ|OSC_INTRC|OSC_PLL_OFF);
	
	setup_timer_1(T1_INTERNAL|T1_DIV_BY_4);
	setup_timer_2(T2_DISABLED, 0, 1);
	setup_timer_3(T3_DISABLED);
	setup_timer_4(T4_DISABLED, 0, 1);
	setup_timer_5(T5_DISABLED);
	setup_timer_6(T6_DISABLED, 0, 1);
	
	setup_spi(FALSE);
	setup_adc(ADC_OFF);
	setup_adc_ports(NO_ANALOGS);
	setup_comparator(NC_NC_NC_NC);
	
	LATA.si8_PORTA = 0;
	LATB.si8_PORTB = 0;
	LATC.si8_PORTC = 0;

	
	port_b_pullups(def_WPUB);
	set_tris_a(def_TRIS_A);
	set_tris_b(def_TRIS_B);
	set_tris_c(def_TRIS_C);

	
	// Errata: BRGH = 1, BRG16 = 1
	// CPU:4MHZ 19200bps -> SPBRGH:SPBRG = 4000000/4/19200 -1 = 51

	BRGH2 = 1;
	BRG16_2 = 1;
	SPBRGH2 = 0;
	SPBRG2 = 16;
	
	BRGH = 1;
	BRG16 = 1;
	SPBRGH1 = 0;
	SPBRG1 = 51;
	
	set_timer1(def_TIMER1);
	clear_interrupt(INT_TIMER1);
	enable_interrupts(INT_TIMER1);
	
	si1_232_1_ok = FALSE;
	clear_232_1();
	clear_interrupt(INT_RDA);
	enable_interrupts(INT_RDA);
	
	si1_232_2_ok = FALSE;
	clear_232_2();
	clear_interrupt(INT_RDA2);
	enable_interrupts(INT_RDA2);
	
	enable_interrupts(GLOBAL);
	delay_ms(2000);


	//sound(1, E6);
	//sound(1, D6);
	//sound(1, C6);
	
	Command_Q_I();
	
	while(!si1_TS02E_exist){
		if(si1_232_1_ok){
			si1_232_1_ok = FALSE;
			if(si8_rx1_cnt >=3 ){
				if((sai8_buffer_rx1[0] == '$')&&(sai8_buffer_rx1[2] != 'r')){
					si1_TS02E_exist = TRUE;	//TS02 Connected
					break;
				}
				else si1_TS02E_exist = FALSE;
			}
			Command_Q_I(); //retry #?I	
		}
				
	}	
	get_id();
	TS02_INIT();
	
	while(TRUE){
		if(si1_232_2_ok){
			si1_232_2_ok = FALSE;
			check_ts02_rdy();
			Command_TX();
			fprintf(TS02, "\nData: %s",sai8_buffer_rx2);
			wait_ts02_complete();
			si8_rx2_cnt = 0;
			memset(sai8_buffer_rx2, 0, 255);
		}	
	}	
}	