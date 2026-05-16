# Future Work: Proper PlatformIO Platform

## Problem

The current setup requires a one-time bootstrap command on each new machine:

```sh
pio pkg install -g -t "git+https://github.com/BeyondRobotix/br_platformio_hwdef.git"
```

This is necessary because PlatformIO resolves board IDs **before** downloading `platform_packages`.
Since the board JSONs live in our package, they aren't available during board lookup on a fresh clone.

PlatformIO's execution order for `pio run`:
1. Read `platformio.ini`
2. **Resolve board ID** (searches `boards_dir`, `~/.platformio/boards/`, platform boards)
3. Download/install `platform_packages` and other packages
4. Run SCons build

`platform_packages` is step 3; board lookup is step 2 — wrong order for our use case.

---

## The Clean Solution

Make `br_platformio_hwdef` a proper **PlatformIO platform** (like
[earlephilhower/arduino-pico](https://github.com/earlephilhower/arduino-pico) or
[platform-ststm32](https://github.com/platformio/platform-ststm32)).

When using `platform = https://github.com/BeyondRobotix/br_platformio_hwdef.git`, the platform
repo is downloaded **first** (before board lookup), so boards in its `boards/` directory are found
automatically. No separate install step.

`platformio.ini` would simplify to:

```ini
[env:Micro-Node-Bootloader]
platform = https://github.com/BeyondRobotix/br_platformio_hwdef.git
board = MicroNode
framework = arduino
```

---

## Files Required

### `platform.json`
Platform manifest. Trim ststm32's `platform.json` to the packages we actually use:
- `toolchain-gccarmnoneeabi`
- `framework-arduinoststm32`
- `framework-cmsis`
- `tool-openocd`
- `tool-stm32duino` (optional)
- `tool-jlink` (optional)

Set `frameworks.arduino.script` to point at our `builder/frameworks/arduino.py`.

### `platform.py`
Extend ststm32's `Ststm32Platform` class to inherit:
- `configure_default_packages()` — pins correct toolchain/framework versions per MCU
- `get_boards()` + `_add_default_debug_tools()` — wires up stlink/jlink debug config
- `configure_debug_session()` — speed settings

ststm32's `platform.py` is at `~/.platformio/platforms/ststm32/platform.py`. Since both
platforms live under `~/.platformio/platforms/`, our `platform.py` can locate it via:

```python
ststm32_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ststm32")
```

Or add `platform-ststm32` as a package dependency in `platform.json` so it's guaranteed present.

### `builder/frameworks/arduino.py`
ststm32's equivalent is only ~50 lines — it finds the framework package and runs its build script.
Ours would be almost identical, with one addition: inject `build.variants_dir` pointing at our
platform's `variants/` before handing off.

```python
import sys
from os.path import join, isfile
from SCons.Script import DefaultEnvironment, SConscript

env = DefaultEnvironment()
platform_dir = env.PioPlatform().get_dir()
board_config = env.BoardConfig()

# Default variants_dir to this platform's variants/ unless overridden in platformio.ini
if not board_config.get("build.variants_dir", ""):
    board_config["build"]["variants_dir"] = join(platform_dir, "variants")

build_script = join(
    env.PioPlatform().get_package_dir("framework-arduinoststm32"),
    "tools", "platformio", "platformio-build.py"
)
SConscript(build_script)
```

---

## Key Findings from Investigation

### Where variant handling lives
Not in the platform — in the **framework package**:
`framework-arduinoststm32/tools/platformio/platformio-build.py` lines 48–53:

```python
variants_dir = (
    join(env.subst("$PROJECT_DIR"), board_config.get("build.variants_dir"))
    if board_config.get("build.variants_dir", "")
    else join(FRAMEWORK_DIR, "variants")
)
```

When `board_build.variants_dir` is set in `platformio.ini` it's treated as **relative to
`$PROJECT_DIR`** — which is why absolute paths are needed, or the builder must inject the value
before `platformio-build.py` runs.

### upload_bootloader_app.py path
With the platform approach, reference it via:
```ini
extra_scripts = ${platformio.home_dir}/platforms/br-stm32/upload_bootloader_app.py
```
(where `br-stm32` matches the `"name"` field in `platform.json`).
`upload_bootloader_app.py` already uses `os.path.dirname(__file__)` to locate `MicroNodeBootloader.bin`,
so it will work correctly from the platform directory.

---

## Implementation Checklist

- [ ] Write `platform.json` (trimmed from ststm32's)
- [ ] Write `platform.py` extending `Ststm32Platform`
- [ ] Write `builder/frameworks/arduino.py` with variant injection + framework delegation
- [ ] Update `Arduino-DroneCAN/platformio.ini`: `platform = https://...`, remove `platform_packages` and `boards_dir`
- [ ] Verify `board_build.ldscript` paths work (absolute or injected by builder)
- [ ] Test all four build environments
