# rm winamp_visual.cpython-311-x86_64-linux-gnu.so; python build_c_module_for_python.py build --build-lib=. && LD_LIBRARY_PATH=src/libprojectM python test_winamp_visual.py


# full
# rm CMakeCache.txt; cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_SDL_UI=ON && cmake --build . -- -j && rm winamp_visual.cpython-311-x86_64-linux-gnu.so; python build_c_module_for_python.py build --build-lib=. && LD_LIBRARY_PATH=src/libprojectM python test_winamp_visual.py




import sys
import pathlib

this_file_directory = pathlib.Path(__file__).parent.resolve()
sys.path.insert(0, str(this_file_directory))
from helpers import *

import winamp_visual

# winamp_visual.systemcall('echo "systemcall!"')
# winamp_visual.print_from_c('hello!')

winamp_visual.setup_winamp()


preset_path = this_file_directory.joinpath('presets', 'tests', '001-line.milk')
print(f'Python: {preset_path}')
winamp_visual.load_preset(str(preset_path))
winamp_visual.render_frame()