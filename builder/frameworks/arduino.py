import os
import sys
from os.path import join, isfile

from SCons.Script import DefaultEnvironment, SConscript, Export

env = DefaultEnvironment()
platform_dir = env.PioPlatform().get_dir()
board_config = env.BoardConfig()
variant = board_config.get("build.variant", "")

# Inject variants_dir → this platform's variants/ directory, unless the
# consumer has already set board_build.variants_dir to something custom.
if not board_config.get("build.variants_dir", ""):
    board_config.update("build.variants_dir", join(platform_dir, "variants"))

# Resolve a bare ldscript filename (no path separator) to the absolute path
# inside this platform's variant directory, so consumers can write just
# "ldscript.ld" or "ldscript-no-bootloader.ld" without knowing the platform
# install path.  Paths that contain a separator (e.g. "linker/bootloader_l4.ld")
# are left untouched so BR_bootloader's project-local linker scripts continue
# to resolve relative to the project directory.
#
# We also have to refresh LDSCRIPT_PATH because PlatformIO core
# (LoadPioPlatform) already copied the raw board_build.ldscript value into
# the SCons env before this framework script ran -- updating only
# board_config wouldn't reach the linker.
ldscript = board_config.get("build.ldscript", "")
if ldscript and os.sep not in ldscript and "/" not in ldscript:
    abs_ldscript = join(platform_dir, "variants", variant, ldscript)
    board_config.update("build.ldscript", abs_ldscript)
    env.Replace(LDSCRIPT_PATH=abs_ldscript)

# Delegate to framework-arduinoststm32's build script.
build_script = join(
    env.PioPlatform().get_package_dir("framework-arduinoststm32"),
    "tools", "platformio", "platformio-build.py"
)
if not isfile(build_script):
    sys.stderr.write("Error: Missing framework build script %s!\n" % build_script)
    env.Exit(1)

SConscript(build_script)

# Auto-register the two-stage bootloader+app upload for any env whose name
# contains "Bootloader".  This replaces the explicit extra_scripts entry that
# consumers used to need in their platformio.ini.
pioenv = env.subst("$PIOENV")
if "Bootloader" in pioenv:
    upload_script = join(platform_dir, "upload_bootloader_app.py")
    if isfile(upload_script):
        Export("env")
        env.SConscript(upload_script)
