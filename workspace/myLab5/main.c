/* Lab #5 - Ali Jones */

/* includes */
#include "stdio.h"
#include "MyRio.h"
#include "me477.h"
#include <string.h>
#include "DIIRQ.h"
#include <pthread.h>

/* prototypes */
void wait(void);
void* DI_Irq_Thread(void* resource);

/* global vars */
typedef struct {
	NiFpga_IrqContext irqContext;	// IRQ context received
	NiFpga_Bool irqThreadRdy;		// IRQ thread ready flag
	uint8_t irqNumber;				// IRQ number value
} ThreadResource;

/*--------------------------------------------------
 Function main
 	 Purpose:		initializes MyRio session,
 	 	 	 	 	registers and initializes DiIRQ thread,
 	 	 	 	 	counts to 60 in seconds,
 	 	 	 	 	unregisters thread when done
 	 Parameters:	none
 	 Returns: 		none
 *-------------------------------------------------*/
int main(int argc, char **argv){

    int i;
    int j;
    int32_t thread_status;
    ThreadResource irqThread0;
    pthread_t thread;
    MyRio_IrqDi irqDI0;
	NiFpga_Status status;

    status = MyRio_Open();            /*Open the myRIO NiFpga Session.*/
    if (MyRio_IsNotSuccess(status)) return status;

	// configure DI IRQ
	const uint8_t IrqNumber = 2;
	const uint32_t Count = 1;
	const Irq_Dio_Type TriggerType = Irq_Dio_FallingEdge;

	// specify IRQ channel settings
	irqDI0.dioCount = IRQDIO_A_0CNT;
	irqDI0.dioIrqNumber = IRQDIO_A_0NO;
	irqDI0.dioIrqEnable = IRQDIO_A_70ENA;
	irqDI0.dioIrqRisingEdge = IRQDIO_A_70RISE;
	irqDI0.dioIrqFallingEdge = IRQDIO_A_70FALL;
	irqDI0.dioChannel = Irq_Dio_A0;

	// initiate IRQ number resource for new thread
	irqThread0.irqNumber = IrqNumber;

	// register DIO IRQ, terminate if unsuccessful. Returns 0 for success
	thread_status = Irq_RegisterDiIrq( 	&irqDI0,					// IRQ channel structure
										&(irqThread0.irqContext),	// identifies interrupt
										IrqNumber,
										Count,
										TriggerType);

	// set indicator to allow new thread
	irqThread0.irqThreadRdy = NiFpga_True;

	// create new thread to catch the irq
	thread_status = pthread_create(	&thread,
									NULL,
									DI_Irq_Thread,
									&irqThread0);

    // count to 60 seconds
    for (i=1; i<61; i++){
    	for (j=0; j<200; j++){
    		wait();
    	}
    	printf_lcd("\f%d", i);
    }

    // signal to terminate the thread
    irqThread0.irqThreadRdy = NiFpga_False;
    thread_status = pthread_join(thread, NULL);

    // unregister the interrupt
    thread_status = Irq_UnregisterDiIrq( &irqDI0,
    									 &(irqThread0.irqContext),
    									 IrqNumber);

    status = MyRio_Close();     /*Close the myRIO NiFpga Session. */
    return status;
}

/*--------------------------------------------------
 Function DI_Irq_Thread
 	 Purpose:		waits for IRQ,
 	 	 	 	 	services the interrupt
 	 Parameters:	thread resource
 	 Returns: 		none
 *-------------------------------------------------*/
void* DI_Irq_Thread(void* resource){

	// first step: cast the input to appropriate form
	ThreadResource* threadResource = (ThreadResource*) resource;

	// second step: enter a loop
	while (threadResource->irqThreadRdy == NiFpga_True){
		/* pause the loop while waiting for interrupt */
		uint32_t irqAssert = 0;
		Irq_Wait( 	threadResource->irqContext,
					threadResource->irqNumber,
					&irqAssert,
					(NiFpga_Bool*) &(threadResource->irqThreadRdy));

		/* check for timeout or interrupt */
		if (irqAssert & (1 << threadResource->irqNumber)){
			printf_lcd("interrupt_");
			Irq_Acknowledge(irqAssert);
		}
	}

	// third step: terminates new thread and returns from function
	pthread_exit(NULL);
	return NULL;
}

/*--------------------------------------------------
 Function wait
 	 Purpose:		waits for 5 ms
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

