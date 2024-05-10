#!/bin/bash
make clean --directory=STS1-Bootloader_V2/
make clean --directory=STS1-DummyProgramm_V2/
make --directory=STS1-Bootloader_V2/
make --directory=STS1-DummyProgramm_V2/

python3 pad-bootloader.py
cat STS1-Bootloader_V2/build/STS1-Bootloader_V2.bin STS1-DummyProgramm_V2/build/STS1-DummyProgramm_V2.bin > firmware.bin

st-flash write firmware.bin 0x8000000

#st-flash write STS1_DEV_APP/build/STS1_DEV_APP.bin 0x8008000
#st-flash write STS1_DEV_BOOTLOADER/build/STS1_DEV_BOOTLOADER.bin 0x8000000

#sleep 10
#st-flash reset

#sleep 10 
#st-flash read read-firmware.bin 0x08000000 512K 
