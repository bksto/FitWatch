#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "bit.h"
#include "keypad.h"
#include "lcd.h"
#include "scheduler.h"
#include "usart_ATmega1284.h"

//======= GLOBAL VARIABLES ========
unsigned short numTasks = 5;
unsigned char output = 0x00; //tmp value
int cursor_position = 22;

int hr = 0;
int min = 0;
int sec = 0;

int hr_s = 0;
int min_s = 0;
int sec_s = 0;

int max_cals = 0; 
int cals_remaining = 0; 

// Features: 
//			 enter_cals = '#'
//			 time = 'A'
//			 stop_watch = 'B'
//			 calories = 'C'
//			 enter_time/change_time = 'D'
unsigned char feature = 0;
int write = 0; 
int position_reset = 0;               
unsigned char time[6] = "00:00";
unsigned char hr_temp[2];
unsigned char min_temp[2];
int menu_cnt1 = 0;
int menu_cnt2 = 1; 
unsigned char min_tmp[2];
unsigned char time_send_over[9] = "00:00:00\n"; 
int send = 0; 


//==========================================================

//Joystick 
void A2D_init() {
	// ADEN: Enables analog-to-digital conversion
	// ADSC: Starts analog-to-digital conversion
	// ADATE: Enables auto-triggering, allowing for constant analog to digital conversions.
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

void Set_A2D_Pin(unsigned char pinNum)
{
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
}

//Joystick 
//LEFT AND RIGHT
enum joystick_states_x{on}; 
int move_x(int state)
{
	Set_A2D_Pin(0);
	unsigned short input = 0;
	input = ADC;
	tasks[3].period = 200;
	
		if(input > 600) {
			if(menu_cnt1 >= 3){
				menu_cnt1 = 3;
			}
			else{
				menu_cnt1++;
				menu_cnt2 = menu_cnt1+1; 
			}
		}
			
		if(input < 400){
			if(menu_cnt1 <= 0){
				menu_cnt1 = 0;
			}
			else{
				menu_cnt1--;
				menu_cnt2 = menu_cnt1+1; 
			}
				
		}
	
	
	return state;
}

//UP & DOWN
int move_y(int state)
{
	Set_A2D_Pin(1);
	unsigned short input = 0;
	input = ADC;
	tasks[4].period = 200;

			if(input > 600) {
				if(menu_cnt1 >= 3){
					menu_cnt1 = 3;
				}
				else{
					menu_cnt1++;
					menu_cnt2 = menu_cnt1+1; 
				}
			}
			if(input < 400){
				
				if(menu_cnt1 <= 0){
					menu_cnt1 = 0;
				}
				else{
					menu_cnt1--;
					menu_cnt2 = menu_cnt1+1; 
				}
			}
			
			
	return state;
}



//=================== MENU =====================
char* menu_options[] = {"    - MENU -", "A) Time", "B) Stopwatch", "C) Calorie Tracker", "D) Adjust clock"};

void show_menu(){
	LCD_ClearScreen();
	LCD_DisplayString(1, menu_options[menu_cnt1]);
	LCD_DisplayString(17, menu_options[menu_cnt2]); 
}


void show_time(){
	unsigned char h[2]; //= hr_temp;
	unsigned char m[2];  //= min_temp;
	int i = 22; //cursor position
	LCD_ClearScreen();
	LCD_DisplayString(2, "-Current Time -");

	LCD_Cursor(i);
	if(hr < 10){
		LCD_WriteData('0');
		i++;
	}
	sprintf(h, "%d", hr);
	LCD_DisplayString(i, h);
	
	i++;
	if(hr >= 10) {
		i++;
	}
	
	LCD_Cursor(i);
	LCD_WriteData(':');
	
	i++;
	
	if(min < 10){
		LCD_WriteData('0');
		i++;
	}
	sprintf(m, "%d", min);
	LCD_DisplayString(i, m);

}

void show_stopwatch(){
	
	char h[10];
	char m[10];
	char s[10];
	int i = 20; //cursor position
	
	LCD_ClearScreen();
	LCD_DisplayString(1, "-- Stopwatch --");

	LCD_Cursor(i);
	if(hr_s < 10){
		LCD_WriteData('0');
		i++;
	}
	sprintf(h, "%d", hr_s);
	LCD_DisplayString(i, h);
	
	i++;
	
	LCD_Cursor(i);
	LCD_WriteData(':');
	
	i++;
	
	if(min_s < 10){
		LCD_WriteData('0');
		i++;
	}
	sprintf(m, "%d", min_s);
	LCD_DisplayString(i, m);
	
	i++;
	if(min_s >= 10) {
		i++;
	}
	LCD_Cursor(i);
	LCD_WriteData(':');
	i++;
	
	if(sec_s < 10){
		LCD_WriteData('0');
		i++;
	}
	sprintf(s, "%d", sec_s);
	LCD_DisplayString(i, s);
	
}


void show_resetTime(unsigned char num, int y){
	if(y == 1){
		 hr_temp[0] = num;
		 time[0] = num;  
	}
	else if(y == 2){
		hr_temp[1] = num; 
		time[1] = num;  

	}
	else if(y == 3){
		min_temp[0] = num; 
		time[3] = num;  
	}
	else if(y == 4){
		min_temp[1] = num;
		time[4] = num; 
		hr = atoi(hr_temp); 
		min = atoi(min_tmp); 
	}
	strncpy(min_tmp, min_temp, 2);

	LCD_DisplayString(22, time);
}

unsigned char max_cals_temp[5] = "0000"; 
void show_resetCals(unsigned char num, int y){

	if(y == 1){
		max_cals_temp[0] = num; 
	}
	else if(y == 2){
		max_cals_temp[1] = num; 
	}
	else if(y == 3){
		max_cals_temp[2] = num; 
	}
	else if(y == 4){
		max_cals_temp[3] = num; 
		max_cals = atoi(max_cals_temp); 
	}
	
	LCD_DisplayString(27, max_cals_temp);	
}

unsigned char sub_cals_temp[5] = "0000";
int t = 0; 
void show_subCals(unsigned char num, int y){

	if(y == 1){
		sub_cals_temp[0] = num; 
	}
	else if(y == 2){
		sub_cals_temp[1] = num; 
	}
	else if(y == 3){
		sub_cals_temp[2] = num; 
	}
	else if(y == 4){
		sub_cals_temp[3] = num; 
		t = atoi(sub_cals_temp); 
		cals_remaining = max_cals - t; 
	}

	
	LCD_DisplayString(23, sub_cals_temp);	
}

void show_calories(){
	
	unsigned char tmp[5]; 
	sprintf(tmp, "%d", cals_remaining); 

	LCD_ClearScreen();
	LCD_DisplayString(1, "You have ");
	LCD_DisplayString(10, tmp);
	LCD_DisplayString(17, "calories left!");
}

unsigned char cnt = 0; 
enum Display_States {init_display, wait_start, display_welcome, enter_time, enter_calories, display_menu, display_time, display_stopwatch, sub_cal, display_calories, change_time };
int display(int state){
	 
	
	//transitions
	switch(state){
		case init_display: 
			state = wait_start; 
			break; 
		case wait_start: 
			state = display_welcome; 
			break;
		case display_welcome: 
			if(cnt < 20){
				state = display_welcome; 
			}
			else if(cnt >= 20){
				state = enter_time; 
				position_reset = 0; 
				write = 1; 
				cnt = 0; 
			}
			break; 
		case enter_time: 
			feature = 'D'; 
			if(output != '#'){
				state = enter_time; 
			}
			else if(output == '#'){
				state = enter_calories; 
				output = '\0'; 
				position_reset = 0; 
			}
			break;
		case enter_calories: 
			feature = '#';
			if(output != '#'){
				state = enter_calories;
			}
			else if(output == '#'){
				state = display_menu;
				//menu_cnt = 0; 
			}
			break; 
		case display_menu:
			if(feature == '0'){
				state = display_menu; 
				//menu_cnt = 0; 
			}
			else if(feature == 'A'){
				state = display_time; 
			}
			else if(feature == 'B'){
				state = display_stopwatch; 
			}
			else if(feature == 'C'){
				state = sub_cal; 
				position_reset = 0; 
				output = '\0'; 
			}
			else if(feature == 'D'){
				state = change_time; 
				position_reset = 0; 
				hr = 0; 
				min = 0; 
				time[0] = 0; 
				time[1] = 0; 
				time[3] = 0; 
				time[4] = 0; 
			}
			break;
		case display_time:
			if(feature == '*'){
				state = display_menu; 
				//menu_cnt = 0; 
			}
			else {
				state = display_time; 
			}
			break; 
		case display_stopwatch:
			if(feature == '*'){
				state = display_menu;
				//menu_cnt = 0; 
			}
			else {
				state = display_time;
			}
			break;
		case sub_cal:
			feature = 'C'; 
			if(output != '#'){
				state = sub_cal; 
			}
			else if(output == '#'){
				state = display_calories; 
				output = '\0'; 
				position_reset = 0; 
			}
			break;
		case display_calories:
			if(feature == '*'){
				state = display_menu;
				//menu_cnt = 0; 
			}
			else {
				state = display_time;
			}
			break;

		case change_time: 
			//feature = 'D';
			if(output == '#'){
				state = display_menu;
				//menu_cnt = 0; 
				output = '\0';
				position_reset = 0;
				feature = '*';
			}
			else{
				state = change_time;
			}
			break;
		default: 
			state = init_display;
			break;
			
	}	
	
	//actions
	switch(state){
		case init_display: 
			break; 
		case wait_start: 
			cnt = 0; 
			break; 
		case display_welcome: 
			cnt++; 
			LCD_ClearScreen();
			LCD_DisplayString(3, " WELCOME TO");
			LCD_DisplayString(21, "FITWATCH");
			break; 
		case enter_time: 
			LCD_ClearScreen();
			LCD_DisplayString(1, "ENTER TIME: ");
			LCD_Cursor(cursor_position); 
			LCD_DisplayString(22, "00:00"); 
			show_resetTime(output, position_reset); 
			break; 
		case enter_calories: 
			LCD_ClearScreen();
			LCD_DisplayString(1, "ENTER MAX       CALORIES: ");
			show_resetCals(output, position_reset); 
			break; 
		case display_menu: 
			show_menu(); 
			break; 
		case display_time:
			
			if(feature == 'A')
				show_time();
			
			break; 
		case display_stopwatch:
			if(feature == 'B'){
				show_stopwatch(); 
			}
			break; 
		case sub_cal: 
			if(feature == 'C'){
				LCD_ClearScreen(); 
				LCD_DisplayString(1, "Calorie intake: "); 
				LCD_Cursor(cursor_position);
				LCD_DisplayString(23, "0000");
				show_subCals(output, position_reset); 
			}
			break; 
		case display_calories:
			 if(feature == 'C')
			 	show_calories(); 
			break;
		case change_time:
			if(feature == 'D'){
				LCD_ClearScreen();
				LCD_DisplayString(1, "ENTER TIME: ");
				LCD_Cursor(cursor_position);
				LCD_DisplayString(22, "00:00");
				show_resetTime(output, position_reset);
			}
			break;
			
		default: 
			break; 
	}
	
	return state; 
}


int first = 1; 
//===================== GET KEYPAD VALUE =========================
enum states{init_keypad, go, goW};
int get_keypad(int state)
{
	//unsigned char B2 = ~PINB & 0x04;
	int i = 0;
	unsigned char B = ~PINB;
	char x = GetKeypadKey();
	
	//transiitions
	switch(state)
	{
		case init_keypad:
			state = go;
			break;
		
		case go:
			if(x == '\0')
			state = goW;
			break;
			
		case goW:
			if(x != '\0')
			state = go;
			break;
		
		default:
			break;
	}
	
	//action
	switch(state)
	{
		case init_keypad:
			break;
		case go:
			switch(x)
			{
				case '1':
					//output = 0x01;
					output = '1';
					break; // hex equivalent
				case '2':
					//output = 0x02;
					output = '2';
					break;
				case '3':
					//output = 0x03;
					output = '3';
					break;
				case '4':
					//output = 0x04;
					output = '4';
					break;
				case '5':
					//output = 0x05;
					output = '5';
					break;
				case '6':
					output = '6';
					//output = 0x06;
					break;
				case '7':
					output = '7';
					//output = 0x07;
					break;
				case '8':
					//output = 0x08;
					output = '8';
					break;
				case '9':
					//output = 0x09;
					output = '9';
					break;

				//others
				case 'A':
					//output = 17;
					feature = 'A';
					break;
				case 'B':
					//output = 18;
					feature = 'B';

					if(first != 1){
						unsigned char temp_h[2]; 
						unsigned char temp_m[2]; 
						unsigned char tm[2]; 
						unsigned char temp_s[2]; 

						sprintf(temp_h, "%d", hr_s); 
						sprintf(temp_m, "%d", min_s); 
						sprintf(tm, "%d", min_s); 
						sprintf(temp_s, "%d", sec_s);

						time_send_over[2] = ':'; 
						time_send_over[5] = ':';

						if(hr_s < 10){
							time_send_over[0] = '0'; 
							time_send_over[1] = temp_h[0]; 
						}
						else{
							time_send_over[0] = temp_h[0]; 
							time_send_over[1] = temp_h[1]; 
						}
							
						if(min_s < 10){
							time_send_over[3] = '0'; 
							time_send_over[4] = temp_m[0]; 
						}
						else{
							time_send_over[3] = temp_m[0]; 
							time_send_over[4] = temp_m[1]; 
						}
						
							
						if(sec_s < 10){
							time_send_over[6] = '0'; 
							time_send_over[7] = temp_s[0]; 
						}
						else{
							time_send_over[6] = temp_s[0]; 
							time_send_over[7] = temp_s[1]; 
						}
					}
						
					first = 0;

					send = 1; 
					hr_s = 0;
					min_s = 0;
					sec_s = 0;
					break;
				case 'C':
					feature = 'C';
					strncpy(sub_cals_temp, "0000", 4);
					break;
				case 'D': 
					feature = 'D';  
					break;
				case '*':
					//output = -6;
					feature = '*';
					first = 1; 
					strncpy(time_send_over, "00:00:00", 8);
					break;
				case '0': 
					output = '0'; 
					break;
				case '#': 
					//output = -13;
					output = '#'; 
					break;
				default: 
					output = '0'; 
					break; // Should never occur. Middle LED off.
			}
			cursor_position++;
			position_reset++; 
			break;
		case goW:
			break;
		default:
			break;
	}
	
	return state;
}


//======= TIME =======
enum time_states{init_time, wait_60, update};

int TimeTick(int state){
	
	//transitions
	switch(state){
		case -1:
			state = init_time;
			break;
		case init_time:
			state = wait_60;
			break;
		case wait_60:
			if(sec < 59 || sec_s < 59){
			//if(sec < 59){
				state = wait_60;
			}
			else if(sec >= 59 || sec_s >= 59){
			//else if(sec >= 59){
				state = update;
			}

			break;
		case update:
			state = wait_60;
			break;
	}
	
	//actions
	switch(state){
		case init_time:
		break;
		case wait_60:
			sec++;

			if(feature == 'B'){
				sec_s++;
				show_stopwatch();
			}
			break;
		case update:
			
		
			sec++; 
			if(sec >= 59) {
				min++;
				if(min >= 59){
					hr++;
					min = 0;
				}
			}

			if(feature == 'A'){
				show_time();
			}
			if(feature == 'B'){
				if(sec_s >= 59){
					min_s++; 
					sec_s = 0; 
				}
				if(min_s >= 59){
					hr_s++;
					min_s = 0;
				}
				
				show_stopwatch();
			}
			break;
		default: 
			break; 
	}
	return state;
}


int i = 0; 
int start = 1; 
enum send_data{init_send, wait_send}; 
int send_data(int state){
	
	switch(state){
		case init_send: 
			state = wait_send; 
			break; 
		case wait_send: 
			state = wait_send; 
			break; 
	}
	switch(state){
		case init_send: 
			break; 
		case wait_send: 
			if(USART_IsSendReady(0) && send == 1 && first == 0) {
				USART_Send(time_send_over[i],0);
				i++; 
				USART_Flush(0);
			}
			if(i > 8){
				i = 0;
				send = 0; 
			}
			
			break; 
	}

	return state;
}


int main(void){
	DDRA = 0x0C; PORTA = 0xF3; 
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	//DDRD = 0xFF; PORTD = 0x00;
	
	A2D_init();
	LCD_init();
	initUSART(0);
	USART_Flush(0);
	
	tasksNum = 6; // declare number of tasks
	task tsks[6]; // initialize the task array
	tasks = tsks; // set the task array
	
	// define tasks
	unsigned char i=0; // task counter
	tasks[i].state = init_keypad;
	tasks[i].period = 500;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &get_keypad;
	i++;
	tasks[i].state = init_display;
	tasks[i].period = 1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &display;
	i++;
	tasks[i].state = init_time;
	tasks[i].period = 1000;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TimeTick;
	i++;
	tasks[i].state = on;
	tasks[i].period = 200;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &move_x;
	i++;
	tasks[i].state = on;
	tasks[i].period = 200;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &move_y;
	i++; 
	tasks[i].state = init_send;
	tasks[i].period = 100;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &send_data;

	
	TimerSet(100);
	TimerOn();
	
	while(1)
	{

	}
}