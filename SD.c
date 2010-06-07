
/*
 * Copyright (c) 2010 by Suan-Aik Yeo <yeosuanaik@gmail.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include "SD.h"

#define DEBUG 0 // set to 1 to view debugging messages from this file

#if DEBUG
#include "uart.h"
#endif

int SD_init(void)
{
  int l;
  byte i;

  SPI_BAUD_RATE = 0b01110111; //set to ~12kHz
  Deassert_SPI();  
  for(l=0;l<10;l++)
  {
    i = SPI_Byte(0xFF);
  }
  
  Assert_SPI();     
  i = SPI_Byte(0x00 | 0x40);
  i = SPI_Byte(0x00);
  i = SPI_Byte(0x00);
  i = SPI_Byte(0x00);
  i = SPI_Byte(0x00);
  i = SPI_Byte(0x95); //The CRC check, only matters on cmd0
  
  for(l=0; l<500; l++)
  {
    i=SPI_Byte(0xFF);
    if(i==0x01) break;
  }
  
  if(i==0x01)
  {
    #if DEBUG
    pmsg("cmd0 success!\r\n");
    #endif
  }
  else
  {
    #if DEBUG
    pmsg("Problem sending SD cmd0\r\n");
    #endif
    return 0;
  }
  
  for (l=0;l<1000;l++)
  {
    i = CMD(55,0);
    i = CMD(41,0);
    if (!i){
      #if DEBUG
        pmsg("cmd 55/41 success\r\n");
      #endif
      break;
    }
  }
  if(i != 0) return 0;
  
  SPI_BAUD_RATE =  0b00000000;  // set baud rate to fastest possible
  
  i = CMD(16,512);
  Deassert_SPI();
  for(l=0;l<3;l++)
  {
    i = SPI_Byte(0xFF);
  }
  Assert_SPI();
  
  return 1;
}

byte CMD(byte num, long arg) 
{
  byte b;
  
  Assert_SPI(); // assert chip select for the card
  
  b=SPI_Byte(0xFF); // dummy byte
  b=SPI_Byte(0x40|num);  // command token
  
  // send argument in little endian form (MSB first)
  b=SPI_Byte((byte)(arg>>24));
  b=SPI_Byte((byte)(arg>>16));
  b=SPI_Byte((byte)(arg>>8));
  b=SPI_Byte((byte)arg);
  
  b=SPI_Byte(0x95);  // checksum valid for CMD0, not needed
              // thereafter, so we can hardcode this value
              
  b=SPI_Byte(0xFF); // dummy to give card time to process
  
  return SPI_Byte(0xFF); // query return value from card
}

byte read_block(byte* buf,dword addr)
{
  int i;
  byte c;
  byte r1;
  
  if (addr % 512 != 0)
  {
    #if DEBUG
    pmsg("read_block: Address 0x"); disdword(addr);
      pmsg("is not aligned to a 512 byte block!\r\n");
    #endif
    return 0;
  }
  
  #if DEBUG
  pmsg("Reading block at address 0x"); disdword(addr); pmsg("\r\n");
  #endif
  
  r1=CMD(17,addr);
  
  for (i=0;i<50;i++)     // wait until the data is found
  {
    if (r1==0x00) break;
    r1 = SPI_Byte(0xFF);
  }
  if (r1!=0x00)
  {
    #if DEBUG
    pmsg("Read block timed out!\r\n");
    #endif
    return 0; 
  }
  
  
  c = SPI_Byte(0xFF);
  while (c!=0xFE)
  {
    c=SPI_Byte(0xFF);
  } // wait for the "data follows" code
  for (i=0;i<512;i++)
  {    
    *(buf++)=SPI_Byte(0xFF); 
  }
  c=SPI_Byte(0xFF);
  c=SPI_Byte(0xFF);  // dummy bytes to clear any queues
  return 1;
}

byte write_block(byte* data, dword addr)
{
  byte c;
  short i;
  
  if (addr % 512 != 0)
  {
    #if DEBUG
    pmsg("write_block: Address 0x"); disdword(addr);
      pmsg("is not aligned to a 512 byte block!\r\n");
    #endif
    return 0;
  }
  
  #if DEBUG
  pmsg("Writing block at address 0x"); disdword(addr); pmsg("\r\n");
  #endif
  
  if (CMD(24,addr) != 0) 
  {
    #if DEBUG
    pmsg("Failed to write block!\r\n");
    #endif 
    return 0;
  }
  c=SPI_Byte(0xFF); c=SPI_Byte(0xFF); c=SPI_Byte(0xFE); // lead in to actual data
  for (i=0;i<BLOCKSIZE;i++) c=SPI_Byte(data[i]);
  c=SPI_Byte(0xFF); c=SPI_Byte(0xFF); // dummy before response recieved
  c=SPI_Byte(0xFF); c&=0x1F; // bit mask for write error codes 
  // see http://elm-chan.org/docs/mmc/mmc_e.html
  if (c!=0x05)
  {
    #if DEBUG
    pmsg("Failed to write block!\r\n");
    #endif
    return 0; 
  }
  while (SPI_Byte(0xFF)!=0xFF);  // block until write finished
  
  return 1;
}