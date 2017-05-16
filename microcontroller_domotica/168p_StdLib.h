//-----------------------------------------------------------------------------------------------#
// Created by Joostens Tomek																	 # 			
// Library to control GPIO,ADC,TIMERS,INTERRUPTS,... with Atmega168P							 #
// Feel free to modify or use this software. 												     #
// You can send me a mail(joostenstomek@gmail.com) with your own adapted version of my software  #
// Version: V0.2																			     #
//-----------------------------------------------------------------------------------------------#

#include "defines.h" 						//includes defines 



//--------------------------------------------------------------------------------------------------------------//
// 											Pin configurations													//
//--------------------------------------------------------------------------------------------------------------//

/**
  * @brief  Function to configure pins as input or output
  * @param  pin: select pin which you want to configure
  * @param  state: select if pin is input or output
  * @retval None
  */
void pinMode(volatile uint8_t pin,volatile uint8_t state)
{
	volatile uint8_t newPin; 				//declare a new variable

	if(pin >=2 && pin <= 7)				    //checking if parameter is between a certain range [2-7]
	{
		if(state == 1) 					    //if pin state is 1 then we set pin as an output
			DDRD|= (1 << pin); 			    //pin high
		else 						        //else we set pin as an input
			DDRD &= ~(1 << pin); 			//pin low
	}

	else if(pin >= 8 && pin <= 12) 			//checking if parameter is between a certain range [8-13]
	{
		newPin = pin - PORTCOUNT; 			//initialize variable
		
		if(state == 1)					    //if pin state is 1 then we set pin as an output
			DDRB |= (1 << newPin); 			//pin output
		else 						        //else we set pin as an input
			DDRB &= ~(1 << newPin); 		//pin input
	}
}

/**
  * @brief  Function to configure pins as high or low
  * @param  pin: select pin which you want to configure
  * @param  state: select if pin is low or high
  * @retval None
  */
void digitalWrite(volatile uint8_t pin,volatile uint8_t state)
{
	volatile uint8_t newPin;				//declare a new variable

	if(pin >=2 && pin <= 7)				 	//checking if parameter is between a certain range [2-7]
	{
		if(state == 1)						//if pin state is 1 then we set pin as a high pin
			PORTD|= (1 << pin); 			//pin high
		else								//else we set pin as a low pin
			PORTD &= ~(1 << pin); 			//pin low
	}

	else if(pin >= 8 && pin <= 12) 			//checking if parameter is between a certain range [8-13]
	{
		newPin = pin - PORTCOUNT; 			//initialize variable

		if(state == 1) 						//if pin state is 1 then we set pin as a high pin
			PORTB |= (1 << newPin); 		//pin high
		else 								//else we set pin as a low pin
			PORTB &= ~(1 << newPin); 		//pin low
	}
}

/**
  * @brief  Function toggle pins
  * @param  pin: select pin which you want to toggle
  * @retval None
  */
void digitalTogglePin(volatile uint8_t pin)
{
	volatile uint8_t newPin;

	if(pin >=2 && pin <= 7)
	{
		PORTD ^= (1 << pin); 
	}
	else if(pin >= 8 && pin <= 12)	
	{
		newPin = pin - PORTCOUNT;
		PORTB ^= (1 << newPin);
	}
}

/**
  * @brief  Function to read digital pin
  * @param  pin: select pin which you want to read
  * @retval 1 or 0
  */
int digitalRead(volatile uint8_t pin)
{
	volatile uint8_t newPin;				//declare a new variable

	if((pin >=2 && pin <= 7))				//checking if parameter is between a certain range [2-7]
	{
		return PIND & (1 << pin);			//checks if PIND0 is pressed
	}

	else if(pin >= 8 && pin <= 13) 			//checking if parameter is between a certain range [8-13]
	{
		newPin = pin - PORTCOUNT; 			//initialize variable
		return PINB & (1 << newPin);		//checks if PIND0 is pressed
	}		
}



//--------------------------------------------------------------------------------------------------------------//
// 											  Shiftregister				           								//
//--------------------------------------------------------------------------------------------------------------//

/**
  * @brief  Function to calculate the absolute pin postion 
  * @param  shiftregisterNumber: select which shiftregister to choose
  * @param  position: select pin which you want to read
  * @retval Absolute position
  */
int calculateShiftPosition(int shiftregisterNumber,int position)
{
	int absolutePosition = 0;
	return absolutePosition = ((shiftregisterNumber-1) * 8) + position;
}

/**
  * @brief  Function to shift pins from shiftregister
  * @param  shift: select which master shiftregister you want to use
  * @retval None
  */
void shiftOut(ShiftRegister shift)
{
	digitalWrite(shift.STCP_pin, LOW);
	for (int i = AMOUNT_OF_SHIFTREGISTERS * PORTCOUNT; i>=0; i--)
	{
		digitalWrite(shift.SHCP_pin, LOW);
		digitalWrite(shift.DS_pin, shift.reg[i]);
		digitalWrite(shift.SHCP_pin, HIGH);
	}
	digitalWrite(shift.STCP_pin, HIGH);
}

/**
  * @brief  Function to write date to a shiftregister. Counting starts from 0 to 7
  * @param  shift: select the master shiftregister you want to use
  * @param  shiftregisterNumber: select which shiftregister you want to use
  * @param  pin: select relative pin which you want to configure
  * @param  state: select if pin is low or high
  * @retval None
  */
void registerWrite(ShiftRegister *shift,int shiftregisterNumber,int pin,int state)
{
	int absolutePosition = calculateShiftPosition(shiftregisterNumber,pin);
	shift->reg[absolutePosition] = state; // choose which pin you want to set high or low
	shiftOut(*shift); // shifting out data
}

/**
  * @brief  Function to set shiftregister outputs low
  * @param  shift: select the master shiftregister you want to use
  * @param  shiftregisterNumber: select which shiftregister you want to use
  * @retval None
  */
void initShiftregister(ShiftRegister *shift,int shiftregisterNumber)
 {
	 for(int i = 0; i < 8;i++)
		registerWrite(shift,shiftregisterNumber,i,LOW);
 }

/**
  * @brief  Function to toggle shiftregister pins
  * @param  shift: select the master shiftregister you want to use
  * @param  shiftregisterNumber: select which shiftregister you want to use
  * @param  pin: select relative pin which you want to configure
  * @param  defaultValue: select 1 if there is no need for a known logical level or select 0 if you want to make sure the pin will be LOW
  * @retval None
  */
void registerTogglePin(ShiftRegister *shift,int shiftregisterNumber,int pin,int defaultValue)
{
	static int state[PORTCOUNT] = {1,1,1,1,1,1,1,1}; //pins are high at startup of the functioncall
	int absolutePosition = calculateShiftPosition(shiftregisterNumber,pin);

	//Make sure pins are LOW
	if (defaultValue == 0)
		state[pin] = 0;

	shift->reg[absolutePosition] = state[pin]; // choose which pin you want to set high or low
	shiftOut(*shift); // shifting out data
	state[pin] = !state[pin];
}



//--------------------------------------------------------------------------------------------------------------//
// 											   UART				           				        				//
//--------------------------------------------------------------------------------------------------------------//



/**
  * @brief  Function to setup UART
  * @param  baudrate: select communication speed
  * @retval None
  */
void initSerial(int baudRate)
{
 	UBRR0H =  (unsigned char)(BRC(baudRate) >> 8);      // The UBRR0H contains the four most significant bits
 	UBRR0L =  (unsigned char)BRC(baudRate);             // The UBRR0H contains the four least significant bits

 	UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1 << RXCIE0);   // Enable receiver,transmiter and enable RX interrupts
 	UCSR0C = (3<<UCSZ00);                               // Using 1 stopbit and 8 bit Character Size
 	sei();
}

/**
  * @brief  Function to send one byte over UART
  * @param  data: specify which byte you want to send
  * @retval None
  */
void serialSendChar(char data)
{
	while(!(UCSR0A & (1<<UDRE0))); //Wait for empty transmit buffer
	UDR0 = data; //Get and return received data from buffer 
}

/**
  * @brief  Function to send a whole string over UART
  * @param  data: specify which string you want to send
  * @retval None
  */
void serialSendString(char* sendString)
{
	while(*sendString != NULL)
	{
		serialSendChar(*sendString);
		sendString++;
	}
}

/**
  * @brief  Function to read incomming characters send over UART
  * @param  None
  * @retval received character
  */
char serialReadChar()
{
	 while ( !(UCSR0A & (1<<RXC0))); //Wait for data to be received
	 return UDR0; //Get and return received data from buffer 
}

/**
  * @brief  Function to read a whole incomming strings send over UART
  * @param  str: String you want to copy received data to
  * @retval None
  */
void serialReadString(char *str)
{
	char data;

	//copy data to string until null byte is reached
	do
	{
		data = serialReadChar();
		*str = data;
		str++;
	}while(data != 13); //CR = enter
	*str = '\0'; //add null byte
}



//--------------------------------------------------------------------------------------------------------------//
// 											   ADC				           				        				//
//--------------------------------------------------------------------------------------------------------------//



/**
  * @brief  Function initialize ADC
  * @param  None
  * @retval None
  */
void initADC()
{
	ADMUX |= (1<<REFS0); //Select Vref=AVcc
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0) | (1<<ADEN); //set prescaller to 128 and enable ADC
}

/**
  * @brief  Function to read ADC value from ADC channel
  * @param  ADCchannel: enter an integer ADC channel between (0-7)
  * @retval ADC value
  */
uint16_t readADC(uint8_t ADCchannel)
{
	ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F); //select ADC channel with safety mask
	ADCSRA |= (1<<ADSC); //single conversion mode
	while(ADCSRA & (1<<ADSC)); // wait until ADC conversion is complete
	return ADC;
}

/**
  * @brief  Function to conver ADC valur to volt
  * @param  value: enter the ADC value
  * @retval ADC valur in volt
  */
float voltADC(uint16_t value)
{
	return(value* UREF) /RESOLUTION;
}

/**
  * @brief  Function to conver ADC valur to millivolt
  * @param  volt: enter volt value
  * @retval ADC valur in millivolt
  */
int mVoltADC(float volt)
{
	return volt * 1000;
}



//--------------------------------------------------------------------------------------------------------------//
// 											   Interrupts    	           				        				//
//--------------------------------------------------------------------------------------------------------------//



/**
  * @brief  Function to setup external intterupts
  * @param  None
  * @retval None
  */
void initInterrupt()
{
	 PCMSK2 |= (1 << PCINT23)|(1 << PCINT22)|(1 << PCINT21)|(1 << PCINT20)|(1 << PCINT19)|(1 << PCINT18);
	 PCMSK0 |= (1 << PCINT5)|(1 << PCINT4)|(1 << PCINT3)|(1 << PCINT2)|(1 << PCINT1)|(1 << PCINT0);
	 PCICR |=  (1 << PCIE0)|(1 << PCIE2);
	 sei();
}



//--------------------------------------------------------------------------------------------------------------//
// 														Timers													//
//--------------------------------------------------------------------------------------------------------------//

/**
  * @brief  Function to setup timers
  * @param  None
  * @retval None
  */
void initTimer()
{
	TCCR0A = (1 << WGM01);					//set CTC bit
	OCR0A = 16;								//ticks for 0.001s
	TIMSK0 = (1 << OCIE0A);					//interrupt enable
	sei();
	TCCR0B = (1 << CS00) | (1 << CS02) ;	//1024 prescaller ;
}
