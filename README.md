# Example unoptimized Drag n' Drop Programmer
This example is trying to mimic the way that the raspberry pi pico bootloader copies a UF2 file into the flash.  The actuall bootloader can be found [here](https://github.com/raspberrypi/pico-bootrom), but it's highly optimized "by design", so i made this to explore how it worked at a less optimized way.  Starting point for this was the [tinyUSB CDC_MSC example](https://github.com/hathach/tinyusb/tree/master/examples/device/cdc_msc)

## Things inside this demo are
* USB MSC - "USB storage"
* FAT16 file system
* wrighting to flash
* Basic UF2 Parsing


## to build
* cmake .. -DPICO_COPY_TO_RAM=1

## Command to flash pico
* openocd -f interface/picoprobe.cfg -f target/rp2040.cfg -c "program PICO_MSC.elf verify reset exit"

# Things inside this repo

* fatImages - These are straight rips from a raspberry pi pico.  Usefully for see the raw hex values of the fat table
* FileOnPico - These are the files that the pico has by default
* uploads - these are test programs that you can drag onto the raspberry pi pico




# important links

### Fat file system
* [best overview of fat files](http://elm-chan.org/docs/fat_e.html)
* [another good one](https://www.pjrc.com/tech/8051/ide/fat32.html)
* [more fat](http://www.tavi.co.uk/phobos/fat.html#media_descriptor)

### Moving program from flash to RAM
* [Raspberry pi forum post about flash to ram](https://forums.raspberrypi.com/viewtopic.php?t=318471)
* [LD_script for copy to ram](https://github.com/raspberrypi/pico-sdk/blob/2062372d203b372849d573f252cf7c6dc2800c0a/src/rp2_common/pico_standard_link/memmap_copy_to_ram.ld)
* [CMAKE section to copy flash to ram](https://github.com/raspberrypi/pico-sdk/blob/2062372d203b372849d573f252cf7c6dc2800c0a/src/rp2_common/pico_standard_link/CMakeLists.txt#L49)

### No flash vs copy to ram
* https://forums.raspberrypi.com/viewtopic.php?t=312474


### Wrighting to flash
* https://www.makermatrix.com/blog/read-and-write-data-with-the-pi-pico-onboard-flash/
* https://raspberrypi.github.io/pico-sdk-doxygen/group__hardware__flash.html
* https://kevinboone.me/picoflash.html?i=1


### UF2 File conversion
* [Good wright up on UF2](https://microsoft.github.io/uf2/)

### [How to reset the chip](https://raspberrypi.stackexchange.com/questions/132439/pi-pico-software-reset-using-the-c-sdk)
