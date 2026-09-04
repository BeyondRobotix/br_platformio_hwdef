# br_platformio_hwdef

PlatformIO platform (`br-stm32`) providing board definitions, variants, linker scripts, and bundled bootloader binaries for Beyond Robotix STM32 hardware.

## Supported boards

| Board | MCU | Clock | Flash |
|-------|-----|-------|-------|
| MicroNode | STM32L431 | 80 MHz | 256 KB |
| CoreNode | STM32H743 | 480 MHz | 2 MB |
| MicroNodePlus | STM32H723 | 550 MHz | 1 MB |
| MicroNodeC5 | STM32C532 | 144 MHz | 256 KB |

> **MicroNodeC5 is unverified.** It targets STM32C5's new "HAL2" driver
> generation, has no real Beyond Robotix schematic yet (pin choices are
> placeholders), and has not been build- or hardware-tested. See
> `variants/MicroNodeC5/` file comments for details before relying on it.

## Usage

Reference the platform from your `platformio.ini`. PlatformIO downloads it on first build — nothing to install.

```ini
[env:my-env]
platform = https://github.com/BeyondRobotix/br_platformio_hwdef.git
board = MicroNode       ; or CoreNode, MicroNodePlus
framework = arduino
board_build.ldscript = ldscript.ld
```
### Bootloader-aware envs (two-stage flash)

Any env whose name ends in `-Bootloader` (and does not contain `No-Bootloader`) using a bare ldscript automatically gets a two-stage flash on `pio run -t upload`: first the bundled `variants/<board>/bootloader.bin` at `0x08000000`, then the compiled app at the board's app-start address. No `extra_scripts` entry needed.

```ini
[env:Micro-Node-App]
platform = https://github.com/BeyondRobotix/br_platformio_hwdef.git
board = MicroNode
framework = arduino
board_build.ldscript = ldscript.ld
```

## Refreshing the cached platform

PlatformIO caches the platform locally after the first fetch. To pick up new changes to this repo:

```sh
pio pkg uninstall --platform br-stm32 -g
```

…or delete the platform directory directly:

- Linux/macOS: `~/.platformio/platforms/br-stm32/`
- Windows: `%USERPROFILE%\.platformio\platforms\br-stm32\`

The next `pio run` re-fetches from GitHub.

