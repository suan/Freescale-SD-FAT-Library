== Freescale SD FAT Library (<Place github link here>) ==
------------------------
Copyright (c) 2010 by Suan-Aik Yeo <yeosuanaik@gmail.com>
This is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation.

(I am in no way affiliated with Freescale Semiconductor Inc. The use of "Freescale" in the title is merely to indicate that this software is made to be used with their products.)

== Foreword ==
Although there are many FAT libraries out there written for SD card microcontroller interfacing, I found it hard to port them to the Freescale 9S12C32 and CodeWarrior. This library aims to fill that void, and it is specifically designed to be as “plug-and-play” as possible with 16-bit Freescale microcontrollers and CodeWarrior.
These functions are extremely light on memory usage, and run fine on a system with 2KB memory or less. However this comes at the expense of speed, as there is virtually no caching being used. As such this library is fine for basic usage but is hardly suitable for applications where read/write speed is a priority.

== Features ==
- Performs basic file reading and writing 
- Very low memory usage
- Tailor-made for CodeWarrior and Freescale microcontrollers

== Caveats ==
- Slow
- Only has FAT16 support (no FAT12/FAT32)
- No SDHC support
- No long filename (lfn) support (filenames cannot exceed 8 characters. Please refer to http://en.wikipedia.org/wiki/8.3_filename for more information.)
- No directory support (Can only deal with files in the root/home directory)
- No file features such as deletion and renaming

== Instructions ==
1) Insert the source files into your CodeWarrior project. This is how my project structure looks under CodeWarrior 5.9.0 (Specially marked entries are files that are not part of this library, but rather should be created by CodeWarrior when you create a new project):

Sources
  main.c
  SD.c
  SPI.c
  * datapage.c *
  FAT.c
  uart.c
  util.c
  file.c
Includes
  * derivative.h *
  * mc9s12c32.h *
  FAT.h
  SD.h
  SPI.h
  uart.h
  util.h
  file.h
  
2) If you are not using a 9S12C32 microcontroller, modify the hardware-dependent locations in uart.h and SPI.h as appropriate.

3) file.c is the user-level API and should contain all the functions you need to work with files.

4) main.c contains a sample program and is a good template to start from.

5) Debugging granularity can be set by toggling the DEBUG macros in main.c, file.c, FAT.c, and SD.c

== Tips for SD card interfacing beginners ==
- *Get your hardware down first!* This goes for other hardware aspects too. I've wasted whole days debugging only to find out that my connections were loose!
- *Not all SD cards are created equal* Its sad, but from my limited experience the combination of less-than-commercial-grade SD card readers and SD interfacing software means that many more finicky SD cards simply won't respond correctly, even though they will work fine on say, a laptop SD card reader. Personally the SD card that has been the most reliable for me is a PenDrive brand 256MB card. When in doubt, I'm sure SanDisk SD cards would be your best bet.