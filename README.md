# CANable 2.0 Firmware

This repository contains sources for the slcan CANable 2.0 firmware. This firmware implements non-standard slcan commands to support CANFD messaging alongside a LAWICEL-style command set.

## Supported Commands

- `O` - Open channel 
- `C` - Close channel 
- `S0` - Set nominal bitrate to 10k
- `S1` - Set nominal bitrate to 20k
- `S2` - Set nominal bitrate to 50k
- `S3` - Set nominal bitrate to 100k
- `S4` - Set nominal bitrate to 125k
- `S5` - Set nominal bitrate to 250k
- `S6` - Set nominal bitrate to 500k
- `S7` - Set nominal bitrate to 750k
- `S8` - Set nominal bitrate to 1M
- `S9` - Set nominal bitrate to 83.3k
- `Y1` - Set data bitrate to 1M (CANFD only)
- `Y2` - Set data bitrate to 2M (CANFD only) (default)
- `Y4` - Set data bitrate to 4M (CANFD only)
- `Y5` - Set data bitrate to 5M (CANFD only)
- `M0` - Set mode to normal mode (default)
- `M1` - Set mode to silent mode
- `A0` - Disable automatic retransmission 
- `A1` - Enable automatic retransmission (default)
- `tIIILDD...` - Transmit data frame (Standard ID) [ID, length, data]
- `TIIIIIIIILDD...` - Transmit data frame (Extended ID) [ID, length, data]
- `RIIIIIIIIL` - Transmit remote frame (Extended ID) [ID, length]
- `rIIIL` - Transmit remote frame (Standard ID) [ID, length]
- `dIIILDD...` - Transmit CAN FD standard ID (no BRS) [ID, length]
- `DIIIIIIIILDD...` - Transmit CAN FD extended ID (no BRS) [ID, length]
- `bIIILDD...` - Transmit CAN FD BRS standard ID [ID, length]
- `BIIIIIIIILDD...` - Transmit CAN FD extended ID [ID, length]

- `V` - Returns firmware version and remote path as a string
- `E` - Returns error register

Note: CANFD message lengths are as follows (expressed in hexadecimal):
- `0-8`: Same as standard CAN
- `9`: Length = 12
- `A`: Length = 16
- `B`: Length = 20
- `C`: Length = 24
- `D`: Length = 32
- `E`: Length = 48
- `F`: Length = 64

Note: Channel configuration commands must be sent before opening the channel. The channel must be opened before transmitting frames.

This firmware currently does not provide any ACK/NACK feedback for serial commands.

## Building

Firmware builds with GCC. Specifically, you will need gcc-arm-none-eabi, which
is packaged for Windows, OS X, and Linux on
[Launchpad](https://launchpad.net/gcc-arm-embedded/+download). Download for your
system and add the `bin` folder to your PATH.

Your Linux distribution may also have a prebuilt package for `arm-none-eabi-gcc`, check your distro's repositories to see if a build exists. Simply compile by running `make`. 

## Flashing with the Bootloader

Plug in your CANable2 while pressing down the BOOT button. The blue LED should be dimly illuminated. Next, type `make flash` and your CANable will be updated to the latest firwmare. Unplug/replug the device after moving the boot jumper back, and your CANable will be up and running.


## License

See LICENSE.md
