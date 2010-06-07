
/*
 * Copyright (c) 2010 by Suan-Aik Yeo <yeosuanaik@gmail.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#ifndef __UTIL_H
#define __UTIL_H

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <ctype.h>

void memreverse(void* from, void* to, size_t length);
char* str_toupper(char *str);
char* str_n_toupper(char *str, word n);
void delay(void);

#endif
