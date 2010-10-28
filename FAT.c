
/*
 * Copyright (c) 2010 by Suan-Aik Yeo <yeosuanaik@gmail.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */

#include "FAT.h"

#define DEBUG 0 // set to 1 to view debugging messages from this file

#if DEBUG
#include "uart.h"
#endif

partition_info_t prtn;
byte g_block_buf[512];

byte initFAT(byte* buf)
{
  byte i;
  dword partition_addr;

  i = read_block(buf,0);
  if(!i){
    #if DEBUG
    pmsg("read timed out");
    #endif
    return 0;
  }
  
  if (buf[PARTENTRY1 + 0x04] != 0x04 && buf[PARTENTRY1 + 0x04] != 0x06)
  {
    #if DEBUG
    pmsg("drive is not FAT16!\r\n");
    #endif
    return 0;
  }
  
  partition_addr = ((dword)buf[PARTENTRY1 + 0x08] |
                    (dword)buf[PARTENTRY1 + 0x09] << 8 |
                    (dword)buf[PARTENTRY1 + 0x0a] << 16 |
                    (dword)buf[PARTENTRY1 + 0x0b] << 24) * 512;
  #if DEBUG
  pmsg("partition_addr is: "); disdword(partition_addr); pmsg("\r\n");
  #endif
  if (!read_block(buf, partition_addr))
  {
    #if DEBUG
    pmsg("couldn't read start of partition!\r\n"); 
    #endif
    return 0;
  }
  
  prtn_set_addr(buf, partition_addr);  
  return 1;
}

void prtn_set_addr(byte* bootSec, dword partition_addr)
{
  int resSecs;  //# of reserved sectors
  byte numFATs; //# of FATs
  word max_root_entries; /* max# of root dir entries */
  word secs_per_FAT;
  byte secs_per_clus;
  
  secs_per_clus = bootSec[0x0d];
  resSecs = ((bootSec[0x0f]<<8)&0xFF00)|(bootSec[0x0e]&0x00FF);
  max_root_entries = ((bootSec[0x12]<<8)&0xFF00)|(bootSec[0x11]&0x00FF);
  numFATs = bootSec[0x10];
  secs_per_FAT = ((bootSec[0x17]<<8)&0xFF00)|(bootSec[0x16]&0x00FF);
  
  prtn.bytes_per_sec = ((bootSec[0x0c]<<8)&0xFF00)|(bootSec[0x0b]&0x00FF);
  prtn.root_dir_addr = partition_addr + ((long)resSecs + (long)secs_per_FAT*(long)numFATs)*(long)(prtn.bytes_per_sec);
  prtn.FAT_addr =  partition_addr + (long)resSecs*(long)(prtn.bytes_per_sec);
  prtn.data_addr = prtn.root_dir_addr + (max_root_entries*32);
  prtn.clus_size = prtn.bytes_per_sec * secs_per_clus;
  prtn.FAT_size = prtn.bytes_per_sec * secs_per_FAT;
  prtn.root_dir_size = max_root_entries * 16;
  
  #if DEBUG
  pmsg("Bytes per sector: "); disword_dec(prtn.bytes_per_sec); pmsg("\r\n");
  pmsg("Number of reserved sectors: "); disword_dec(resSecs); pmsg("\r\n");
  pmsg("root dir at: 0x"); disdword(prtn.root_dir_addr); pmsg("\r\n");
  pmsg("FAT at: 0x"); disdword(prtn.FAT_addr); pmsg("\r\n");
  pmsg("Data starts at: 0x"); disdword(prtn.data_addr); pmsg("\r\n");
  #endif
}

// returns address of first empty root dir entry
dword get_empty_dir_entry(byte* buf)
{
  byte i,j;
  byte b; 
  
  for (i=0;i<32;i++)
  {
    b = read_block(buf, prtn.root_dir_addr + i*512);
    for(j=0;j<16;j++)
    {
      if(buf[j*32] == 0x00 || buf[j*32] == 0xE5)
      {
        #if DEBUG
        pmsg("found empty direntry at addr: 0x"); disdword(prtn.root_dir_addr + i*512 + j*32); pmsg("\r\n");
        #endif
        return (prtn.root_dir_addr + i*512 + j*32);
      }
    }
  }
  
  #if DEBUG
  pmsg("Failed to find empty directory entry!\r\n");
  #endif
  return 0;
}

// returns FAT index (cluster number) of first unused cluster found in FAT
word get_unused_cluster(byte* buf)
{
  byte i,j;
  byte b; 
  word FAT_entry;
  
  for (i=0;i<prtn.FAT_size/prtn.bytes_per_sec;i++)
  {
    b = read_block(buf, prtn.FAT_addr + i*512);
    for(j=0;j<512/2;j++)
    {
      FAT_entry = buf[j*2]&0x00ff | (buf[j*2+1]&0x00ff)<<8;
      if(FAT_entry == 0x0000)
      {
        #if DEBUG
        pmsg("found unused cluster. FAT index: 0x");
          disword(i*512/2 + j); pmsg("\r\n");
        #endif   
        return i*512/2 + j;                        
      }
    }
  }
  
  #if DEBUG
  pmsg("Failed to find unused cluster!\r\n");
  #endif
  return 0;
}

byte create_dir_entry(dir_entry_t* de, char* filename, byte fn_length, byte* buf)
{
  dword dir_entry_addr;

  memmove(de->filename, filename, fn_length);
  de->attributes = NEW_FILE_ATTR;
  de->filesize = 0;
  memset(de->unused_attr, 0, 14);
  
  dir_entry_addr = get_empty_dir_entry(buf);
  #if DEBUG
  pmsg("get_direntry returned: 0x"); disdword(dir_entry_addr); pmsg("\r\n");
  #endif
  if (!dir_entry_addr) 
  {
    #if DEBUG
    pmsg("Couldn't find empty direntry!\r\n");
    #endif
    return 0;
  }
  de->entry_addr = dir_entry_addr;
  
  // when direntry is allocated cluster is not yet entered
  de->first_cluster = 0x0000;
  
  return 1;
}

// writes a dir entry to disk
byte write_dir_entry(dir_entry_t* de, byte* buf)
{
  dword de_sec_offset;
  dword de_sec_start;
  byte b;
  word cluster_rev;
  dword filesize_rev;
  
  de_sec_offset = de->entry_addr % 512;
  de_sec_start = de->entry_addr - de_sec_offset;
  b = read_block(buf, de_sec_start);
  
  memreverse((void*)&(de->first_cluster), &cluster_rev, 2);
  memreverse((void*)&(de->filesize), &filesize_rev, 4);
  
  // enter dir_entry info into buffer (based on http://en.wikipedia.org/wiki/8.3_filename)
  memmove(buf+de_sec_offset, de->filename, 11);
  memmove(buf+de_sec_offset+0x0b, &(de->attributes), 1);
  memmove(buf+de_sec_offset+0x0c, &(de->unused_attr), 14);
  memmove(buf+de_sec_offset+0x1a, &cluster_rev, 2);
  memmove(buf+de_sec_offset+0x1c, &filesize_rev, 4);
  
  if(!write_block(buf, de_sec_start))
  {
    #if DEBUG
    pmsg("problem writing direntry!\r\n");
    #endif
    return 0;
  }
  
  return 1;
}

byte invalidate_dir_entry(dir_entry_t* de, byte* buf)
{
  de->filename[0] = 0xe5;
  return write_dir_entry(de, buf);
}

// make FAT[_cluster_] point to cluster _value_
byte update_FAT(byte* buf, word cluster, word value)
{
  dword abs_addr;
  dword sec_start;
  word value_rev;
  
  memreverse(&value, &value_rev, 2);
  abs_addr = prtn.FAT_addr + cluster*2;
  sec_start = abs_addr - (abs_addr % 512);
  if(!read_block(buf, sec_start))
  { 
    #if DEBUG
    pmsg("Problem updating FAT!\r\n");
    #endif
    return 0;
  }
  memmove(buf + (abs_addr % 512), &value_rev, 2);
  if (!write_block(buf, sec_start))
  {
    #if DEBUG
    pmsg("Problem updating FAT!\r\n");
    #endif
    return 0;
  }
  
  return 1;
}

// sets FAT[_cluster_] to 0x00 and returns FAT[_cluster_]'s original value
word clear_FAT(byte* buf, word cluster)
{
  dword abs_addr;
  dword sec_start;
  word ori_cluster;

  abs_addr = prtn.FAT_addr + cluster*2;
  sec_start = abs_addr - (abs_addr % 512);
  if(!read_block(buf, sec_start))
  { 
    #if DEBUG
    pmsg("Problem clearing FAT!\r\n");
    #endif
    return 0;
  }
  
  ori_cluster = (word)(buf + (abs_addr % 512));
  
  memmove(buf + (abs_addr % 512), (word)0, 2);
  if (!write_block(buf, sec_start))
  {
    #if DEBUG
    pmsg("Problem clearing FAT!\r\n");
    #endif
    return 0;
  }
  
  return ori_cluster;
}

// converts a file offset position to the actual address of the corresponding sector
dword get_sec_addr(word cluster, dword position)
{
  dword clus_addr;
  
  // data area starts from the 2nd cluster
  clus_addr = prtn.data_addr + prtn.clus_size*(cluster - 2);
  return (clus_addr + position%prtn.clus_size - position%512);
}

word next_clus(word clus, byte *buf)
{
  dword addr;
  word next;
  
  addr = prtn.FAT_addr + 2*clus;
  if(!read_block(buf, addr - addr % 512))
  { 
    #if DEBUG
    pmsg("next_clus failed when reading block!\r\n");
    #endif
    return 0;
  }
  memreverse(buf + addr%512, &next, 2);
  
  return next;
}

byte getFile(dir_entry_t* de, byte* buf, char* filename, byte length)
{
  int i,j;
  word cluster;
  dword file_size;
  byte b;
  char filename_upper[12];
  
  for (i = 0; i < prtn.root_dir_size/512; i++)
  {
    b = read_block(buf, prtn.root_dir_addr + i*512);
    for(j = 0; j < 16; j++)
    {
      if (*(buf + j*32)==0 || *(buf + j*32)==0x2e || *(buf + j*32)==0xe5)
      { continue; }
      
      strncpy(filename_upper, (char*)(buf+j*32), 11);
      filename_upper[11] = '\0';
      
      if(strcmp(filename, str_toupper(filename_upper)) == 0)
      {
        #if DEBUG
        pmsg("found file!\r\n");
        #endif
        
        memreverse(buf + j*32 + 0x1c, &file_size, 4);
        memreverse(buf + j*32 + 0x1a, &cluster, 2);
        
        #if DEBUG
        pmsg("cluster numer:"); disword_dec(cluster); pmsg("\r\n");
        #endif
        
        // fill in dir_entry
        memmove(de->filename, filename, length);
        de->attributes = *(buf + j*32 + 0x0b);
        memmove(de->unused_attr, buf + j*32 + 0x0c, 14);
        de->filesize = file_size;
        de->entry_addr = prtn.root_dir_addr + i*512 + j*32;
        de->first_cluster = cluster;
        
        return 1;                        
      }
    }
  }
  
  return 0;
}

// creates a new file that can be used in an OS
byte create_file(dir_entry_t* de, char* filename, byte fn_length, byte* buf)
{
  if(!create_dir_entry(de, filename, fn_length, buf))
  {
    #if DEBUG
    pmsg("Couldn't create direntry!\r\n");
    #endif
    return 0;
  }
  
  if(!write_dir_entry(de, buf))
  {
    #if DEBUG
    pmsg("problem writing direntry!\r\n");
    #endif
    return 0;
  }
  
  return 1;
}