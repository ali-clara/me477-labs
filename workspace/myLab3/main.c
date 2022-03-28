/* Lab 3 - Ali Jones */

/* includes */
#include "stdio.h"
#include "MyRio.h"
#include "me477.h"
#include "UART.h"
#include "DIO.h"
#include <time.h>
#include <stdarg.h>
#include <string.h>

/* prototypes */
int putchar_lcd(int value);
char getkey(void);
void wait(void);
NiFpga_Bool Dio_ReadBit(MyRio_Dio* channel);
void Dio_WriteBit(MyRio_Dio* channel, NiFpga_Bool value);

/* definitions */
#define write_len 5		// length of writeS array (see putchar_lcd)

/*--------------------------------------------------
 Function main
 	 Purpose:		- initializes the myRIO session,
 	 	 	 	 	- calls putchar_lcd() and getkey() to
 	 	 	 	 		communicate with LCD and keypad
 	 Parameters:	argc, **argv
 	 Returns: 		NiFPga status
 *-------------------------------------------------*/
int main(int argc, char **argv){

	// variables
	NiFpga_Status status;
    int key;
    char str[50];
    char *prompt1 = "Enter value: ";
    char *prompt2 = "Enter values: ";

    status = MyRio_Open();		    /*Open the myRIO NiFpga Session.*/
    if (MyRio_IsNotSuccess(status)) return status;

    /* testing the functions */

    // individual calls to putchar_lcd() and getkey()
    putchar_lcd('\f');
    putchar_lcd(400);	// out of range, sends nothing
    putchar_lcd(':');	// prints a smiley face
    putchar_lcd(')');
    putchar_lcd('\n');
    printf_lcd("%s", prompt1);
    key = getkey();		// grabs a single key
    printf("getkey value: %c\n", key);

    // collect a string with fgets_keypad()
    printf_lcd("\f%s", prompt2);
    fgets_keypad(str, 10);
    printf_lcd("\f");
    printf("fgets_keypad string: %s\n", str);

    // print a string with printf_lcd()
    printf_lcd("\fHello World \n\b! \nI'm writing to the \nLCD \v");

	status = MyRio_Close();	 /*Close the myRIO NiFpga Session. */
	return status;
}

/*--------------------------------------------------
 Function putchar_lcd
 	 Purpose:		- writes to LCD screen with UART
 	 	 	 	 	- interprets escape sequences
 	 Parameters:	value (user input)
 	 Returns: 		value (user input)
 *-------------------------------------------------*/
int putchar_lcd(value){

	// variables
	static int flag = 1;		// flag for initializing UART only once
	static MyRio_Uart uart;		// UART structure (defined in UART.h)
	NiFpga_Status status;		// UART success status
	uint8_t writeS[write_len];	// string to write
	size_t nData;				// number of data codes in writeS
	int i;						// loop increment

	// initialize UART the first time the function is called
	if (flag == 1){
		uart.name = "ASRL2::INSTR";
		uart.defaultRM = 0;
		uart.session = 0;
		status = Uart_Open(&uart,
							19200,
							8,
							Uart_StopBits1_0,
							Uart_ParityNone);
		flag = 0;
	}
	if (status < VI_SUCCESS)		// exit if the call is unsuccessful
		return EOF;
	if (value > 255)				// exit if the input value is out of range
		return EOF;
	else if (value == '\f'){		// process "backlight & clear lcd" escape sequence
		writeS[0] = 12;
		writeS[1] = 17;
		nData = 2;
	}
	else if (value == '\b'){		// process "cursor left, 1 space" escape sequence
		writeS[0] = 8;
		nData = 1;
	}
	else if (value == '\v'){		// process "cursor to start line-0" escape sequence
		writeS[0] = 128;
		nData = 1;
	}
	else if (value == '\n'){		// process "cursor to start next line" escape sequence
		writeS[0] = 13;
		nData = 1;
	}
	else{							// process valid character input
		for(i=0; i<write_len; i++)
			writeS[i] = value;
		nData = 1;
	}

	status = Uart_Write(&uart, writeS, nData);		// write the processed input
	return value;
}

/*--------------------------------------------------
 Function getkey
 	 Purpose:		- detects keypad input by controlling
 	  	  	  	  	  	  column and row voltages with
 	  	  	  	  	  	  Dio_ReadBit and Dio_WriteBit
 	 Parameters:	none
 	 Returns: 		key (keypad character input)
 *-------------------------------------------------*/
char getkey(){

	// variables
	MyRio_Dio Ch[8];	// array of MyRio_Dio structures for initialization
	NiFpga_Bool bit;	// row and column status (True = high, False = low)
	int i;				// initialization loop increment
	int col;			// column loop increment
	int row;			// row loop increment
	int high_z;			// set-columns-high-z loop increment
	char key;			// stores keypress value
	int bit_check = 1;	// detects if bit has been pressed (1 = key up, 0 = key pressed)
	char table[4][4] =	{ {'1', '2', '3', UP},
						  {'4', '5', '6', DN},
						  {'7', '8', '9', ENT},
						  {'0', '.', '-', DEL}  };	// table representing keypad values

	// initialize channels
	for (i=0; i<8; i++){
		Ch[i].dir = DIOB_70DIR;
		Ch[i].out = DIOB_70OUT;
		Ch[i].in = DIOB_70IN;
		Ch[i].bit = i;
	}

	while (bit_check==1){					// while a low bit has not been detected
		for (col=0; col<4; col++){				// for each column
			for (high_z=0; high_z<4; high_z++){
				bit = Dio_ReadBit(&Ch[high_z]);		// set all columns to High-Z
			}
			Dio_WriteBit(&Ch[col], NiFpga_False);	// write ith column low
			for (row=0; row<4; row++){				// for each row
				bit = Dio_ReadBit(&Ch[row+4]);			// read jth bit
				if (bit == NiFpga_False){				// if bit is low
					bit_check = 0;							// update bit_check
					break;									// exit row loop
				}
			}
			if (bit == NiFpga_False){				// it bit is low
				break;									// exit column loop
			}
			wait();									// wait for xxx ms
		}
	}

	key = table[row][col];			// convert row and column values to correct key
	while (bit_check == 0){			// while a low bit has been detected
		bit = Dio_ReadBit(&Ch[row+4]);	// read the detected row
		if (bit == NiFpga_True){		// wait for the key to go high
			bit_check = 1;					// update bit_check
			return key;
			break;
		}
	}
}

/*--------------------------------------------------
 Function wait
 	 Purpose:		waits for xxx ms
 	 Parameters:	none
 	 Returns: 		none
 *-------------------------------------------------*/
void wait(){
	uint32_t i;

	i = 417000;
	while(i>0){
		i--;
	}
	return;
}

