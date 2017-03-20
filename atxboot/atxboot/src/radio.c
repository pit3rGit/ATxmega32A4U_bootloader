#define USART_RADIO USARTD0
#define RADIO_TIMEOUT 30000
//#define RADIO_ERROR_CHAR 0x00
#define RADIO_FRAME_START 0x02
#define RADIO_FRAME_SIZE_MAX 140

char radioTimeoutFlag = 0;
char *radioFrameReceiveBuffer;

void BufferClear(char* buffer,char size){
	for(int i=0;i<size;i++){
		buffer[i] = 0;
	}
}
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
	PORTD_OUTSET = PIN6_bm;

	//CONFIG
	PORTD_DIRSET = PIN7_bm;
	PORTD_OUTSET = PIN7_bm;

	radioFrameReceiveBuffer = (char*)malloc(RADIO_FRAME_SIZE_MAX);
}
void SetRadioConfigPinLow(){
	PORTD_OUTCLR = PIN7_bm;
}
void SetRadioConfigPinHigh(){
	PORTD_OUTSET = PIN7_bm;
}
state GetRadioRtsPinState(){
	if(PIN5_bm == PORTD.IN & PIN5_bm)
		return HIGH;
	else
		return LOW;
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
	char buffer[RADIO_FRAME_SIZE_MAX];
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
	//SetRadioConfigPinLow();
	//SetRadioConfigPinHigh();
	SendBufferToRadio(buffer,length+4);
	//SetRadioConfigPinHigh();
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
void ReceiveFrameFromRadio(char *buffer){
	BufferClear(buffer,140);
	char receivedChar = ReceiveCharFromRadio();
	if(radioTimeoutFlag){
		SendString("TimeoutStartChar\n");
		return;
	}
	buffer[0]=receivedChar;
	if(RADIO_FRAME_START == receivedChar){
		SendFrame("0x02");
		char command = ReceiveCharFromRadio();
		if(radioTimeoutFlag){
			SendString("TimeoutC\n");
			return;
		}
		buffer[1] = command;
		char length = ReceiveCharFromRadio();
		if(radioTimeoutFlag){
			SendString("TimeoutL\n");
			return;
		}
		buffer[2] = length;
		char radioFrameBufferIterator = 3;
		for(int i=0; i<length; i++){
			receivedChar = ReceiveCharFromRadio();
			if(radioTimeoutFlag){
				SendString("TimeoutD\n");
				break;
			}
				
			buffer[radioFrameBufferIterator++] = receivedChar;
			if(radioFrameBufferIterator > RADIO_FRAME_SIZE_MAX){
				break;
			}
		}
		char checksum = ReceiveCharFromRadio();
		if(radioTimeoutFlag){
			SendString("TimeoutCRC\n");
			return;
		}
		buffer[radioFrameBufferIterator] = checksum;
	}
}
char hexToChar(char hex){
	if(hex<10)
	return hex+'0';
	else{
		return (hex-10+'A');
	}
}
char *BufferToHexAscii(char *buffer, char bufferSize){
	char *hexBuffer = (char*)malloc(bufferSize*3+1);
	int j = 0;
	for(int i=0; i<bufferSize; i++){
		hexBuffer[j++] = '|';
		hexBuffer[j++] = hexToChar(buffer[i]>>4);
		hexBuffer[j++] = hexToChar(buffer[i] & 0x0f);
	}
	hexBuffer[j] = '\0';
	return hexBuffer;
}
void SendRadioFrameToPc(){
	SendString(BufferToHexAscii(radioFrameReceiveBuffer, radioFrameReceiveBuffer[2]+4));
}

