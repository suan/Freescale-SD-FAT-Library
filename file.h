
/*
 * Copyright (c) 2010 by Suan-Aik Yeo <yeosuanaik@gmail.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#ifndef __FILE_H
#define __FILE_H

#include "FAT.h"

#define SEEK_START 0
#define SEEK_END 1

typedef struct
{
  dir_entry_t dir_entry;
  dword position;
  word cluster;   /* Current cluster number */
} file_descriptor_t;

byte file_open(char* filename, file_descriptor_t* fd, byte start_pos);
word file_read(file_descriptor_t *fd, char *buf, word length);
word file_write(file_descriptor_t* fd, char* write_str, word length);
byte file_seek(file_descriptor_t *fd, byte to);
byte file_seek_to_offset(file_descriptor_t *fd, dword offset);
byte init_FAT_fs(void);
byte file_delete(file_descriptor_t* fd);

#endif
