#pragma once

#include "assert.h"
#include "stdio.h"
#include "stdint.h"

typedef struct {
    char name[8];
    char ext[3];
    uint8_t attrs;
    uint8_t reserved;
    uint8_t createTimeFine;
    uint16_t createTime;
    uint16_t createDate;
    uint16_t lastAccessDate;
    uint16_t highStartCluster;
    uint16_t updateTime;
    uint16_t updateDate;
    uint16_t startCluster;
    uint32_t size;

} __attribute__((packed)) DirEntry;

static_assert(sizeof(DirEntry) == 32, "DirEntry size is not right");

typedef struct ReserveSect {
    uint8_t     bootJump[3];                              /* 0  Jump to bootstrap */
    uint8_t     oemName[8];                               /* 3  OEM Manufacturer name */
    uint16_t    sectorSize         __attribute((packed)); /* 11 Sector size in bytes */
    uint8_t     clusterSize;                              /* 13 Sectors per cluster */
    uint16_t    reservedSectors    __attribute((packed)); /* 14 Number of reserved sectors */
    uint8_t     fatCopies;                                /* 16 # of FAT copies */
    uint16_t    rootEirEntries     __attribute((packed)); /* 17 # of root directory entries */
    uint16_t    totalSectorsFat16  __attribute((packed)); /* 19 (UNUSED FAT32) Total number of sectors */
    uint8_t     mediaType;                                /* 21 Media type */
    uint16_t    fatTableSize       __attribute((packed));
    uint16_t    secPerTrk          __attribute((packed));
    uint16_t    numHeads           __attribute((packed));
    uint32_t    hiddSec            __attribute((packed));
    uint32_t    totalSec32         __attribute((packed));
}  __attribute((packed)) ReserveSect;


void printReserveSect(ReserveSect * bootSect)
{
    printf("bootOEM %.8s\n", bootSect->oemName);
    printf("cluster size %x\n",bootSect->clusterSize);
    printf("fat copies %x\n", bootSect->fatCopies);
    printf("fat table size %x\n", bootSect->fatTableSize);
    printf("totalSec32 %x\n", bootSect->totalSec32);
}

