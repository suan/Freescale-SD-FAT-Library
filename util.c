
/*
 * Copyright (c) 2010 by Suan-Aik Yeo <yeosuanaik@gmail.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include "util.h"

// reverses a buffer
void memreverse(void* from, void* to, size_t length)
{
  byte i;
  byte* from_b = (byte*)from;
  byte* to_b = (byte*)to;
  from_b--;   // it appears that sending in "&some_num_type" is offset by 1 to the left
  
  for (i = 0; i < length; i++) 
  {
    to_b[i] = from_b[length - i];
  }
}

char* str_toupper(char *str)
{
  word i = 0;

  while (str[i])
  {
    str[i] = toupper(str[i]);
    i++;
  }
  return str;
}

char* str_n_toupper(char *str, word n)
{
  word i = 0;

  while (i < n)
  {
    str[i] = toupper(str[i]);
    i++;
  }
  return str;
}

void delay(void){
  int i;
  int j;
  for (i=0; i<100; i++) {
    j = i;
  }
}