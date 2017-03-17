#define USART_RADIO USARTD0
#define RADIO_TIMEOUT 10000
//#define RADIO_ERROR_CHAR 0x00
#define RADIO_FRAME_START 0x02
#define RADIO_FRAME_CHAR_MAX 140

char radioTimeoutFlag = 0;
char radioFrameReceiveBuffer[RADIO_FRAME_CHAR_MAX];

void radioInit(){
	//USART PORT D
	PORTD_DIRSET = PIN3_bm;
	PORTD_OUTSET = PIN3_bm;
	PORTD_DIRCLR = PIN2_bm;

	USART_RADIO.CTRLC = USART_CHSIZE_8BIT_gc;

	USART_RADIO.CTRLA = USART_RXCINTLVL_MED_gc;
	USART_RADIO.BAUDCTRLA = 23;
	USART_RADIO.BAUDCTRLB = 0;

	USART_RADIO.CTRLB = USART_RXEN_bm | USART_TXEN_bm;

	PMIC_CTRL |= PMIC_MEDLVLEN_bm;

	//RTS
	PORTD_DIRCLR = PIN5_bm;

	//DATA_REQ
	PORTD_DIRCLR = PIN4_bm;

	//RESET
	PORTD_DIRSET = PIN6_bm;

	//CONFIG
	PORTD_DIRSET = PIN7_bm;
}
void SendCharToRadio(char charToSend){
	while(!(USART_RADIO.STATUS & USART_DREIF_bm));
	USART_RADIO.DATA = charToSend;
}
void SendBufferToRadio(char *buffer, int length){
	for(int charCounter = 0; charCounter < length; charCounter++){
		SendCharToRadio(buffer[charCounter]);
	}
}
void SendCommandToRadio(char command,char length, char *data){
	char buffer[RADIO_FRAME_CHAR_MAX];
	char checksum;
	buffer[0] = 0x02;
	buffer[1] = command;
	buffer[2] = length;
	checksum = buffer[0] ^ buffer[1] ^ buffer[2];
	for(int i = 0; i<length; i++){
		buffer[i+3] = data[i];
		checksum ^= buffer[i+3];
	}
	buffer[length+3] = checksum;
	SendBufferToRadio(buffer,length+4);
}
char ReceiveCharFromRadio(){
	int timeoutCounter = 0;
	radioTimeoutFlag = 0;
	while(!(USART_RADIO.STATUS & USART_RXCIF_bm)){
		if(RADIO_TIMEOUT <= timeoutCounter++){
			radioTimeoutFlag = 1;
			return 0;
		}
	}
	return USART_RADIO.DATA;
}
void ReceiveFrameFromRadio(){
	char receivedChar = ReceiveCharFromRadio();
	radioFrameReceiveBuffer[0];
	if(RADIO_FRAME_START == receivedChar){
		char command = ReceiveCharFromRadio();
		radioFrameReceiveBuffer[1];
		char length = ReceiveCharFromRadio();
		radioFrameReceiveBuffer[2];
		char radioFrameBufferIterator = 3;
		for(int i=0; i<length; i++){
			receivedChar = ReceiveCharFromRadio();
			if(radioTimeoutFlag)
				break;
			radioFrameReceiveBuffer[radioFrameBufferIterator++] = receivedChar;
			if(radioFrameBufferIterator > RADIO_FRAME_CHAR_MAX){
				//SendFrame(ERROR_TO_LONG_FRAME);
				break;
			}
		}
		char checksum = ReceiveCharFromRadio();
		frameReceiveBuffer[radioFrameBufferIterator] = checksum;
	}
}