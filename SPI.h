
/*
 * Copyright (c) 2010 by Suan-Aik Yeo <yeosuanaik@gmail.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#ifndef __SPI_H
#define __SPI_H

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */

/*
 * 9s12c32 microcontroller dependent locations that you might have to change!
 */
#define SPI_ENABLE SPICR1_SPE /* SPI System Enable Bit */
#define SPI_MASTER_SEL SPICR1_MSTR  /* SPI Master/Slave Mode Select Bit */
#define SPI_CLK_PHASE SPICR1_CPHA /* SPI Clock Phase Bit */
#define SPI_BAUD_RATE SPIBR /* SPI Baud Rate Register */
#define SPI_SS_DIR DDRM_DDRM3 /* SPI Slave Select Pin Data Direction Register */
#define SPI_SS PTM_PTM3 /* SPI Slave Select Pin */
#define SPI_TX_EMPTY SPISR_SPTEF /* SPI Transmit Empty Interrupt Flag */
#define SPI_INTERRUPT SPISR_SPIF /* SPI Interrupt Flag */
#define SPI_DATA SPIDR  /* SPI Data Register */

void SPI_init(void);
void Assert_SPI(void);
void Deassert_SPI(void);
byte SPI_Byte(byte);

#endif
