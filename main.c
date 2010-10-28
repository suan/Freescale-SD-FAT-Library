
/*
 * Copyright (c) 2010 by Suan-Aik Yeo <yeosuanaik@gmail.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include "uart.h"
#include "file.h"

#define DEBUG 1

void init(void);

/*
 * This is a sample demo program that writes to a file, then reads and copies the
 * contents of that file to a 2nd file.
 */
void main(void)
{
  file_descriptor_t fd1, fd2;
  char *str = "The quick brown fox jumps over the lazy dog!";
  char str_buf[50];

  byte int_size;
  byte word_size;
  int_size = sizeof(int);
  word_size = sizeof(word);

  // initialize system
  init();
  #if DEBUG
  pmsg("All initializations successful!\r\n");
  #endif
  
  // open file1
  if (!file_open("FILE1.TXT", &fd1, SEEK_START))
  {
    #if DEBUG
    pmsg("Failed to create file FILE1.TXT!\r\n");
    #endif
    for(;;);
  }
  
  // write to file1
  if(file_write(&fd1, str, strlen(str)) != strlen(str))
  {
    #if DEBUG
    pmsg("problem writing to file FILE1.TXT!\r\n");
    #endif
    for(;;);
  }
  
  // seek back to beginning of file1
  if (!file_seek(&fd1, SEEK_START))
  {
    #if DEBUG
    pmsg("Failed to seek to beginning of FILE1.TXT!\r\n");
    #endif
    for(;;);
  }
  
  // read from file1 and save contents to _str_buf_
  if(file_read(&fd1, str_buf, strlen(str)) != strlen(str))
  {
    #if DEBUG
    pmsg("problem reading from file FILE1.TXT!\r\n");
    #endif
    for(;;);
  }
  
  // open file2
  if (!file_open("FILE2.TXT", &fd2, SEEK_START))
  {
    #if DEBUG
    pmsg("Failed to create file FILE2.TXT!\r\n");
    #endif
    for(;;);
  }
  
  // write to file2 what was read from file1
  if(file_write(&fd2, str_buf, strlen(str)) != strlen(str))
  {
    #if DEBUG
    pmsg("problem writing to file!\r\n");
    #endif
    for(;;);
  }
  
  // delete file1
  if (!file_delete(&fd1))
  {
    #if DEBUG
    pmsg("problem deleting to file1!\r\n");
    #endif
    for(;;);
  }
  
  #if DEBUG        
  pmsg("File operations successful!\r\n");
  #endif
  
  for(;;);  // make sure not to exit main!
}

void init(void)
{
  uart_init();
  SPI_init();
  if (!SD_init())
  {
    #if DEBUG
    pmsg("Problem initializing SD card!\r\n");
    #endif
    for(;;);
  }
  if (!init_FAT_fs())
  {
    #if DEBUG
    pmsg("Problem initializing FAT filesystem!\r\n");
    #endif
    for(;;);
  }
}