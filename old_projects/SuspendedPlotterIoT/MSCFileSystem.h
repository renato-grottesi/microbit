/* USB Mass Storage device file system
 * Copyrigh (c) 2010, Igor Skochinsky
 * based on SDFileStorage
 * Copyright (c) 2008-2009, sford
 */
 
#ifndef MSCFILESYSTEM_H
#define MSCFILESYSTEM_H

#include "mbed.h"
#include "FATFileSystem.h"

/* Class: MSCFileSystem
 *  Access the filesystem on an attached USB mass storage device (e.g. a memory stick)
 *
 * Example:
 * > MSCFileSystem msc("msc");
 * > 
 * > int main() {
 * >     FILE *fp = fopen("/msc/myfile.txt", "w");
 * >     fprintf(fp, "Hello World!\n");
 * >     fclose(fp);
 * > }
 */
class MSCFileSystem : public FATFileSystem {
public:

    /* Constructor: MSCFileSystem
     *  Create the File System for accessing a USB mass storage device
     *
     * Parameters:
     *  name - The name used to access the filesystem
     */
    MSCFileSystem(const char* name);
    virtual int disk_initialize();
    virtual int disk_write(const char *buffer, int block_number);
    virtual int disk_read(char *buffer, int block_number);    
    virtual int disk_status();
    virtual int disk_sync();
    virtual int disk_sectors();

protected:

    int initialise_msc();
    uint32_t _numBlks;
    uint32_t _blkSize;
};

#endif
