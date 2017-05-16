#ifndef CONSTANTS
	#define CONSTANTS
	#define TRUE       1
	#define FALSE      0

	#define HIGH       1
	#define LOW        0

	#define OUTPUT     1
	#define INPUT      0

	#define PORTCOUNT  8 // number of GPIO pins
	#define F_CPU      16000000 // clock frequentie
	#define AMOUNT_OF_SHIFTREGISTERS   2

	#define BAUD_PRESCALLER(baudRate) (((F_CPU / (baudRate * 16UL))) - 1)  // pagina 173 tabel 20-1
	#define RX_BUFFER_SIZE 128
	#define BRC(baudRate) ((F_CPU/16/baudRate) - 1)

    #define UREF        4.7
    #define RESOLUTION  1024.0

	typedef struct{
		uint8_t DS_pin;
		uint8_t STCP_pin;
		uint8_t SHCP_pin;
		int reg[PORTCOUNT*AMOUNT_OF_SHIFTREGISTERS];
	}ShiftRegister;
#endif