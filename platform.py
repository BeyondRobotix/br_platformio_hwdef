try:
    from platformio.public import PlatformBase
except ImportError:
    from platformio.managers.platform import PlatformBase


class BrStm32Platform(PlatformBase):

    def configure_default_packages(self, variables, targets):
        frameworks = variables.get("pioframework", [])
        if "arduino" in frameworks:
            self.packages["toolchain-gccarmnoneeabi"]["version"] = "~1.120301.0"
            self.packages["framework-cmsis"]["version"] = "~2.60300.0"
            self.packages["framework-cmsis"]["optional"] = False
        return super().configure_default_packages(variables, targets)
