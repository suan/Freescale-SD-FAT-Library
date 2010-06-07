
/*
 * Copyright (c) 2010 by Suan-Aik Yeo <yeosuanaik@gmail.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include "SPI.h"

void SPI_init(void)
{
  SPI_ENABLE = 1;
  SPI_MASTER_SEL = 1;
  SPI_CLK_PHASE = 0;
  SPIBR = 0b00000101; //set to ~400kHz
  
  // Enable SS (PM3) as an output pin
  SPI_SS_DIR = 1;
}

//assert SS pin
void Assert_SPI()
{
  SPI_SS = 0;
}

//deassert SS pin
void Deassert_SPI()
{
  SPI_SS = 1;
}

//transmit a byte to SD and return response byte
byte SPI_Byte(byte transmit)
{
  while(!SPI_TX_EMPTY);    // Wait for Transmit Empty Interrupt Flag
  SPI_DATA = transmit;     // This is CMD0 or'ed with Bit 6
  while(!SPI_INTERRUPT);
  
  return SPI_DATA;

}