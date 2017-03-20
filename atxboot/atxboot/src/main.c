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
#include <stdlib.h>
//#include <avr/iox32a4u.h>
#include "def.h"
typedef enum {HIGH,LOW} state;
char frameReceiveBuffer[FRAME_CHAR_MAX];
#include "usart.c"
#include "radio.c"



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
	radioInit();
	
	char command;
	char len;
	char *data;
	char *buffer;

	Send("Set command mode\n",17);
	SetRadioConfigPinLow();
	SetRadioConfigPinHigh();
	//SendCommandToRadio(0x44,0x01,0x10);
	ReceiveFrameFromRadio(radioFrameReceiveBuffer);
	SendString("Odpowiedz CM: ");
	SendRadioFrameToPc();
	SendString(":END\n");

	while(1)
	{
		//SendFrame("Podaj komende dla radia: ");
		//command = ReceiveChar();
		//SendFrame("Podaj ilosc bajtow danych: ");
		//len = ReceiveChar();
		//SendFrame("Podaj dane: ");
		//data = (char*)malloc(len);
		//ReceiveNChars(data,len);
		//free(data);
		
		ledLongTick();

		Send("Ask for serial number\n",22);
		SendCommandToRadio(0x0b,0x00,0x00);
		ReceiveFrameFromRadio(radioFrameReceiveBuffer);
		SendString("Odpowiedz: ");
		SendRadioFrameToPc();
		SendString(":END\n");
		//Send(BufferToHexAscii(radioFrameReceiveBuffer, radioFrameReceiveBuffer[2]+4),(radioFrameReceiveBuffer[2]+4)*3+1);
		ledLongTick();

	
	}
	
	/* Insert application code here, after the board has been initialized. */
}
