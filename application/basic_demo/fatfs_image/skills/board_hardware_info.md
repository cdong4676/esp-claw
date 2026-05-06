# Current Board Hardware: elegoo_esp32_s3_superkit

Read this skill before operating hardware, assigning GPIOs, or writing Lua and board-specific code.

## Rules
- Before operating any hardware, read this skill first.
- Before assigning a GPIO, check whether it is already occupied below.
- When writing Lua or board-specific code, use the listed device names instead of guessing hardware wiring.

## Board Summary
- Board: `elegoo_esp32_s3_superkit`
- Chip: `esp32s3`
- Version: `1.0.0`
- Manufacturer: `elegoo`
- Description: elegoo_esp32_s3_superkit board configuration

## Device Inventory

### camera
- Occupied IO:
  - `dvp.dvp.data[0]` -> `GPIO11`
  - `dvp.dvp.data[1]` -> `GPIO9`
  - `dvp.dvp.data[2]` -> `GPIO8`
  - `dvp.dvp.data[3]` -> `GPIO10`
  - `dvp.dvp.data[4]` -> `GPIO12`
  - `dvp.dvp.data[5]` -> `GPIO18`
  - `dvp.dvp.data[6]` -> `GPIO17`
  - `dvp.dvp.data[7]` -> `GPIO16`
  - `dvp.dvp.vsync` -> `GPIO6`
  - `dvp.dvp.de` -> `GPIO7`
  - `dvp.dvp.pclk` -> `GPIO13`
  - `dvp.dvp.xclk` -> `GPIO15`
  - `sda` -> `GPIO4`
  - `scl` -> `GPIO5`

### display_lcd
- Occupied IO:
  - `spi.cs` -> `GPIO44`
  - `spi.dc` -> `GPIO43`
  - `mosi` -> `GPIO47`
  - `sclk` -> `GPIO21`

## Notes
- If a device has no explicit IO mapping here, treat it as unknown instead of guessing.
