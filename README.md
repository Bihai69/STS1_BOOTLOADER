Bootloader files for prototype STS1_BOOTLOADER.
The aim of this repository is to offer a playground fo the people working on the officiall STS1_Bootloader.

All files in this repo are the most current WORKING Files, these may not have all codes snippets that are on the mebers PC's. 

To compile and flash the Bootloader and Dummy-app exedcute the following command: ./MakeAll.sh
This builds the different code projects, pads all necessary files with zero's and combines them into one image ready to flash.
Currently this repo is only tested on the official stm32 dev board : STM32F411RE.

For debugging a combination of serial com's and memory dumps are used. (stm32 programmer needed)
