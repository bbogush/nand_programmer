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
