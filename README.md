# br_platformio_hwdef

PlatformIO platform package containing board definitions, variants, and upload scripts for Beyond Robotix hardware.

## Supported boards

| Board | MCU | Clock | Flash |
|-------|-----|-------|-------|
| MicroNode | STM32L431 | 80 MHz | 256 KB |
| CoreNode | STM32H743 | 480 MHz | 2 MB |
| MicroNodePlus | STM32H723 | 550 MHz | 1 MB |

## First-time setup

PlatformIO needs the board definitions present before it can resolve packages, so a one-time global install is required on each machine:

```sh
pio pkg install -g -t "git+https://github.com/BeyondRobotix/br_platformio_hwdef.git"
```

After that, `pio run` handles everything automatically and will keep the package up to date.

## Usage

Add the following to your `platformio.ini`. No files are added to your project.

```ini
[platformio]
boards_dir = ${platformio.packages_dir}/br-boards/boards

[env]
platform_packages = br-boards @ git+https://github.com/BeyondRobotix/br_platformio_hwdef.git
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

### Bootloader upload

To flash the bootloader and application in a single ST-Link step, add the bundled upload script:

```ini
extra_scripts = ${platformio.packages_dir}/br-boards/upload_bootloader_app.py
board_build.ldscript = ${platformio.packages_dir}/br-boards/variants/<Board>/ldscript.ld
```

The script dispatches off the env's `board` setting to pick the right OpenOCD target, the per-board bootloader binary from `variants/<Board>/bootloader.bin`, and the per-board app flash address (`0x0800A000` for L4, `0x08020000` for H7). Works on all three boards.

For a standalone app build (no bootloader), use the matching `ldscript-no-bootloader.ld` and omit `extra_scripts`.

## Package contents

```
boards/                              Board JSON definitions
variants/
  MicroNode/
    bootloader.bin                   Pre-compiled bootloader (STM32L431)
    ldscript.ld                      Bootloader-aware linker script (app @ 0x0800A000)
    ldscript-no-bootloader.ld        Standalone linker script (app @ 0x08000000)
    (pin map, peripheral config)
  MicroNodePlus/                     same layout (STM32H723)
  CoreNode/                          same layout (STM32H743)
upload_bootloader_app.py             PlatformIO extra_script for two-stage bootloader flash
```

The bootloader binaries are built from [BR_bootloader](https://github.com/BeyondRobotix/BR_bootloader) and refreshed via `tools/distribute_bins.py` in the mono root.
