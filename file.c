
/*
 * Copyright (c) 2010 by Suan-Aik Yeo <yeosuanaik@gmail.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include "file.h"
#include <ctype.h>

#define DEBUG 0 // set to 1 to view debugging messages from this file

#if DEBUG
#include "uart.h"
#endif

byte init_FAT_fs()
{
  return initFAT(g_block_buf);
}

/*
 * set file position to the beginning or end of file. the to argument has to be
 * either SEEK_START or SEEK_END.
 */
byte file_seek(file_descriptor_t *fd, byte to)
{
  word clus;

  if (SEEK_START == to)
  {
    fd->cluster = fd->dir_entry.first_cluster;
    fd->position = 0;
    return 1;
  }
  else if (to != SEEK_END)
  {
    #if DEBUG
    pmsg("Unknown file_seek target!\r\n");
    #endif
    return 0;
  }
  
  while (fd->position != fd->dir_entry.filesize)
  {
    if (fd->dir_entry.filesize - fd->position < prtn.clus_size)
    {
      fd->position = fd->dir_entry.filesize;
    }
    else
    {
      fd->position += prtn.clus_size - fd->position%prtn.clus_size;
      clus = next_clus(fd->cluster, g_block_buf);
      if (!clus)
      { 
        #if DEBUG
        pmsg("file_seek failed when seeking to next cluster!\r\n");
        #endif
        return 0;
      }
      fd->cluster = clus;
    }
  }
  
  return 1;
}

/*
 * set file position to the offset position
 */
byte file_seek_to_offset(file_descriptor_t *fd, dword offset)
{
  word clus;

  if (offset > fd->dir_entry.filesize)
  {
    #if DEBUG
    pmsg("offset position is bigger than file size!\r\n");
    #endif
    return 0;
  }
  
  while (fd->position != offset)
  {
    if (offset - fd->position < prtn.clus_size)
    {
      fd->position = offset;
    }
    else
    {
      fd->position += prtn.clus_size - fd->position%prtn.clus_size;
      clus = next_clus(fd->cluster, g_block_buf);
      if (!clus)
      {
        #if DEBUG
        pmsg("file_seek_to_offset failed when seeking to next cluster!\r\n");
        #endif
        return 0;
      }
      fd->cluster = clus;
    }
  }
  
  return 1;
}

/*
 * open a file for reading/writing. File has to be in the root directory.
 * If it does not exist, it will be created. An example argument for _filename_
 * is "test.txt". Filenames will be converted to all uppercase to adhere to the
 * FAT16 standard. _start_pos_ specifies the initial position for file
 * operations. It has to be set to SEEK_START or SEEK_END.
 */
byte file_open(char* filename, file_descriptor_t* fd, byte start_pos)
{
  char filename_cpy[13];
  char FAT16_filename[12];
  char *name;
  char *extension;
  byte i;
  byte file_exists;

  if (start_pos != SEEK_START && start_pos != SEEK_END)
  {
    #if DEBUG
    pmsg("Unknown start position for file_open()!\r\n");
    #endif
    return 0;
  }

  if (strlen(filename) > 12)
  {
    #if DEBUG
    pmsg("long filenames are not supported!\r\n");
    #endif
    return 0;
  }
  
  strncpy(filename_cpy, filename, 12);
  filename_cpy[12] = '\0';
  name = strtok(filename_cpy, ".");
  if (strlen(name) > 8)
  {
    #if DEBUG
    pmsg("The filename (excluding extension) exceeds 8 characters!\r\n");
    #endif
    return 0;
  }
  extension = strtok(NULL, ".");
  if (strlen(extension) > 3)
  {
    #if DEBUG
    pmsg("The extension exceeds 3 characters!\r\n");
    #endif
    return 0;
  }
  
  /* make the FAT16-compatible filename
      (must be all caps regardless if the file is not caps, 
      11 chars long, delimited with spaces) */
  strcpy(FAT16_filename, str_toupper(name));
  for (i = 0; i < 8 - strlen(name); i++)
  {
    strcat(FAT16_filename, " ");
  }
  strcat(FAT16_filename, str_toupper(extension));
  
  file_exists = getFile(&(fd->dir_entry), g_block_buf, FAT16_filename, 11);
  if (!file_exists)
  {
    if (!create_file(&(fd->dir_entry), FAT16_filename, 11, g_block_buf))
    {
      return 0;
    }
  }
  
  fd->cluster = fd->dir_entry.first_cluster;
  fd->position = 0;
  if (file_exists && start_pos == SEEK_END)
  {
    if(!file_seek(fd, SEEK_END)) { return 0; }
  }
  
  return 1;
}

/*
 * write a string to a file. length cannot exceed 512. Returns the number of
 * chars successfully written.
 */
word file_write(file_descriptor_t* fd, char* write_str, word length)
{
  word clus;
  word sec_bytes_left;
  word sec_offset;
  word write_length;
  word bytes_written = 0;
  dword sec_addr;

  // if this is the first write, alloc a cluster.
  if (fd->dir_entry.first_cluster == 0x0000)
  {
    clus = get_unused_cluster(g_block_buf);
    if (!clus) { return 0; }
    fd->dir_entry.first_cluster = clus;
    fd->cluster = clus;    
    if (!update_FAT(g_block_buf, fd->cluster, 0xffff)) { return 0; }
  }
  
  sec_offset = fd->position % 512;
  sec_bytes_left = 512 - sec_offset;
  sec_addr = get_sec_addr(fd->cluster, fd->position);
  if(!read_block(g_block_buf, sec_addr)) { return 0; }
  
  if (length <= sec_bytes_left)
  {
    write_length = length;
  }
  else
  {
    write_length = sec_bytes_left;
  }
  memmove(g_block_buf + sec_offset, write_str, write_length);
  if (!write_block(g_block_buf, sec_addr)) { return 0; }
  
  // update fd pointers and alloc new cluster if necessary
  if (fd->position%prtn.clus_size + write_length >= prtn.clus_size)
  {
    clus = get_unused_cluster(g_block_buf);
    if (!clus) { return 0; }
    if (!update_FAT(g_block_buf, fd->cluster, clus)) { return 0; }
    fd->cluster = clus;
    // terminate chain in FAT
    if (!update_FAT(g_block_buf, clus, 0xffff)) { return 0; }
  }
  fd->position += write_length;
  bytes_written += write_length;
  
  // write remainder of string to next sector
  if (length > sec_bytes_left)
  {
    memset(g_block_buf, '\0', 512);
    memmove(g_block_buf, write_str + sec_bytes_left, length - sec_bytes_left);
    if (!write_block(g_block_buf, get_sec_addr(fd->cluster, fd->position)))
    { 
      return 0; 
    }
    
    fd->position += length - sec_bytes_left;
    bytes_written += length - sec_bytes_left;
  }
  
  // update dir entry
  if (fd->position > fd->dir_entry.filesize)
  {
    fd->dir_entry.filesize = fd->position;
    if (!write_dir_entry(&(fd->dir_entry), g_block_buf)){ return 0; }
  }
  
  return bytes_written;
}

/*
 * Read _read_len_ characters from the file in _fd_ into the buffer _buf_.
 * _read_len_ cannot exceed 512. Returns the number of characters successfully
 * read, and 0 for error.
 */
word file_read(file_descriptor_t *fd, char *buf, word length)
{
  word read_length;
  word bytes_read = 0;
  word sec_offset;
  word sec_bytes_left;
  dword sec_addr;
  word clus;

  if (fd->cluster == 0)
  {
    #if DEBUG
    pmsg("Attempting to read from an empty file!\r\n");
    #endif
    return 0;
  }
  if (length > fd->dir_entry.filesize - fd->position)
  {
    #if DEBUG
    pmsg("[Warning] Attempting to read past end of file!\r\n");
    #endif
    if (fd->position == fd->dir_entry.filesize) { return 0; }
    length = fd->dir_entry.filesize - fd->position;
  }
  
  sec_offset = fd->position % 512;
  sec_bytes_left = 512 - sec_offset;
  sec_addr = get_sec_addr(fd->cluster, fd->position);
  if(!read_block(g_block_buf, sec_addr)) { return 0; }
  
  if (length <= sec_bytes_left)
  {
    read_length = length;
  }
  else
  {
    read_length = sec_bytes_left;
  }
  memmove(buf, g_block_buf + sec_offset, read_length); 
  
  // update fd pointers and move to new cluster if necessary
  if (fd->position%prtn.clus_size + read_length >= prtn.clus_size)
  {
    clus = next_clus(fd->cluster, g_block_buf);
    if (!clus) { return 0; }
    fd->cluster = clus;
  }
  fd->position += read_length;
  bytes_read += read_length;
  
  // read remainder of string from next sector
  if (length > sec_bytes_left)
  {
    if (!read_block(g_block_buf, get_sec_addr(fd->cluster, fd->position)))
    { 
      return 0; 
    }
    memmove(buf + sec_bytes_left, g_block_buf, length - sec_bytes_left);
    
    fd->position += length - sec_bytes_left;
    bytes_read += length - sec_bytes_left;
  }
  
  return bytes_read;
}

byte file_delete(file_descriptor_t* fd)
{
  word clus = fd->dir_entry.first_cluster;

  do
  {
    clus = clear_FAT(g_block_buf, clus);
    if (!clus)
    {
      #if DEBUG
      pmsg("Deleting file failed because there was a 0x00 in its FAT chain!\r\n");
      #endif
      return 0;
    }
  } while (clus < 0xfff8);
  
  if (!invalidate_dir_entry(&(fd->dir_entry), g_block_buf))
  {
    #if DEBUG
    pmsg("Deleting file failed while invalidating directory entry!\r\n");
    #endif
    return 0;
  }
  
  return 1;
}