# rm winamp_visual.cpython-311-x86_64-linux-gnu.so; python build_c_module_for_python.py build --build-lib=. && LD_LIBRARY_PATH=src/libprojectM python test_winamp_visual.py


# full
# rm CMakeCache.txt; cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_SDL_UI=ON && cmake --build . -- -j && rm winamp_visual.cpython-311-x86_64-linux-gnu.so; python build_c_module_for_python.py build --build-lib=. && LD_LIBRARY_PATH=src/libprojectM python test_winamp_visual.py

# must be on 311..., not b311



import sys
import time
import numpy as np
import pathlib

this_file_directory = pathlib.Path(__file__).parent.resolve()
sys.path.insert(0, str(this_file_directory))
from helpers import *

# print('before import')
import winamp_visual
# print('after import')

# winamp_visual.print_from_c()

winamp_visual.setup_winamp()


preset_path = this_file_directory.joinpath('presets', 'tests', '001-line.milk')
print(f'Python: {preset_path}')
winamp_visual.load_preset(str(preset_path))



GRID_WIDTH = 20
GRID_HEIGHT = 32
grid = np.array(np.zeros((GRID_WIDTH, GRID_HEIGHT, 3)), np.double)

def print_grid_to_terminal():
    # to_print_grid = grid * 2.55
    to_print_grid = grid
    to_print_grid = to_print_grid.astype(int)
    [print(''.join(f'\033[38;2;{rgb[0]};{rgb[1]};{rgb[2]}m▆\033[0m' for rgb in to_print_grid[x])) for x in range(GRID_WIDTH)]
    print('\033[F' * GRID_WIDTH, end='')


# runs 24 times a second
while True:
    winamp_visual.render_frame()
    winamp_visual.load_into_numpy_array(grid)
    print_grid_to_terminal()
    time.sleep(1/24)
