/* Lab #4 - Ali Jones */

/* includes */
#include <stdio.h>
#include "Encoder.h"
#include "MyRio.h"
#include "DIO.h"
#include "me477.h"
#include <unistd.h>
#include <string.h>
#include "matlabfiles.h"
//#include "emulate.h"

/* prototypes */
double	double_in(char *prompt);
NiFpga_Status EncoderC_initialize(NiFpga_Session myrio_session, MyRio_Encoder *channel);
uint32_t Encoder_Counter(MyRio_Encoder* channel);
void stateHigh(void);
void stateLow(void);
void stateSpeed(void);
void stateStop(void);
void initializeSM(void);
void wait(void);
double vel(void);

/* definitions */
#define wait_time 0.005		// 5ms
#define BDI 1/2000		// encoder state change = 1/2000 revolutions
#define IMAX 2400		// speed buffer max points

/* global declarations */
NiFpga_Session myrio_session;
MyRio_Encoder encC0;
typedef enum {LOW=0, HIGH, SPEED, STOP, EXIT} State_Type;
static void (*state_table[])(void)={stateLow, stateHigh, stateSpeed, stateStop};
static State_Type curr_state;
static int Clock, M, N;
MyRio_Dio Ch0, Ch6, Ch7;
static double buffer[IMAX];		// buffer to store speed values
static double *bp = buffer;		// buffer pointer


/*--------------------------------------------------
 Function main
 	 Purpose:		initializes MyRio session,
 	 	 	 	 	initializes state machine,
 	 	 	 	 	loops through state transitions
 	 Parameters:	none
 	 Returns: 		none
 *-------------------------------------------------*/
int main(int argc, char **argv)
{
	NiFpga_Status status;
	char *prompt_N = "Enter N";
	char *prompt_M = "Enter M";

    status = MyRio_Open();		    /*Open the myRIO NiFpga Session.*/
    if (MyRio_IsNotSuccess(status)) return status;

    // initialize MyRio channels and state machine
    initializeSM();

    N = double_in(prompt_N);		// number of wait intervals in each BTI
    M = double_in(prompt_M);		// number of intervals that the motor signal is "on"

    // main state transition loop
    while(1){
    	state_table[curr_state]();	// pointer to each state
    	wait();
    	Clock++;
    	if(curr_state == EXIT){
    		break;
    	}
    }

    status = MyRio_Close();	 /*Close the myRIO NiFpga Session. */
	return status;
}

/*--------------------------------------------------
 Function wait
 	 Purpose:		stops the motor every M cycles
 	 Parameters:	none
 	 Returns: 		none
 *-------------------------------------------------*/
void stateLow(void){
	if(Clock == M){ 	// if M cycles have been reached
		curr_state = HIGH; 		// change to stateHigh
		Dio_WriteBit(&Ch0, NiFpga_True); // run=1 (stop motor)
	}
}

/*--------------------------------------------------
 Function stateHigh
 	 Purpose:		resets clock and starts motor every N cycles,
 	 	 	 	 	processes button presses
 	 Parameters:	none
 	 Returns: 		none
 *-------------------------------------------------*/
void stateHigh(void){
	if(Clock == N){ 	// if N cycles have been reached
		Clock = 0;			// reset clock
		Dio_WriteBit(&Ch0, NiFpga_False); // run=0 (start motor)
		if(Dio_ReadBit(&Ch7) == NiFpga_False)
			curr_state = SPEED;		// enter 'print speed' mode when channel 7 is pressed
		else if(Dio_ReadBit(&Ch6) == NiFpga_False)
			curr_state = STOP;		// stop the motor when channel 6 is pressed
		else
			curr_state = LOW;	// change to stateLow
	}
}

/*--------------------------------------------------
 Function stateSpeed
 	 Purpose:		converts speed from BDI/BTI to rpm,
 	 	 	 	 	prints speed to LCD screen
 	 Parameters:	none
 	 Returns: 		none
 *-------------------------------------------------*/
void stateSpeed(void){
	double curr_speed, rpm;

	curr_speed = vel();		// call vel() for current speed
	rpm = curr_speed*60*BDI/N/wait_time;	// convert from BDI/BTI to rpm
	printf_lcd("\fspeed %g rpm", rpm);
	curr_state = LOW;		// change state to stateLow

	if (bp < buffer+IMAX)
		*bp++ = rpm;	// add speed to rpm buffer
}

/*--------------------------------------------------
 Function stateStop
 	 Purpose:		stops the motor,
 	 	 	 	 	publishes data to matlab script
 	 Parameters:	none
 	 Returns: 		none
 *-------------------------------------------------*/
void stateStop(void){

	Dio_WriteBit(&Ch0, NiFpga_True); // run=1 (stop the motor)
	printf_lcd("\fStopping");
	curr_state = EXIT;		// exit the loop in 'main()'

	// save speed data to matlab file
	MATFILE *mf;
	int err;
	double Npar = (double)N;
	double Mpar = (double)M;

	mf = openmatfile("Lab4.mat", &err);
		if(!mf) printf("Can’t open mat file %d\n", err);

	matfile_addstring(mf, "myName", "Ali Jones");
	matfile_addmatrix(mf, "N", &Npar, 1, 1, 0);
	matfile_addmatrix(mf, "M", &Mpar, 1, 1, 0);
	matfile_addmatrix(mf, "vel", buffer, IMAX, 1, 0);
	matfile_close(mf);
}

/*--------------------------------------------------
 Function vel
 	 Purpose:		calculates velocity via encoder
 	 Parameters:	none
 	 Returns: 		motor angular velocity in BDI/BTI
 *-------------------------------------------------*/
double vel(void){
	double curr_speed;
	static int c_n, c_n1;	// current encoder count and previous encoder count
	static int initial_call = 0;

	c_n = Encoder_Counter(&encC0);	// get current count from encoder

	if (initial_call == 0){		// if it's the first call,
		c_n1 = c_n;					// set previous count = current count
		initial_call++;
	}

	curr_speed = c_n - c_n1;	// calculate speed
	c_n1 = c_n;					// set previous count = current count

	return curr_speed;
}

/*-------------------------------------------------------------
 Function initializeSM
 	 Purpose:		initialize the MyRio channels and encoder,
 	 	 	 	 	stop the motor,
 	 	 	 	  	set the correct state,
 	 	 	 	 	reset the clock
 	 Parameters:	none
 	 Returns: 		none
 *------------------------------------------------------------*/
void initializeSM(void){

	/* initialize channels 0, 6, 7*/
	Ch0.dir = DIOA_70DIR;
	Ch0.out = DIOA_70OUT;
	Ch0.in = DIOA_70IN;
	Ch0.bit = 0;

	Ch6.dir = DIOA_70DIR;
	Ch6.out = DIOA_70OUT;
	Ch6.in = DIOA_70IN;
	Ch6.bit = 6;

	Ch7.dir = DIOA_70DIR;
	Ch7.out = DIOA_70OUT;
	Ch7.in = DIOA_70IN;
	Ch7.bit = 7;

	/* initialize encoder interface*/
	EncoderC_initialize(myrio_session, &encC0);

	/* stop motor (run=1)*/
	Dio_WriteBit(&Ch0, NiFpga_True);

	/* set initial state to low*/
	curr_state = LOW;

	/* set clock to zero*/
	Clock = 0;
}

/*--------------------------------------------------
 Function wait
 	 Purpose:		waits for xx ms
 	 Parameters:	none
 	 Returns: 		none
 *-------------------------------------------------*/
void wait(void){
	uint32_t i;

	i = 417000;
	while(i>0){
		i--;
	}

	return;
}
