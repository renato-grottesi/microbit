/* USB Mass Storage device file system
 * Copyrigh (c) 2010, Igor Skochinsky
 * based on SDFileStorage
 * Copyright (c) 2008-2009, sford
 */
 
/* Introduction
 * ------------
 * TODO: write one
 * we're basically using NXP's USBHotLite sample code, just plugging in our own FAT library
 */
 
#include "MSCFileSystem.h"
#include "usbhost_inc.h"

MSCFileSystem::MSCFileSystem(const char* name) :
  FATFileSystem(name)
{
}

void print_inquiry(USB_INT08U *inqReply)
{
    // see USB Mass Storage Class – UFI Command Specification,
    // 4.2 INQUIRY Command
    printf("Inquiry reply:\n");
    uint8_t tmp = inqReply[0]&0x1F;
    printf("Peripheral device type: %02Xh\n", tmp);
    if ( tmp == 0 )
        printf("\t- Direct access (floppy)\n");
    else if ( tmp == 0x1F )
        printf("\t- none (no FDD connected)\n");
    else
        printf("\t- unknown type\n");
    tmp = inqReply[1] >> 7;
    printf("Removable Media Bit: %d\n", tmp);
    tmp = inqReply[2] & 3;
    printf("ANSI Version: %02Xh\n", tmp);
    if ( tmp != 0 )
        printf("\t- warning! must be 0\n");
    tmp = (inqReply[2]>>3) & 3;
    printf("ECMA Version: %02Xh\n", tmp);
    if ( tmp != 0 )
        printf("\t- warning! should be 0\n");
    tmp = inqReply[2]>>6;
    printf("ISO Version: %02Xh\n", tmp);
    if ( tmp != 0 )
        printf("\t- warning! should be 0\n");
    tmp = inqReply[3] & 0xF;
    printf("Response Data Format: %02Xh\n", tmp);
    if ( tmp != 1 )
        printf("\t- warning! should be 1\n");
    tmp = inqReply[4];
    printf("Additional length: %02Xh\n", tmp);
    if ( tmp != 0x1F )
        printf("\t- warning! should be 1Fh\n");
    printf("Vendor Information: '%.8s'\n", &inqReply[8]);
    printf("Product Identification: '%.16s'\n", &inqReply[16]);
    printf("Product Revision: '%.4s'\n", &inqReply[32]);        
}

int MSCFileSystem::initialise_msc()
{
    USB_INT32S  rc;
    USB_INT08U  inquiryResult[INQUIRY_LENGTH];
    
    //print_clock();
    Host_Init();               /* Initialize the  host controller                                    */
    rc = Host_EnumDev();       /* Enumerate the device connected                                            */
    if (rc != OK)
    {
        fprintf(stderr, "Could not enumerate device: %d\n", rc);
        return rc;
    }
        
    
    /* Initialize the mass storage and scsi interfaces */
    rc = MS_Init( &_blkSize, &_numBlks, inquiryResult );
    if (rc != OK)
    {
        fprintf(stderr, "Could not initialize mass storage interface: %d\n", rc);
        return rc;
    }
    printf("Successfully initialized mass storage interface; %d blocks of size %d\n", _numBlks, _blkSize);
    print_inquiry(inquiryResult);
    // FATFileSystem supports only 512-byte blocks
    return _blkSize == 512 ? OK : 1;
}

int MSCFileSystem::disk_initialize()
{
    if ( initialise_msc() != OK )
        return 1;
        
    return 0;
}

int MSCFileSystem::disk_write(const char *buffer, int block_number)
{
    if ( OK == MS_BulkSend(block_number, 1, (USB_INT08U *)buffer) )
        return 0;
    return 1;
}

int MSCFileSystem::disk_read(char *buffer, int block_number)
{
    if ( OK == MS_BulkRecv(block_number, 1, (USB_INT08U *)buffer) )
        return 0;
    return 1;
}

int MSCFileSystem::disk_status() { return 0; }
int MSCFileSystem::disk_sync() { return 0; }
int MSCFileSystem::disk_sectors() { return _numBlks; }
