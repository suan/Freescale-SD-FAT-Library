
/*
 * Copyright (c) 2010 by Suan-Aik Yeo <yeosuanaik@gmail.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include "uart.h"

void uart_init(void)
{
  SCI_BAUD_LOW = 26;   // set to ~9600 baud
  SCI_TX_ENABLE = 1;
  SCI_RX_ENABLE = 1;
}

void outchar(unsigned char data)
{
  while(!SCI_TDRE);
  SCI_DATA_LOW = data;    
}

unsigned char inchar(void)
{
  while(!SCI_RDRF);
  return SCI_DATA_LOW;      
}

// print byte in hex
void disbyte(byte b)
{
  byte temp;
  
  temp = b;
  b >>= 4;
  b &= 0b00001111;
  if (b > 9)  outchar(b + 0x37);  //show first nibble
  else  outchar(b + 0x30);
  
  temp &= 0b00001111;
  if (temp > 9)  outchar(temp + 0x37);  //show second nibble
  else  outchar(temp + 0x30);
}

// print word in hex
void disword(word w)
{
    disbyte((byte) (w >> 8));
    disbyte((byte) (w & 0xff));
}

// print dword in hex
void disdword(dword dw)
{
    disword((word) (dw >> 16));
    disword((word) (dw & 0xffff));
}

// print byte as binary
void disbytebin(byte b)
{
  byte mask = 0b10000000;
  
  while(mask) {
    if(b & mask) outchar('1');
    else outchar('0');
    mask >>= 1;
  }
}

// print a message
void pmsg(char* msg)
{
  while(*msg)
  {
    outchar(*msg);
    msg++;
  }
}

// print word in decimal form
void disword_dec(word w)
{
    word num = 10000;
    byte started = 0;

    while(num > 0)
    {
        byte b = w / num;
        if(b > 0 || started || num == 1)
        {
            outchar('0' + b);
            started = 1;
        }
        w -= b * num;

        num /= 10;
    }
}

// print dword in decimal form
void disdword_dec(dword dw)
{
  dword num = 1000000000;
  byte started = 0;
  byte b;

  while(num > 0)
  {
      b = dw / num;
      if(b > 0 || started || num == 1)
      {
          outchar('0' + b);
          started = 1;
      }
      dw -= b * num;

      num /= 10;
  }
}

// show contents of buffer as hex bytes
void print_buffer(byte* buffer, word size, byte show_addr)
{
  word i;
  
  for (i = 0; i < size; i++)
  {
    if (show_addr)
    {
      outchar('['); disword(i); outchar(']');
    }
    disbyte(buffer[i]); outchar(' ');
  }
}

// show contents of buffer as chars
void outchar_buffer(byte* buffer, word size)
{
  word i;
  
  for (i = 0; i < size; i++)
  {
    outchar(buffer[i]); outchar(' ');
  }
}