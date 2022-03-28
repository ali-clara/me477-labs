/* Lab #2 - Ali Jones */

/* includes */
#include "stdio.h"
#include "MyRio.h"
#include "me477.h"
#include <string.h>
#include <stdarg.h>

/* prototypes */
double	double_in(char *prompt);
char *fgets_keypad(char *buf, int buflen);
int printf_lcd(char *format, ...);
int putchar_lcd(int c);
int getchar_keypad(void);
char getkey(void);


/* definitions */
#define buf_len 60

int main(int argc, char **argv){
	char *prompt = "Enter First Value: ";
	char *prompt2 = "Enter Second Value: ";
	char static test[buf_len];
	char static test2[buf_len];

	NiFpga_Status status;

    status = MyRio_Open();		    /*Open the myRIO NiFpga Session.*/
    if (MyRio_IsNotSuccess(status)) return status;

    // call fgets_keypad twice to test the getchar_keypad input function, print the results

    printf_lcd("\f%s\n", prompt);
    fgets_keypad(test, buf_len);
    printf("Value 1: %s\n", test);

    printf_lcd("\f%s", prompt2);
    fgets_keypad(test2, buf_len);
    printf("Value 2: %s\n", test2);

	status = MyRio_Close();	 /*Close the myRIO NiFpga Session. */
	return status;
}

int getchar_keypad(){
/*
 * getchar_keypad
 * Purpose: gets key from keypad
 * Information transfer: receives prompts from 'main, sends prompts and outputs to 'printf_lcd' and
 * 		'printf_lcd' to print, sends user input back to 'main'
 * Input: prompt (character string from 'main')
 */
	static char buffer[buf_len];	// will store characters
	static char *point;				// pointer to buffer
	static int n = 0;		// number of characters in buffer
	static char ch;			// character to store keys

	if (n==0){		// if there are no keys in the buffer, get keys
		point = buffer;				// pointer set to start of buffer
		while( (ch=getkey()) != ENT ){		// loop through keys until "enter" is pressed
			if (n < buf_len){
				*point++ = ch;		// character put in buffer, pointer incremented for next character
				n++;
				if (ch == DEL){		// if the delete key is pressed
					n--;				// remove the 'delete' key from buffer counter
					point--;			// remove the 'delete' key from the pointer count
					if (n != 0){	// if there are characters to delete
						n--;			// remove the key to be deleted from the buffer counter
						point--;		// remove the key to be deleted from the pointer count
						putchar_lcd('\b');
						putchar_lcd(' ');
						putchar_lcd('\b');
					}
				}
				else{				// if the delete key is not pressed
					putchar_lcd(ch);	// display character on lcd
				}
			}
		}
		n++;
		point = buffer;		// pointer reset to start of buffer
	}
	if (n > 1){		// if there is >1 key in the buffer, return it
		n--;
		sscanf(buffer, "%c", buffer);		// convert the chars in buffer from ascii to decimal
		return *point++;
	}
	else{			// if there is 1 key in the buffer, return end of string
		n--;
		point = buffer;
		return EOF;
	}
}

double double_in(char *prompt){
/*
 * double_in
 * Purpose: identifies prompt to print on lcd display and accepts user input. Ensures user input is valid
 * Information transfer: receives prompts from 'main, sends prompts and outputs to 'printf_lcd' and
 * 		'printf_lcd' to print, sends user input back to 'main'
 * Input: prompt (character string from 'main')
 */

	// variables
	char str[10];
	double read_val;
	char invalid_str[] = "[]";
	char *up_down_check;
	char doub_decimal[] = "..";
	char *double_decimal_check;
	int string_check = 0;		// bad string = 0, good string = 1
	char *error_message = "Bad Key. Try Again";

	// display prompt on lcd and take input
	printf_lcd("\f%s\n", prompt);
	fgets_keypad(str, 80);

	// set up variables to check for bad strings
	up_down_check = strpbrk(str, invalid_str);
	double_decimal_check = strstr(str, doub_decimal);

	// check for bad strings
	while (string_check == 0){
		if (str == NULL){
			printf_lcd("\f%s\n", prompt);
			printf_lcd("Short. Try again: ");
			fgets_keypad(str, 10);
		}
		if (up_down_check){		// if variable not null
			printf_lcd("\f%s\n", prompt);
			printf_lcd("\f%s\n", error_message);
			string_check = 0;		// indicates bad string

			fgets_keypad(str, 10);

			// recheck input string validity
			up_down_check = strpbrk(str, invalid_str);
			double_decimal_check = strstr(str, doub_decimal);
		}
		if (double_decimal_check){		// if variable not null
			printf_lcd("\f%s\n", prompt);
			printf_lcd("\f%s\n", error_message);
			string_check = 0;		// indicates bad string

			fgets_keypad(str, 10);

			// recheck input string validity
			double_decimal_check = strstr(str, doub_decimal);
			up_down_check = strpbrk(str, invalid_str);
		}
		else{
			printf_lcd("\fValue Accepted\n");
			printf_lcd("%s", str);
			string_check = 1;		// indicates good string
		}
	}

	sscanf(str, "%lf", &read_val);		// convert from string to float
	return read_val;
}

int printf_lcd(char *format, ...){
/*
 * printf_lcd
 * Purpose: Prints to lcd screen
 * Information transfer: receives string  to print from double_in
 * Input: format
 */

	// variables
	int val;
	char str[80];
	char *point;

	point = str;

	// writing the data to a string
	va_list args;
	va_start(args, format);
		val = vsprintf(str, format, args);
	va_end(args);

	// loops across the length of 'str' to print every character
	int i = 0;
	while (i < strlen(str)){
		putchar_lcd(*point);
		point++;
		++i;
	}

	return *point;
}
