//////////////////////////////////////////////////////////////////
void eeprom_write_string(unsigned int8 addr,unsigned char*str){
	while(*str){
		write_eeprom(addr,*str);
		addr++;
		str++;
	}
}

void eeprom_read_string(unsigned int8 addr, unsigned char* str,unsigned int8 len){
	unsigned int8 i;
	for ( i = 0;i<len;i++){
   		str[i]=read_eeprom(addr+i);
	}
	str[len]=0;
}
void delay_about_1ms(){
	int8 i;
	for(i = 0; i < 165; i++){}
}
void delay_about_ms(int16 i16_ms){
	int16 i;
	for(i = 0; i < i16_ms; i++){
		delay_about_1ms();
	}
}
int8 mid(int16 i16_data, int8 i8_no){
	int8 i;
	int16 i16_temp;
	int16 i16_rdata;
	
	i16_temp = 0;
	i16_rdata = 0;
	for(i = 0; i < i8_no; i++){
		i16_temp = (i16_data/10)*10;
		i16_rdata = i16_data - i16_temp;
		i16_data = i16_data/10;
	}
	return (i16_rdata);
}
int8 hex2asc(int8 data){
	switch(data){
	case 0:
		return('0');			
	case 1:
		return('1');
	case 2:
		return('2');			
	case 3:
		return('3');
	case 4:
		return('4');			
	case 5:
		return('5');
	case 6:
		return('6');			
	case 7:
		return('7');
	case 8:
		return('8');			
	case 9:
		return('9');
	case 10:
		return('A');			
	case 11:
		return('B');
	case 12:
		return('C');			
	case 13:
		return('D');
	case 14:
		return('E');			
	case 15:
		return('F');			
	default:
		break;
	}
}			
uint8_t ascii2hex(uint8_t value){
	if((value) >= 0x41){
		return (value - 0x37);
	}
	else return (value - 0x30);
}
///////////////////////////////////////////////////////////////////
void check_ts02_rdy(){
	if(PORTC.RC.TS02E_RDY){
		while(PORTC.RC.TS02E_RDY){}
		delay_about_1ms();
	}
}
void wait_ts02_complete(){
	//add 1ms margin
	while(!TRMT){}
	delay_about_1ms();
	//wait ts02 process
	check_ts02_rdy();
}
void tx_232(int8 i8_data){
	putc(i8_data, TS02);
}
void tx_232_2(int8 i8_data){
	putc(i8_data, EX25);
}
void Command_TX(){
	check_ts02_rdy();
	
	tx_232('#');
	tx_232('T');
	tx_232('X');
	tx_232(0x0D);

	wait_ts02_complete();
}
void Command_SB(){
	check_ts02_rdy();
	
	tx_232('#');
	tx_232('S');
	tx_232('B');
	tx_232(0x0D);

	wait_ts02_complete();
}
void Command_CH(int8 i8_no){
	check_ts02_rdy();
	
	tx_232('#');
	tx_232('C');
	tx_232('H');
	tx_232(hex2asc(mid(i8_no, 2)));
	tx_232(hex2asc(mid(i8_no, 1)));
	tx_232(0x0D);

	wait_ts02_complete();
}	
void Command_LN(int8 i8_no){
	check_ts02_rdy();
	
	tx_232('#');
	tx_232('L');
	tx_232('N');
	tx_232(hex2asc(mid(i8_no, 3)));
	tx_232(hex2asc(mid(i8_no, 2)));
	tx_232(hex2asc(mid(i8_no, 1)));
	tx_232(0x0D);

	wait_ts02_complete();
}
void Command_ID(int16 i16_no){
	check_ts02_rdy();
	
	tx_232('#');
	tx_232('I');
	tx_232('D');
	tx_232(hex2asc(mid(i16_no, 4)));
	tx_232(hex2asc(mid(i16_no, 3)));
	tx_232(hex2asc(mid(i16_no, 2)));
	tx_232(hex2asc(mid(i16_no, 1)));
	tx_232(0x0D);

	wait_ts02_complete();
}	
void command_Q_I(){
	check_ts02_rdy();
	fprintf(TS02, "#?I");
	putc(0x0D,TS02);
	wait_ts02_complete();
}	
void get_id (void){
	check_ts02_rdy();
	fprintf(TS02, "#?I");
	putc(0x0D,TS02);
	wait_ts02_complete();
	
	memset(ai8_ts02_id, 0, 5);
	
	if (si1_232_1_ok){
		for(int u = 0; u < strlen(sai8_buffer_rx1)-1; u++){
			if((sai8_buffer_rx1[u] == '$')&&(sai8_buffer_rx1[u]) != 'E'){
				strcpy(&sai8_buffer_rx1[u], &sai8_buffer_rx1[u+1]);
				u--;
			}
		}
	}
	//strcpy(ai8_ts02_id, sai8_buffer_rx1);
	ai8_ts02_id[0] = sai8_buffer_rx1[0];
	ai8_ts02_id[1] = sai8_buffer_rx1[1];
	ai8_ts02_id[2] = sai8_buffer_rx1[2];
	ai8_ts02_id[3] = sai8_buffer_rx1[3];
	
	i8_ts02_id = ascii2hex(sai8_buffer_rx1[0])*4096+ascii2hex(sai8_buffer_rx1[1])*256+ascii2hex(sai8_buffer_rx1[2])*16+ascii2hex(sai8_buffer_rx1[3]);
	//clear_sai8_buffer_rx1();
}
void TS02_INIT(void){
	check_ts02_rdy();
	fprintf(TS02, "#CG02");
	putc(0x0D,TS02);
	delay_ms(200);
	wait_ts02_complete();
	check_ts02_rdy();
	fprintf(TS02, "#ID1D70");
	putc(0x0D,TS02);
	delay_ms(200);
	wait_ts02_complete();
	check_ts02_rdy();
	Command_TX();
	fprintf(TS02, "\nTS02 Available-ID: %s",ai8_ts02_id);
	wait_ts02_complete();
}
///////////////////////////////////////////////////////////////////////////////
void debug (void){
	fprintf(TS02, "\nRESET:%sV0024",ai8_ts02_id );
	check_ts02_rdy();
}

//////////////////////////////////////////////////////////////////////////////////
void sound(int s, int8 t){
	setup_ccp1(CCP_PWM);
   	setup_timer_2(T2_DIV_BY_16, t, 1);
	int n = 0;
   	while (n<s)	{
		set_pwm1_duty(300);
	   	delay_ms(50);
		set_pwm1_duty(1023);
	   	delay_ms(25);
		n++;
	}
	setup_ccp1(CCP_OFF);
}
////////////////////////////////////////////////////////////////////