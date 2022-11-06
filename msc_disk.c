#include "bsp/board.h"
#include "tusb.h"
#include "class/msc/msc.h"
#include "Fat16Struct.h"
#include "pico.h"
#include <hardware/flash.h>
#include "checkUF2.h"
// whether host does safe-eject
static bool ejected = false;

// Some MCU doesn't have enough 8KB SRAM to store the whole disk
// We will use Flash as read-only disk with board that has

void software_reset()
{
    watchdog_enable(1, 1);
    while(1);
}

#define HTML_Page_Data \
 R"(<html><head><meta http-equiv="refresh" content="0;URL='https://raspberrypi.com/device/RP2?version=E0C9125B0D9B'"/></head><body>Redirecting to <a href='https://raspberrypi.com/device/RP2?version=E0C9125B0D9B'>raspberrypi.com</a></body></html>)"


#define DATA_Page_Data \
 "UF2 Bootloader v3.0\n\
Model: Raspberry Pi RP2\n\
Board-ID: RPI-RP2\n"


enum
{
  DISK_BLOCK_NUM  = 0x3ffff, // 8KB is the smallest size that windows allow to mount
  DISK_BLOCK_SIZE = 512,
  DISK_CLUSTER_SIZE = 8
};

//indexs of places
enum 
{
  INDEX_RESERVED = 0,
  INDEX_FAT_TABLE_1_START = 1, //Fat table size is 0x81
  INDEX_FAT_TABLE_2_START = 0x82,
  INDEX_ROOT_DIRECTORY = 0x103,
  INDEX_DATA_STARTS = 0x123,
  INDEX_DATA_END = (0x12b + DISK_CLUSTER_SIZE - 1)
};


#define MAX_SECTION_COUNT_FOR_FLASH_SECTION (FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE)
struct flashingLocation {
  unsigned int pageCountFlash;
}flashingLocation = {.pageCountFlash = 0};


uint8_t DISK_reservedSection[DISK_BLOCK_SIZE] = 
{
    0xEB, 0x3C, 0x90, 
    0x4D, 0x53, 0x57, 0x49, 0x4E, 0x34, 0x2E, 0x31, //MSWIN4.1
    0x00, 0x02,  //sector 512
    0x08,        //cluster size 8 sectors -unit for file sizes
    0x01, 0x00,  //BPB_RsvdSecCnt
    0x02,        //Number of fat tables
    0x00, 0x02,  //BPB_RootEntCnt  
    0x00, 0x00,  //16 bit fat sector count - 0 larger then 0x10000
    0xF8,        //- non-removable disks a 
    0x81, 0x00,       //BPB_FATSz16 - Size of fat table
    0x01, 0x00, //BPB_SecPerTrk 
    0x01, 0x00, //BPB_NumHeads
    0x01, 0x00, 0x00, 0x00, //??? BPB_HiddSec 
    0xFF, 0xFF, 0x03, 0x00, //BPB_TotSec32
    0x00,  //BS_DrvNum  - probably be 0x80 but is not? 
    0x00,  //
    0x29,
    0x50, 0x04, 0x0B, 0x00, //Volume Serial Number
    'R' , 'P' , 'I' , '-' , 'R' , 'P' , '2' , ' ' , ' ' , ' ' , ' ' , 
    'F', 'A', 'T', '1', '6', ' ', ' ', ' ', 

    0x00, 0x00,

    // Zero up to 2 last bytes of FAT magic code
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA
};

uint8_t DISK_fatTable[DISK_BLOCK_SIZE] =
{
    0xF8, 0xFF, 
    0xFF, 0xFF, 
    0xFF, 0xFF, //HTML doc
    0xFF, 0xFF, //Txt file
};


uint8_t DISK_rootDirectory[DISK_BLOCK_SIZE] = 
{
      // first entry is volume label
      'R' , 'P' , 'I' , '-' , 'R' , 'P' , '2' , ' ' , ' ' , ' ' , ' ' , 0x28, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0, 0x0, 0x0, 0x0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      // second entry is html file
      'I' , 'N' , 'D' , 'E' , 'X' , ' ' , ' ' , ' ' , 'H' , 'T' , 'M' , 0x21, 0x00, 0xC6, 0x52, 0x6D,
      0x65, 0x43, 0x65, 0x43, 0x00, 0x00, 0x88, 0x6D, 0x65, 0x43, 
      0x02, 0x00, //cluster location
      0xF1, 0x00, 0x00, 0x00, // html's files size (4 Bytes)
      // third entry is text file
      'I' , 'N' , 'F' , 'O' , '_' , 'U' , 'F' , '2' , 'T' , 'X' , 'T' , 0x21, 0x00, 0xC6, 0x52, 0x6D,
      0x65, 0x43, 0x65, 0x43, 0x00, 0x00, 0x88, 0x6D, 0x65, 0x43, 
      0x03, 0x00, //cluster location
      0x3E, 0x00, 0x00, 0x00 // readme's files size (4 Bytes)
};

//block size is not cluster size
uint8_t DISK_data[2][DISK_BLOCK_SIZE] =
{
  {HTML_Page_Data},
  {DATA_Page_Data}
};




void printReserveSectFat()
{
  printReserveSect((ReserveSect*)DISK_reservedSection);
}

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
  (void) lun;

  const char vid[] = "TinyUSB";
  const char pid[] = "Mass Storage";
  const char rev[] = "1.0";

  memcpy(vendor_id  , vid, strlen(vid));
  memcpy(product_id , pid, strlen(pid));
  memcpy(product_rev, rev, strlen(rev));
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
  (void) lun;

  // RAM disk is ready until ejected
  if (ejected) {
    // Additional Sense 3A-00 is NOT_FOUND
    tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
    return false;
  }

  return true;
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size)
{
  (void) lun;

  *block_count = DISK_BLOCK_NUM;
  *block_size  = DISK_BLOCK_SIZE;
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
  (void) lun;
  (void) power_condition;

  if ( load_eject )
  {
    if (start)
    {
      // load disk storage
    }else
    {
      // unload disk storage
      ejected = true;
    }
  }

  return true;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
  (void) lun;

  // out of ramdisk
  if ( lba >= DISK_BLOCK_NUM ) return -1;
  //printf("lba 0x%x, bufsize %d, offset %d\n",lba, bufsize, offset);
  //uint8_t const* addr = msc_disk[lba] + offset;
  //memcpy(buffer, addr, bufsize);

  uint8_t * addr = 0;
  if(lba == INDEX_RESERVED)
  {
    addr = DISK_reservedSection;
  }
  else if(lba == INDEX_FAT_TABLE_1_START || lba == INDEX_FAT_TABLE_2_START)
  {
    addr = DISK_fatTable;
  }
  else if(lba == INDEX_ROOT_DIRECTORY)
  {
    addr = DISK_rootDirectory;
  }
  else if(lba >= INDEX_DATA_STARTS && lba <= INDEX_DATA_END )
  {
    //printf("lba %d, bufsize %d, offset %d\n",lba, bufsize, offset);
    //DISK_data is only one section large but the cluster sizes are 8.
    //So if there was a larger file it would be bad.
    addr = DISK_data[(lba - INDEX_DATA_STARTS) / 8];
    //printf("loading section %d\n",(lba - INDEX_DATA_STARTS) / 8);
  }
  if(addr != 0)
  {
    memcpy(buffer, addr, bufsize);
  }
  else{
    memset(buffer,0, bufsize);
  }
  return (int32_t) bufsize;
}

bool tud_msc_is_writable_cb (uint8_t lun)
{
  (void) lun;


  return true;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
  (void) lun;
  printf("write - lba 0x%x, bufsize%d\n", lba,bufsize);
  
  // out of ramdisk
  if ( lba >= DISK_BLOCK_NUM ) return -1;
  //page to sector
  if(lba >= INDEX_DATA_END){
    //memcpy(&flashingLocation.buff[flashingLocation.sectionCount * 512], buffer, bufsize);
    //uint32_t ints = save_and_disable_interrupts();
    if(flashingLocation.pageCountFlash % MAX_SECTION_COUNT_FOR_FLASH_SECTION == 0)
    {
      printf("doing flash at offset 0x%x\n",flashingLocation.pageCountFlash * FLASH_PAGE_SIZE);
      flash_range_erase((flashingLocation.pageCountFlash / 16) * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
    }
    unsigned int * returnSize;
    unsigned char *addressUF2 = getUF2Info(buffer,returnSize);
    if(addressUF2 != 0)
    {
      printf("UF2 FILE %d!!!\n",flashingLocation.pageCountFlash );
      flash_range_program(flashingLocation.pageCountFlash * FLASH_PAGE_SIZE,
                          &buffer[32], 
                          256);
      flashingLocation.pageCountFlash += 1; //(*returnSize/ FLASH_PAGE_SIZE);
    }
    else{
      flash_range_program(flashingLocation.pageCountFlash * FLASH_PAGE_SIZE,
                          buffer, 
                          bufsize);
      flashingLocation.pageCountFlash += (bufsize/ FLASH_PAGE_SIZE);
    }
  }

  if(lba == INDEX_ROOT_DIRECTORY)
  {
    printf("\n\n\nRestarting the raspberry pi pico!!!! \n\n\n");
    sleep_ms(100);//just to make sure all uart get out

    software_reset();
  }

//#ifndef CFG_EXAMPLE_MSC_READONLY
//  uint8_t* addr = msc_disk[lba] + offset;
//  memcpy(addr, buffer, bufsize);
//#else
  

  return (int32_t) bufsize;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb (uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize)
{
  // read10 & write10 has their own callback and MUST not be handled here

  void const* response = NULL;
  int32_t resplen = 0;

  // most scsi handled is input
  bool in_xfer = true;

  switch (scsi_cmd[0])
  {
    default:
      // Set Sense = Invalid Command Operation
      tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

      // negative means error -> tinyusb could stall and/or response with failed status
      resplen = -1;
    break;
  }

  // return resplen must not larger than bufsize
  if ( resplen > bufsize ) resplen = bufsize;

  if ( response && (resplen > 0) )
  {
    if(in_xfer)
    {
      memcpy(buffer, response, (size_t) resplen);
    }else
    {
      // SCSI output
    }
  }

  return (int32_t) resplen;
}
