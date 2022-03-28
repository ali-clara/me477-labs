/* Lab #6 - Ali Jones */

/* includes */
#include "stdio.h"
#include "MyRio.h"
#include "me477.h"
#include <string.h>
#include "emulate.h"
#include "TimerIRQ.h"
#include <pthread.h>
#include "matlabfiles.h"

/* definitions */
#define IMAX 500	// output buffer max points
struct biquad{
	double b0; double b1; double b2; 	// numerator
	double a0; double a1; double a2;	// denominator
	double x0; double x1; double x2;	// input
	double y1; double y2;				// output
};
#define SATURATE(x,lo,hi)((x)<(lo)?(lo):(x)>(hi)?(hi):(x))

/* prototypes */
void* Timer_Irq_Thread(void* resource);
int32_t Irq_RegisterTimerIrq( MyRio_IrqTimer* irqChannel,
							  NiFpga_IrqContext* irqContext,
							  uint32_t timeout);
double cascade(	double xin,			// input
				struct biquad *fa,	// biquad array
				int ns, 			// number of biquad sections
				double ymin,		// min output (saturation lower limit)
				double ymax);		// max output (saturation upper limit)

/* global vars */
typedef struct {
	NiFpga_IrqContext irqContext;	// IRQ context received
	NiFpga_Bool irqThreadRdy;		// IRQ thread ready flag
} ThreadResource;

static double input_buff[IMAX], output_buff[IMAX];
static double *in_bp = input_buff, *out_bp = output_buff;

uint32_t timeoutValue;

int main(int argc, char **argv)
{
	NiFpga_Status status;
	int32_t irq_status;
	MyRio_IrqTimer irqTimer0;
	ThreadResource irqThread0;
	pthread_t thread;
	MATFILE *mf;
	int err;
	int key;

    status = MyRio_Open();		    /*Open the myRIO NiFpga Session.*/
    if (MyRio_IsNotSuccess(status)) return status;

    // Registers corresponding to the IRQ channel
    irqTimer0.timerWrite = IRQTIMERWRITE;
    irqTimer0.timerSet = IRQTIMERSETTIME;
    timeoutValue = 500;

    irq_status = Irq_RegisterTimerIrq( &irqTimer0,
    							   	   &irqThread0.irqContext,
    							   	   timeoutValue);

    // Set the indicator to allow the new thread
    irqThread0.irqThreadRdy = NiFpga_True;

    // Create new thread to catch the IRQ
    irq_status = pthread_create( &thread,
    							 NULL,
    							 Timer_Irq_Thread,
    							 &irqThread0);


    // clear LCD screen
    printf_lcd("\f");

    // enter loop
    while ( (key=getkey()) != DEL);

    // write to matlab file
    mf = openmatfile("Lab6.mat", &err);
    		if(!mf) printf("Can’t open mat file %d\n", err);

    matfile_addstring(mf, "myName", "Ali Jones");
    matfile_addmatrix(mf, "input", input_buff, IMAX, 1, 0);
    matfile_addmatrix(mf, "output", output_buff, IMAX, 1, 0);
    matfile_close(mf);

    // terminate thread
    irqThread0.irqThreadRdy = NiFpga_False;
    irq_status = pthread_join(thread, NULL);

    // unregister timer interrupt
    irq_status = Irq_UnregisterTimerIrq( &irqTimer0,
    									 irqThread0.irqContext);

	status = MyRio_Close();	 /*Close the myRIO NiFpga Session. */
	return status;
}

void* Timer_Irq_Thread(void* resource){

	MyRio_Aio CI0, CO0;
	double input, output;
	extern NiFpga_Session myrio_session;
	double ymin = -10.000, ymax = 10;

	// set up biquad
	int myFilter_ns = 2;			// Number of sections
	timeoutValue = 500;	// T - us. f_s = 2000 Hz
	static struct biquad myFilter[] = {
				{1.0000e+00,  9.9999e-01, 0.0000e+00,
				 1.0000e+00, -8.8177e-01, 0.0000e+00, 0, 0, 0, 0, 0},
				{2.1878e-04,  4.3755e-04, 2.1878e-04,
				 1.0000e+00, -1.8674e+00, 8.8220e-01, 0, 0, 0, 0, 0}
	};

	ThreadResource* threadResource = (ThreadResource*) resource;

	AIO_initialize(&CI0, &CO0);
	Aio_Write(&CI0, 0);

	while (threadResource->irqThreadRdy == NiFpga_True){
		/* pause the loop while waiting for interrupt */
		uint32_t irqAssert = 0;
		Irq_Wait( threadResource->irqContext,
				  TIMERIRQNO,
				  &irqAssert,
				  (NiFpga_Bool*) &(threadResource->irqThreadRdy));
		/* schedule the next interrupt */
		NiFpga_WriteU32( myrio_session,
						 IRQTIMERWRITE,
						 timeoutValue);
		NiFpga_WriteBool( myrio_session,
						  IRQTIMERSETTIME,
						  NiFpga_True);

		if(irqAssert){
			/* service interrupt */

			// read analog input
			input = Aio_Read(&CI0);
			if (in_bp < input_buff+IMAX)
				*in_bp++ = input;

			// call cascade
//			output = input;
			output = cascade(input, myFilter, myFilter_ns, ymin, ymax);

			// send to analog output
			Aio_Write(&CO0, output);
			if (out_bp < output_buff+IMAX)
				*out_bp++ = output;

			/* acknowledge interrupt */
			Irq_Acknowledge(irqAssert);
		}
	}

	pthread_exit(NULL);
	return NULL;

}

double cascade(double xin, struct biquad *fa, int ns, double ymin, double ymax){
	int i;
	struct biquad *f;
	f = fa;
	double y0;

	for (i = 1; i <= ns; i++){
		if(i == 1){
			f->x0 = xin;
			y0 = xin;
		}
		else{
			f->x0 = y0;
		}
		y0 = (f->b0*f->x0 + f->b1*f->x1 + f->b2*f->x2 - f->a1*f->y1 - f->a2*f->y2)/f->a0;
		// update xi and yi for the next step through the loop
		f->x2 = f->x1;
		f->x1 = f->x0;
		f->y2 = f->y1;
		f->y1 = y0;
		// advance to next biquad in array
		f++;
	}

	// saturate y0 for D/A converter
	y0 = SATURATE(y0, ymin, ymax);
	return y0;

}
