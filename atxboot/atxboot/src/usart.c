//#include "../def.h"
void UsartInit(){
	PORTE_DIRSET = PIN3_bm;
	PORTE_OUTSET = PIN3_bm;
	PORTE_DIRCLR = PIN2_bm;

	USARTE0_CTRLC = USART_CHSIZE_8BIT_gc;

	USARTE0_CTRLA = USART_RXCINTLVL_MED_gc;
	USARTE0_BAUDCTRLA = 64;//23;
	USARTE0_BAUDCTRLB = 160;//0;

	USARTE0_CTRLB = USART_RXEN_bm | USART_TXEN_bm;

	PMIC_CTRL |= PMIC_MEDLVLEN_bm;
}
void SendChar(char charToSend){
	while(!(USARTE0.STATUS & USART_DREIF_bm));
	USARTE0.DATA = charToSend;
}
void Send(char *buffer, int length){
	for(int charCounter = 0; charCounter < length; charCounter++){
		SendChar(buffer[charCounter]);
	}
}
void SendFrame(char *buffer){
	SendChar(FRAME_START);
	for(uint8_t charCounter = 0; (buffer[charCounter] != FRAME_ENDING) && (buffer[charCounter] != '\0') && (charCounter < FRAME_CHAR_MAX); charCounter++){
		SendChar(buffer[charCounter]);
	}
	SendChar(FRAME_ENDING);
	_delay_us(DELAY_AFTER_FRAME_SENDING);
}
void Send1CharFrame(char buffer){
	SendChar(FRAME_START);
	SendChar(buffer);
	SendChar(FRAME_ENDING);
}
void SendString(char* str){
	for(int i=0;str[i]!='\0' && i < 140; i++){
		SendChar(str[i]);
	}
}

char ReceiveChar(){
	int timeoutCounter = 0;
	while(!(USARTE0.STATUS & USART_RXCIF_bm)){
		if(TIMEOUT <= timeoutCounter++){
			//SendFrame(ERROR_RC_TIMEOUT);
			//return ERROR_CHAR;
		}
	}
	return USARTE0.DATA;
}
void ReceiveNChars(char *buffer, char countOfChars){
	if(countOfChars == 0)
	return;
	for(int i = 0; i<countOfChars; i++){
		buffer[i] = ReceiveChar();
	}
}
void ReceiveFrame(){
	char receivedChar = ReceiveChar();
	if(FRAME_START == receivedChar){
		uint8_t frameBufferIterator = 0;
		while(FRAME_ENDING != (receivedChar = ReceiveChar())){
			if(ERROR_CHAR == receivedChar)
			break;
			frameReceiveBuffer[frameBufferIterator++] = receivedChar;
			if(frameBufferIterator > FRAME_CHAR_MAX){
				SendFrame(ERROR_TO_LONG_FRAME);
				break;
			}
		}
		frameReceiveBuffer[frameBufferIterator] = FRAME_ENDING;
	}
}