# nand_programmer
NAND programmer

# General
NAND Programmer based on STM32 processor.

## Version
Unreleased version.

## Features
- USB interface
- PC client software for Linux. Windows, Mac planned to be supported.
- TSOP-48 socket for NAND chip
- NAND read/write/erase support
- NAND read of chip ID support
- Open KiCad PCB & Schematic
- Open source code

### Supported chips
- K9F2G08U0C
- You are welcome to add others

### License
In general the sorce code, PCB and schematic are under GPLv3 license but with limitations of:

firmware/libs/spl/CMSIS/License.doc

firmware/libs/spl/STM32_USB-FS-Device_Driver/ - http://www.st.com/software_license_agreement_liberty_v2

firmware/usb_cdc - http://www.st.com/software_license_agreement_liberty_v2
