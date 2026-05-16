# br_platformio_hwfef

PlatformIO platform package containing board definitions, variants, and upload scripts for Beyond Robotix hardware.

## Supported boards

| Board | MCU | Clock | Flash |
|-------|-----|-------|-------|
| MicroNode | STM32L431 | 80 MHz | 256 KB |
| CoreNode | STM32H743 | 480 MHz | 2 MB |
| MicroNodePlus | STM32H723 | 550 MHz | 1 MB |

## Usage

Add the following to your `platformio.ini`. PlatformIO will clone this package into its global cache on first build — no files are added to your project.

```ini
[platformio]
boards_dir = ${platformio.packages_dir}/br-boards/boards

[env]
platform_packages = br-boards @ git+https://github.com/BeyondRobotix/br_platformio_hwfef.git
board_build.variants_dir = ${platformio.packages_dir}/br-boards/variants
```

Then select a board as normal:

```ini
[env:my-env]
platform = ststm32
board = MicroNode       ; or CoreNode, MicroNodePlus
framework = arduino
board_build.ldscript = ${platformio.packages_dir}/br-boards/variants/MicroNode/ldscript.ld
```

### Bootloader upload (MicroNode only)

To flash the bootloader and application in a single step, add the bundled upload script:

```ini
extra_scripts = ${platformio.packages_dir}/br-boards/upload_bootloader_app.py
```

This flashes `MicroNodeBootloader.bin` to `0x8000000` followed by the compiled firmware to `0x800A000` via OpenOCD/ST-Link.

## Package contents

```
boards/                  Board JSON definitions
variants/
  MicroNode/             Pin map, peripheral config, linker scripts
  CoreNode/
  MicroNodePlus/
MicroNodeBootloader.bin  Pre-compiled bootloader for MicroNode (STM32L431)
upload_bootloader_app.py PlatformIO extra_script for two-stage bootloader flash
```
