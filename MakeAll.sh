#!/bin/bash
make clean --directory=STS1_DEV_APP/
make clean --directory=STS1_DEV_BOOTLOADER/
make --directory=STS1_DEV_APP/
make --directory=STS1_DEV_BOOTLOADER/

#python3 pad-bootloader.py
#cat write STS1_DEV_BOOTLOADER/build/STS1_DEV_BOOTLOADER.bin STS1_DEV_APP/build/STS1_DEV_APP.bin > firmware.bin

#st-flash write firmware.bin 0x8000000

st-flash write STS1_DEV_APP/build/STS1_DEV_APP.bin 0x8008000
st-flash write STS1_DEV_BOOTLOADER/build/STS1_DEV_BOOTLOADER.bin 0x8000000