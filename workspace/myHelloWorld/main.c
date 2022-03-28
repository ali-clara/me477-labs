
#include <stdio.h>
#include "MyRio.h"
#include "me477.h"

int main(int argc, char **argv)
{
	NiFpga_Status status;

    status = MyRio_Open();		    /*Open the myRIO NiFpga Session.*/
    if (MyRio_IsNotSuccess(status)) return status;

    printf("ME 477 Hello World!!\n");	 		// Print to Console
    printf_lcd("\fME 477 Hello World!\n");		// Print to LCD screen

    status = MyRio_Close();	 /*Close the myRIO NiFpga Session. */

    return status;
}


