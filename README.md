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
- Write speed: 512KBytes/s
- Read speed: 640KBytes/s
- Erase speed: 256MBytes/s

### Supported chips
- K9F2G08U0C
- You are welcome to add others

### License
In general the sorce code, PCB and schematic are under GPLv3 license but with limitations of:

firmware/libs/spl/CMSIS/License.doc

firmware/libs/spl/STM32_USB-FS-Device_Driver/ - http://www.st.com/software_license_agreement_liberty_v2

firmware/usb_cdc - http://www.st.com/software_license_agreement_liberty_v2

# Build
## Requirements
Linux based OS
## Build firmware
1. Download compiler and documents from Google Drive:
- Go to https://drive.google.com/drive/folders/1zLtebfLW6CxaWMq0rr4EfEPmeXfGk-7n?usp=sharing
- Press "Download All"
- mkdir ~/dev/
- mv ~/Downloads/nand_programmer*.zip ~/dev/
- unzip nand_programmer-*.zip
- cd nand_programmer/compiler/
- tar -xvf gcc-arm-none-eabi-4_9-2015q1-20150306-linux.tar.bz2
- cd ..
- git clone https://github.com/bbogush/nand_programmer.git
- cd nand_programmer/firmware
- make
## Burn firmware
- connect JTAG (J-Link) to board.
- sudo apt-get install openocd
- cd ~/dev/nand_programmer/nand_programmer/firmware
- make install
- NOTE: first time flash of chip should be burnt via internal bootloader (TODO).
## Build host application
- Install Qt5.5.1
- Open ~/dev/nand_programmer/nand_programmer/qt/qt.pro with QtCreator.
- Build->Run qmake
- Build->Build All
- Build->Run
## Schematic and PCB
- Install KiCad 5.0.1
- Open ~/dev/nand_programmer/nand_programmer/kicad/nand_programmator.pro
