"""
Hatch Build Hook to compile the rpi-rgb-led-matrix library.
"""
from hatchling.plugin import hookimpl
from hatchling.builders.hooks.plugin.interface import BuildHookInterface

import os, subprocess
import pathlib

@hookimpl
def hatch_register_build_hook():
    """
    Register the build hook.
    :return:
    """
    return PyRGBMatrixBuildHook

class PyRGBMatrixBuildHook(BuildHookInterface):
    PLUGIN_NAME = "pyrgbmatrix"

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def clean(self, *args, **kwargs):
        self.app.display_info("Running make clean...")
        clean_process = subprocess.Popen('make clean', shell=True)
        stdout, stderr = clean_process.communicate()
        if stderr:
            self.app.display_warning("Make clean encountered error.")

    def initialize(self, _: str, build_data: dict):
        """
        Initialize the build, fetch the repo, determine version and compile.
        :param _:
        :param build_data:
        :return:
        """

        self.app.display("Building rpi-rgb-led-matrix library...")
        make_process = subprocess.Popen('make build-python HARDWARE_DESC="adafruit-hat-pwm"', shell=True)
        stdout, stderr = make_process.communicate()
        if stderr:
            raise BaseException("Error occurred during build.")
