## [1.0.0] - 2018-11-25
- Initial release. Basic NAND operations support.
## [2.0.0] - 2019-03-09
### Added
- Reading of bad blocks.
- Skipping of bad blocks during read/write/erase.
- New PCB with smaller size, serial, LEDs and SWD instead of J-TAG.
- Firmware error codes.
- Firmware support of read and write LEDs.
### Fixed
- File-Save operation.
## [2.1.0] - 2019-04-16
### Added
- Chip database.
### Fixed
- Disconnect on the first connection.
## [2.2.0] - 2019-04-28
### Added
- Windows support.
- Added description of error codes.
### Fixed
- Fixed multiple prints of read ID completed log message.
## [2.3.0] - 2019-05-05
### Added
- Version support.
- Added progress indicator for erase, read and write commands.
### Fixed
- Fixed erase command.
- Fixed read bad block table command.
## [3.0.0] - 2019-09-26
### Added
- New board design. Support of internal clock in FW.
### Fixed
- Race condition in packet handling code.
## [3.1.0] - 2019-11-16
### Added
- Read including spare area
- Configuration of spare area size, number of column and row cycles, commands, bad block mark offset
- New chips support
- BOOT0 switch to PCB.
### Fixed
- Lost of some packets due to optimization.
## [3.2.0] - 2020-03-14
### Added
- Chip autodetection.
- Firmware update.
- Support of K9F1208U0B and F59L2G81A.
### Fixed
- Bad block table initialization.
- Read of bad blocks.
## [3.3.0] - 2020-08-31
### Added
- Single side components PCB 
- Support of HY27US08121B.
### Fixed
- Fixed read of bad blocks when read spare area command is available.
## [3.4.0] - 2020-12-22
### Added
SPI flash support.
SOIC-8 adapter for AT45DB.
New parallel NAND and SPI chips.
Boost library for serial port.
### Fixed
Fixed saving of settings.
Fixed lost of packets in Windows.
Fixed UI on Windows.
Schematic refactoring.
## [3.4.1] - 2021-02-14
### Added
Added NAND reset on init.
## [3.4.2] - 2021-03-17
### Added
Firmware compilation on Windows.
#### Fixed
Firmware update.
UI issues.
## [3.5.0] - 2023-05-20
### Added
Support of HW ECC.
Operation time estimate.
Serial port selector.
Progress indication.
Read/write to file instead of memory buffer.
Support of flash more then 4GB.
UI improvements for large buffers.
Custom read/write range.
Pinout table on PCB.
Verify operation.
### Fixed
NAND timings.
Synchronization issues.
Low write speed on Windows.
