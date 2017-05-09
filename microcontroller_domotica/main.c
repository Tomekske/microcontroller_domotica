/*
 * microcontroller_domotica.c
 *
 * Created: 09-May-17 9:19:13 AM
 * Author : Tomek
 */ 


#include <avr/io.h>
#include "defines.h" //uC settings,clock freq, ...
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "168p_StdLib.h" //uC general purpose library
#include "project_defines.h" //defines,constants, flags and function prototypes using in this project



int main(void)
{
    //Initialize peripheral
	initSerial(9600);
	initInterrupt();
	initADC();
	initTimer();

	//Shiftregister pins
	s1.DS_pin = DS;
	s1.SHCP_pin = SHCP;
	s1.STCP_pin = STCP;

	//set pins as Input or output
	pinMode(FAN,OUTPUT);
	pinMode(BUZZER,OUTPUT);
	pinMode(PIR,INPUT);
	pinMode(DS,OUTPUT);
	pinMode(STCP,OUTPUT);
	pinMode(SHCP,OUTPUT);

	//make sure shift register pins are LOW on startup
	initShiftregister(&s1,1);
	initShiftregister(&s1,2);


    while (1) 
    {
		filterReceived(receivedStr,filteredStr); //filter incoming strings
		
		//copy filtered string to different variables
 		strcpy(component,filteredStr[0]);  //Select which component you want to adress
 		state = atoi(filteredStr[1]);      //Select logical state of component
 		position = atoi(filteredStr[2]);   //Select which pin position you want to adress
		
	    checkComponent();  //Check which component you currenlty want to control
		alarm(); //holds alarm logic
    }
}

/**
  * @brief  External interupt vector 0 => pins(D8 - D13) on  port B
  * @param  None
  * @retval None
  */
ISR(PCINT0_vect)
{
	if(digitalRead(PIR))
		enablePIR_flag = 1;
	else
		enablePIR_flag = 0;
}

/**
  * @brief  External interupt vector 2 => pins(D2 - D7) on  port D
  * @param  None
  * @retval None
  */
ISR(PCINT2_vect)
{
	//Empty vector, because all external interrupt ports (port D and port B) are enabled
}

/**
  * @brief  UART RX vector which fetches receiving bytes
  * @param  None
  * @retval None
  */
ISR(USART_RX_vect)
{
	serialReadString(receivedStr);
}

/**
  * @brief  Timer 0 gives an interrupt every 0.001s
  * @param  None
  * @retval None
  */
ISR(TIMER0_COMPA_vect)
{
	sendTemp ++; //counter

	//sends on startup a string holding the current temperature over UART
	if((sendTemp > 50) && firstTemp == 0)
	{
		tempSensor = readADC(0); //reads ADC value
		temp = mVoltADC(voltADC(tempSensor)) / 10; //convert mV to *C
		sprintf(sendTempStr,"temp_%d\r",temp); //format string which has to be send
		serialSendString(sendTempStr); //send indecator string with current temperature
		firstTemp = TRUE; 
		sendTemp = 0; //reset counter
	}

    //sends every 5 second the temperature over UART
	if(sendTemp > 5000)
	{
		tempSensor = readADC(0);
		temp = mVoltADC(voltADC(tempSensor)) / 10;
		sprintf(sendTempStr,"temp_%d\r",temp);
		serialSendString(sendTempStr);
		sendTemp = 0;
	}
}

/**
  * @brief  Filter string which has been received
  * @param  unfiltered: pass unfiltered string received by UART
  * @param  filtered: array of strings chopped into words
  * @retval returns filtered
  */
void filterReceived(char *unfiltered,char *filtered[])
{
	char filter[3] = "_", *tok; //filter string on '_'

	tok = strtok(unfiltered, filter); //copy string without '_' in tok

	//while tok != null byte
	while (tok != NULL)
	{
		*filtered = tok; //copy token into filtered at the current index
		tok = strtok(NULL, filter); //continue filtering on same string, NULL indicates operation on the same string
		filtered++; //add up pointer
	}
}

/**
  * @brief  check which component has to interact with the environment and how to interact with the environment
  * @param  component: received state of component
  * @param  position:  received state of position
  * @param  state:     received which component to interact with
  * @retval None
  */
void checkComponent()
{
	if(strcmp(component,"led") == 0)
	{
		registerWrite(&s1,LIGHTS,position,state);
	}

	else if(strcmp(component,"fan") == 0)
	{
		digitalWrite(position,state);
	}

	else if(strcmp(component,"alarm") == 0)
	{
		//if alarm has been enabled over UART toggle bool
		if(state == 1)
		{
			enableAlarmUART_flag = TRUE;
		}

		//if alarm has been disabled over UART we make sure all need bools are toggled to their default state
		else
		{
			enableAlarmUART_flag = FALSE;
			alarmFlag = FALSE;
			sendAlarmTextOnce = FALSE;

			//Make sure leds and buzzer are LOW
			for(int i = 0;i < PORTCOUNT;i++)
				registerTogglePin(&s1,ALARM,i,0); //aan
			digitalWrite(BUZZER,LOW);
		}
	}
}

/**
  * @brief  Function which hold the logic of the alarm
  * @param  component: received state of component
  * @param  position:  received state of position
  * @param  state:     received which component to interact with
  * @retval None
  */
void alarm()
{
	//if alarm got triggerd and the alarm has been anabled in the QT app we toggle a bool. And we want to make sure the blinky funtions stays enabled till the alarm gets disabled over UART
	if(enablePIR_flag == 1 && enableAlarmUART_flag == 1)
	{
		alarmFlag = 1;

		//Sends triggerd string only once when the alarm turns on
		if(sendAlarmTextOnce == FALSE)
		{
			serialSendString("Alarm_Triggerd\r");
			sendAlarmTextOnce = TRUE;
		}
	}

	//Make sure the blinky function wont stop until the alarm is disabled
	if(alarmFlag == 1 && enableAlarmUART_flag == 1)
		blinky();
}

/**
  * @brief  Function which displays the animation when PIR gets triggerd
  * @param  component: received state of component
  * @param  position:  received state of position
  * @param  state:     received which component to interact with
  * @retval None
  */
void blinky()
{
	int stopBlinky = FALSE;

	//toggle all leds 
	for(int i = 0;i < PORTCOUNT;i++)
		registerTogglePin(&s1,ALARM,i,1); 
	digitalTogglePin(BUZZER);

	//wait an amount of ms to toggle leds, this loops gives us the oppertunity to not block the controller for a certain amount of time
	//the loop is devided in an amount of pieces with a total delay of 120ms (12x 10ms)
	for(int j = 0;j < 12;j++)
	{
		//We check if there is a new received string
		filterReceived(receivedStr,filteredStr);
 		strcpy(component,filteredStr[0]);  //Select which component you want to adress
 		state = atoi(filteredStr[1]);      //Select logical state of component
 		position = atoi(filteredStr[2]);   //Select which pin position you want to adress
 		checkComponent(component,position,state);  //Check which component you currently want to control

		//if the received string got the key word alarm and with the command to disable it then we toggle a flag
		if(enableAlarmUART_flag == FALSE && (strcmp(component,"alarm") == 0) && state == FALSE)
			stopBlinky = TRUE;

		//break the loop
		if(stopBlinky == TRUE)
			break;

		_delay_ms(10);
	}
	//digitalTogglePin(BUZZER);
}