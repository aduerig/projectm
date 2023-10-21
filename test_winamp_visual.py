# python build_c_module_for_python.py build --build-lib=. && python test_winamp_visual.py


import sys
import pathlib

this_file_directory = pathlib.Path(__file__).parent.resolve()
sys.path.insert(0, str(this_file_directory))
from helpers import *


# ok this is a hack currently i think i should just compile statically?
import ctypes
ctypes.CDLL(str(this_file_directory.joinpath('libprojectM-4.so.4')))
ctypes.CDLL(str(this_file_directory.joinpath('libprojectM-4d.so.4')))

import winamp_visual

winamp_visual.systemcall('echo "systemcall!"')
winamp_visual.print_from_c('hello!')
winamp_visual.setup_winamp()