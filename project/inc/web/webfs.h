/*********************************************************************
 *
 * FileName:        WEBFS.h
 * Basis of MPFS2 (Microchip File System).
 * WEBFS has its differences Based.
 *
 ********************************************************************/
#ifndef __WEBFS1_H
#define __WEBFS1_H

#include "flash_eep.h"

#define WEBFS_DISK_FADDR  0x0D0000

#define WEBFS_DISK_ID 	0x42455746 // заголовок WEBFiles.bin
#define WEBFS_DISK_VER 	0x0001	   // версия

#define MAX_FILE_NAME_SIZE  64

#ifndef MAX_WEBFS_OPENFILES
  #define MAX_WEBFS_OPENFILES  31
#endif

        #define WEBFS_FLAG_ISZIPPED	0x0001  // Indicates a file is compressed with GZIP compression
        #define WEBFS_FLAG_HASINDEX	0x0002  // Indicates a file has an associated index of dynamic variables
        #define WEBFS_INVALID		0xffffffff   // Indicates a position pointer is invalid
        #define WEBFS_INVALID_FAT	0xffff       // Indicates an invalid FAT cache
		typedef uint32_t WEBFS_PTR;	// WEBFS Pointers are currently uint32s
		typedef uint8_t WEBFS_HANDLE;	// WEBFS Handles are currently stored as uint8s
        #define WEBFS_INVALID_HANDLE	0xff	// Indicates that a handle is not valid


        // Stores each file handle's information
        // Handles are free when addr = WEBFS_INVALID
        typedef struct
        {
                WEBFS_PTR addr;          // Current address in the file system
                uint32_t bytesRem;         // How many uint8s remain in this file
#ifdef USE_MAX_IRAM
                uint32_t fatID;            // ID of which file in the FAT was accessed
#else
                uint16_t fatID;            // ID of which file in the FAT was accessed
#endif
        } WEBFS_STUB;

        // Indicates the method for WEBFSSeek
        typedef enum
        {
                WEBFS_SEEK_START         = 0,    // Seek forwards from the front of the file
                WEBFS_SEEK_END,                  // Seek backwards from the end of the file
                WEBFS_SEEK_FORWARD,              // Seek forward from the current position
                WEBFS_SEEK_REWIND                // See backwards from the current position
        } WEBFS_SEEK_MODE;


        typedef struct __attribute__((packed))
        {
                uint32_t id;
                uint16_t ver;
                uint16_t numFiles;
                uint32_t disksize;
        } WEBFS_DISK_HEADER ;

        typedef struct  __attribute__((packed))
        {
                uint32_t blksize;             // Length of file data - headlen
                uint16_t headlen;             // headlen (Length of File Name + 0)
                uint16_t flags;               // Flags for this file
        } WEBFS_FHEADER;

        // Stores the data for an WEBFS1 FAT record
        typedef struct
        {
                uint32_t string;           // Pointer to the file name
                uint32_t data;             // Address of the file data
                uint32_t len;              // Length of file data
#ifdef USE_MAX_IRAM
                uint32_t  flags;           // Flags for this file
#else
                uint16_t  flags;           // Flags for this file
#endif
        } WEBFS_FAT_RECORD ;


void WEBFSInit(void);
WEBFS_HANDLE WEBFSOpen(uint8_t* cFile);
void WEBFSClose(WEBFS_HANDLE hWEBFS);

uint16_t WEBFSGetArray(WEBFS_HANDLE hWEBFS, uint8_t* cData, uint16_t wLen);

uint16_t WEBFSGetFlags(WEBFS_HANDLE hWEBFS);
uint32_t WEBFSGetSize(WEBFS_HANDLE hWEBFS);
uint32_t WEBFSGetBytesRem(WEBFS_HANDLE hWEBFS);
WEBFS_PTR WEBFSGetStartAddr(WEBFS_HANDLE hWEBFS);
WEBFS_PTR WEBFSGetEndAddr(WEBFS_HANDLE hWEBFS);
bool WEBFSGetFilename(WEBFS_HANDLE hWEBFS, uint8_t* cName, uint16_t wLen);
uint32_t WEBFSGetPosition(WEBFS_HANDLE hWEBFS);
uint32_t WEBFS_max_size(void);
uint32_t WEBFS_curent_size(void);
uint32_t WEBFS_base_addr(void);


extern volatile bool isWEBFSLocked; // Lock WEBFS access during the upgrade
extern uint16_t numFiles;

extern WEBFS_FAT_RECORD fatCache;
extern WEBFS_STUB WEBFSStubs[MAX_WEBFS_OPENFILES+1];
extern uint32_t disk_base_addr;

#endif
