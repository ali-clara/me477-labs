/*
 * emulate.h
 *
 *  Created on: Nov 12, 2020
 *      Author: garbini
 */

#ifndef EMULATE_H_
#define EMULATE_H_

#include "AIO.h"
#include "DIO.h"
#include "Encoder.h"

// substitute emulated i/o
#define Encoder_Counter(x) 			Encoder_CounterE(x)
#define EncoderC_initialize(x,y)	EncoderC_initializeE(x,y)
#define Aio_Write(x,y) 				Aio_WriteE(x,y)
#define	Aio_Read(x) 				Aio_ReadE(x)
#define AIO_initialize(x,y)			AIO_initializeE(x,y)
#define Dio_WriteBit(x,y) 			Dio_WriteBitE(x,y)


// emulator prototypes
uint32_t 		Encoder_CounterE(MyRio_Encoder* channel);
NiFpga_Status	EncoderC_initializeE(NiFpga_Session myrio_session, MyRio_Encoder *encC0p);
void 			Aio_WriteE(MyRio_Aio* channel, double value);
double 			Aio_ReadE(MyRio_Aio* channel);
void 			AIO_initializeE(MyRio_Aio *CI0p, MyRio_Aio *CO0p);
void 			Dio_WriteBitE(MyRio_Dio* channel, NiFpga_Bool value);
double			TODsec(void);
void 			fg_set(uint32_t mode, double amp, double freq_Hz);


#endif /* EMULATE_H_ */
