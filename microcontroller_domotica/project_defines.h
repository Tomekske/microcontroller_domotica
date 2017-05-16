#ifndef PROJECT_DEFINES
	#define PROJECT_DEFINES

	#define  BUZZER  10
	#define  PIR     11
	#define  FAN     12

	#define  DS      2 	//serial data input
	#define  SHCP    3  //shiftregister clock pin
	#define  STCP    4  //shiftregister latch pin

	#define ALARM    1 //shiftregister alarm led position
	#define LIGHTS   2 //shiftregister room led position

	ShiftRegister s1;
	
	//UART
	char *filteredStr[64];
	char receivedStr[64];
	volatile uint8_t state = 0;        //received state of component
	volatile uint8_t position = 0;     //received position of component
	char component[16] = "";           //received which component to interact with	

	//Temperarture
	char sendTempStr[64];
	int sendTemp = 0;
	int temp = 0;
	int disableAlarmFlag = 0;
	int firstTemp = FALSE;
	uint16_t tempSensor = 0;
	int adcFlag = FALSE;

	//Alarm
	int enablePIR_flag = FALSE;
	int enableAlarmUART_flag = FALSE;
	int disbaleAlarmUART_flag = FALSE;
	int sendAlarmTextOnce = FALSE;
	int alarmFlag = 0;

	//Function prototypes
	void filterReceived(char *unfiltered,char *filtered[]);
	void checkComponent();
	void alarm();
	void temperature();
	void blinky();
#endif