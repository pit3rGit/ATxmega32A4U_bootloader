/*#define USART_SERIAL                 USART0
#define USART_SERIAL_ID              ID_USART0  //USART0 for sam4l
#define USART_SERIAL_BAUDRATE        9600
#define USART_SERIAL_CHAR_LENGTH     US_MR_CHRL_8_BIT
#define USART_SERIAL_PARITY          US_MR_PAR_NO
#define USART_SERIAL_STOP_BIT        US_MR_NBSTOP_1_BIT*/

//#include <asf.h>

#define F_CPU 3686400
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
//#include <avr/iox32a4u.h>
#include "def.h"

char frameReceiveBuffer[FRAME_CHAR_MAX];
#include "usart.c"



void XtalInit(){
	CCP = CCP_IOREG_gc;
	
	OSC_XOSCCTRL = OSC_FRQRANGE_2TO9_gc | OSC_XOSCSEL0_bm | OSC_XOSCSEL1_bm; // OSC_XOSCPWR_bm

	CCP = CCP_IOREG_gc;
	OSC_CTRL = OSC_XOSCEN_bm;
	
	while(OSC_XOSCRDY_bm != (OSC_STATUS & OSC_XOSCRDY_bm));
	
	CLK_LOCK = 0;
	CCP = CCP_IOREG_gc;
	CLK_CTRL = CLK_SCLKSEL_XOSC_gc;
	
	OSC_CTRL = OSC_XOSCEN_bm;
	CLK_LOCK = 1;
}
void delay(int ms){
	while(ms--)
	_delay_ms(1);
}
void ledInit(){
	PORTA_DIRSET = PIN2_bm;
	PORTA.OUTCLR = PIN2_bm;
}
void ledShortTick(){
	delay(50);
	PORTA.OUTSET = PIN2_bm;
	delay(100);
	PORTA.OUTCLR = PIN2_bm;
	delay(50);
}
void ledLongTick(){
	delay(300);
	PORTA.OUTSET = PIN2_bm;
	delay(600);
	PORTA.OUTCLR = PIN2_bm;
	delay(300);
}
int main (void)
{
	//sysclk_init();
	XtalInit();
	delay(500);
	ledInit();
	ledShortTick();
	ledShortTick();
	ledShortTick();
	//board_init();
	UsartInit();
	

	while(1)
	{
		SendChar('A');
		ledLongTick();
	}
	
	/* Insert application code here, after the board has been initialized. */
}
