
/*
 * Copyright (c) 2010 by Suan-Aik Yeo <yeosuanaik@gmail.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#ifndef __UART_H
#define __UART_H

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */

/*
 * 9s12c32 microcontroller dependent locations that you might have to change!
 */
#define SCI_TX_ENABLE SCICR2_TE /* SCI Transmitter enable bit */
#define SCI_RX_ENABLE SCICR2_RE /* SCI Receiver enable bit */
#define SCI_BAUD_LOW SCIBDL /* SCI Baud Rate Register (Low Byte) */
#define SCI_TDRE SCISR1_TDRE /* SCI Transmit Data Register Empty flag */
#define SCI_DATA_LOW SCIDRL  /* SCI Data Register (Low Byte) */
#define SCI_RDRF SCISR1_RDRF /* SCI Receive Data Register Full flag */

void uart_init(void);
void outchar(unsigned char);
unsigned char inchar(void);
void pmsg(char* msg);
void disword_dec(word w);
void disdword_dec(dword dw);
void print_buffer(byte* buffer, word size, byte show_addr);
void outchar_buffer(byte* buffer, word size);
void disword(word w);
void disdword(dword dw);
void disbyte(byte);
void disbytebin(byte);

#endif
