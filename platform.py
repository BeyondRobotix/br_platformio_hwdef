import sys

try:
    from platformio.public import PlatformBase
except ImportError:
    from platformio.managers.platform import PlatformBase


IS_WINDOWS = sys.platform.startswith("win")


class Brstm32Platform(PlatformBase):

    def configure_default_packages(self, variables, targets):
        frameworks = variables.get("pioframework", [])
        if "arduino" in frameworks:
            self.packages["toolchain-gccarmnoneeabi"]["version"] = "~1.120301.0"
            self.packages["framework-cmsis"]["version"] = "~2.60300.0"
            self.packages["framework-cmsis"]["optional"] = False
            self.packages["framework-cmsis-dsp"]["optional"] = False
        return super().configure_default_packages(variables, targets)

    def get_boards(self, id_=None):
        result = PlatformBase.get_boards(self, id_)
        if not result:
            return result
        if id_:
            return self._add_default_debug_tools(result)
        for key in result:
            result[key] = self._add_default_debug_tools(result[key])
        return result

    def _add_default_debug_tools(self, board):
        # Injects debug.tools.<link>.server.arguments entries that the boards'
        # JSONs don't carry directly.  Without this, main.py's stlink/openocd
        # upload path can't find the openocd argument list and the upload (and
        # debug) fails with a None lookup.  Trimmed copy of platform-ststm32's
        # equivalent method -- only the link types our boards actually use.
        debug = board.manifest.get("debug", {})
        upload_protocols = board.manifest.get("upload", {}).get("protocols", [])
        if "tools" not in debug:
            debug["tools"] = {}

        for link in ("blackmagic", "jlink", "stlink", "cmsis-dap"):
            if link not in upload_protocols or link in debug["tools"]:
                continue
            if link == "blackmagic":
                debug["tools"]["blackmagic"] = {
                    "hwids": [["0x1d50", "0x6018"]],
                    "require_debug_port": True,
                }
            elif link == "jlink":
                assert debug.get("jlink_device"), (
                    "Missed J-Link Device ID for %s" % board.id
                )
                debug["tools"][link] = {
                    "server": {
                        "package": "tool-jlink",
                        "arguments": [
                            "-singlerun",
                            "-if", "SWD",
                            "-select", "USB",
                            "-device", debug.get("jlink_device"),
                            "-port", "2331",
                        ],
                        "executable": (
                            "JLinkGDBServerCL.exe" if IS_WINDOWS else "JLinkGDBServer"
                        ),
                    }
                }
            else:
                # stlink, cmsis-dap -- both driven through openocd
                server_args = ["-s", "$PACKAGE_DIR/openocd/scripts"]
                if debug.get("openocd_board"):
                    server_args.extend(
                        ["-f", "board/%s.cfg" % debug.get("openocd_board")]
                    )
                else:
                    assert debug.get("openocd_target"), (
                        "Missed target configuration for %s" % board.id
                    )
                    server_args.extend([
                        "-f", "interface/%s.cfg" % link,
                        "-c", "transport select %s" % (
                            "hla_swd" if link == "stlink" else "swd"
                        ),
                        "-f", "target/%s.cfg" % debug.get("openocd_target"),
                    ])
                    server_args.extend(debug.get("openocd_extra_args", []))

                debug["tools"][link] = {
                    "server": {
                        "package": "tool-openocd",
                        "executable": "bin/openocd",
                        "arguments": server_args,
                    }
                }
            debug["tools"][link]["onboard"] = link in debug.get("onboard_tools", [])
            debug["tools"][link]["default"] = link in debug.get("default_tools", [])

        board.manifest["debug"] = debug
        return board
