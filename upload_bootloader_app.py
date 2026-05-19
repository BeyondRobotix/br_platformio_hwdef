"""
PlatformIO post-script wired into the *-Bootloader app envs.

When the user runs `pio run -e <board>-Bootloader -t upload`, this script
takes over the upload step and flashes both:
  1. The bootloader binary shipped with this br-boards package, at
     0x08000000 (always re-flashed -- guarantees the bootloader on the chip
     matches the version distributed alongside this hwdef package).
  2. The app binary just built, at the board-specific app start address.

Per-board parameters (OpenOCD target, app flash address, bootloader bin
location) are dispatched off the PIO `board` option so a single script
covers L4 (MicroNode) and H7 (MicroNodePlus, CoreNode) without
duplication.

Adding a new board: extend the BOARDS table.
"""

Import("env")

import os
import sys
import subprocess

# board -> (openocd target cfg, app flash address, bootloader bin filename
#           within variants/<variant>/)
BOARDS = {
    "MicroNode":     ("target/stm32l4x.cfg", "0x0800A000", "MicroNode/bootloader.bin"),
    "MicroNodePlus": ("target/stm32h7x.cfg", "0x08020000", "MicroNodePlus/bootloader.bin"),
    "CoreNode":      ("target/stm32h7x.cfg", "0x08020000", "CoreNode/bootloader.bin"),
}

board = env.subst("$BOARD")
if board not in BOARDS:
    sys.stderr.write(
        f"\n[upload_bootloader_app] Unknown board '{board}'.\n"
        f"Add it to BOARDS in br_platformio_hwdef/upload_bootloader_app.py.\n\n"
    )
    env.Exit(1)

target_cfg, app_addr, bootloader_rel = BOARDS[board]

# Resolve filesystem paths from the PIO environment.
openocd_exec = "openocd.exe" if sys.platform.startswith("win") else "openocd"
packages_dir = env.subst("$PROJECT_PACKAGES_DIR")
build_dir    = env.subst("$BUILD_DIR")

# Bootloader binary lives alongside this script in the platform's variants/ dir.
platform_dir  = os.path.dirname(os.path.abspath(__file__))
bootloader_path = os.path.join(platform_dir, "variants", bootloader_rel).replace("\\", "/")
firmware_path   = os.path.abspath(os.path.join(build_dir, "firmware.bin")).replace("\\", "/")
elf_path        = os.path.join(build_dir, "firmware.elf")

openocd = os.path.join(packages_dir, "tool-openocd", "bin", openocd_exec)

scripts_dir = os.path.join(packages_dir, "tool-openocd", "openocd", "scripts")
if not os.path.isdir(scripts_dir):
    # Fallback for older tool-openocd package layouts.
    scripts_dir = os.path.join(packages_dir, "tool-openocd", "scripts")

interface_cfg = "interface/stlink.cfg"


def generate_bin(source, target, env):
    """Produce firmware.bin from firmware.elf so the upload step has it."""
    env.Execute(f"$OBJCOPY -O binary {elf_path} {firmware_path}")


env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", generate_bin)


def custom_upload(source, target, env):
    """Flash bootloader then app via two back-to-back openocd invocations."""
    if not os.path.isfile(bootloader_path):
        sys.stderr.write(
            f"\n[upload_bootloader_app] Bootloader binary not found:\n"
            f"  {bootloader_path}\n"
            f"This bin is shipped via the br-boards platform_package. If you've just\n"
            f"updated BR_bootloader, re-run the binary distribution step:\n"
            f"  cd ArduinoDroneCAN_Mono && python tools/distribute_bins.py\n"
            f"then push the br_platformio_hwdef changes so PlatformIO picks up the new\n"
            f"package (or rm -rf $PROJECT_PACKAGES_DIR/br-boards to force a refetch).\n\n"
        )
        env.Exit(1)

    cmd_bootloader = [
        openocd, "-s", scripts_dir,
        "-f", interface_cfg, "-f", target_cfg,
        "-c", f'program "{bootloader_path}" 0x08000000 verify reset exit',
    ]
    cmd_app = [
        openocd, "-s", scripts_dir,
        "-f", interface_cfg, "-f", target_cfg,
        "-c", f'program "{firmware_path}" {app_addr} verify reset exit',
    ]

    try:
        print(f"[upload_bootloader_app] board={board} target={target_cfg} app@{app_addr}")
        print("Flashing bootloader")
        subprocess.run(cmd_bootloader, check=True)
        print("Flashing application firmware")
        subprocess.run(cmd_app, check=True)
    except subprocess.CalledProcessError as e:
        print(f"\n\n--- Upload failed ---\nException: {e}\n---------------------\n")
        env.Exit(1)


env.Replace(UPLOADCMD=custom_upload)
