# no build
    # LD_LIBRARY_PATH=src/libprojectM python test_winamp_visual.py

# partial 
    # rm winamp_visual.cpython-311-x86_64-linux-gnu.so; python build_c_module_for_python.py build --build-lib=. && LD_LIBRARY_PATH=src/libprojectM python test_winamp_visual.py

# full
    # rm CMakeCache.txt; cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_SDL_UI=ON && cmake --build . -- -j && rm winamp_visual.cpython-311-x86_64-linux-gnu.so; python build_c_module_for_python.py build --build-lib=. && LD_LIBRARY_PATH=src/libprojectM python test_winamp_visual.py

# must be on 311..., not b311



import sys
import time
import pathlib
import random
import threading

import numpy as np
from pynput.keyboard import Listener, KeyCode


this_file_directory = pathlib.Path(__file__).parent.resolve()
sys.path.insert(0, str(this_file_directory))
from helpers import *

import winamp_visual
winamp_visual.setup_winamp()



_return_code, stdout, _stderr = run_command_blocking([
    'xdotool',
    'getactivewindow',
])
process_window_id = int(stdout.strip())

# https://stackoverflow.com/questions/24072790/how-to-detect-key-presses how to check window name (not global)
def window_focus():
    return_code, stdout, _stderr = run_command_blocking([
        'xdotool',
        'getwindowfocus',
    ])
    if return_code != 0:
        return False
    other = int(stdout.strip())
    return process_window_id == other

def on_press(key):
    if not window_focus():
        return
    if type(key) == KeyCode:
        key_name = key.char
    else:
        key_name = key.name
    if key_name in keyboard_dict:
        keyboard_dict[key_name]()

def on_release(key):
    if not window_focus():
        return
    if type(key) == KeyCode:
        key_name = key.char
    else:
        key_name = key.name


def listen_for_keystrokes():
    with Listener(on_press=on_press, on_release=on_release) as listener:
        listener.join()




def load_preset(preset_path):
    print(f'Python: loading {preset_path}')
    winamp_visual.load_preset(str(preset_path))

# load_preset(presets_directory.joinpath('tests', '001-line.milk'))


presets_directory = this_file_directory.joinpath('presets')
all_presets = list(get_all_paths(presets_directory, recursive=True, only_files=True, allowed_extensions=['.milk']))

print_green(f'{len(all_presets):,} milk visualizer presets to choose from')

def random_preset():
    preset_path = random.choice(all_presets)
    load_preset(preset_path)



grid = np.array(np.zeros((20, 32, 3)), np.double)
def print_grid_to_terminal():
    to_print_grid = grid.astype(int)
    [print(''.join(f'\033[38;2;{rgb[0]};{rgb[1]};{rgb[2]}m▆\033[0m' for rgb in to_print_grid[x])) for x in range(20)]
    print('\033[F' * 20, end='')

keyboard_dict = {
    'r': lambda: random_preset(),
    # 'down': lambda: restart_show(skip=-2),
    # 'left': lambda: restart_show(skip=-skip_time),
    # 'right': lambda: restart_show(skip=skip_time),
    # 'space': 'UV',
}
threading.Thread(target=listen_for_keystrokes, args=[], daemon=True).start()


index = 0
while True:
    winamp_visual.render_frame()
    print(f'{index}: BEFORE LOAD: {np.isnan(grid).any()=}')
    winamp_visual.load_into_numpy_array(grid)
    print(f'{index}: AFTER LOAD: {np.isnan(grid).any()=}')
    print_grid_to_terminal()
    # winamp_visual.print_to_terminal_higher_level()
    time.sleep(1/24)
    index += 1
