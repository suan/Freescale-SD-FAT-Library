
/*
 * Copyright (c) 2010 by Suan-Aik Yeo <yeosuanaik@gmail.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#ifndef __SD_H
#define __SD_H

#include "SPI.h"

#define BLOCKSIZE 512

byte CMD(byte, long);
byte read_block(byte* buf,dword addr);
byte write_block(byte* data, dword addr);
int SD_init(void);

#endif
