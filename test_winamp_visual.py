# no build
    # LD_LIBRARY_PATH=src/libprojectM python test_winamp_visual.py

# partial 
    # rm winamp_visual.cpython-311-x86_64-linux-gnu.so; python build_c_module_for_python.py build --build-lib=. && LD_LIBRARY_PATH=src/libprojectM python test_winamp_visual.py

# full
    # rm CMakeCache.txt; cmake -DCMAKE_BUILD_TYPE=Release && cmake --build . -- -j && rm winamp_visual.cpython-311-x86_64-linux-gnu.so; python build_c_module_for_python.py build --build-lib=. && LD_LIBRARY_PATH=src/libprojectM python test_winamp_visual.py

# must be on 311..., not b311


# FOR RASPERRY PI:
    # RUNNING ONLY:
        # LD_LIBRARY_PATH=src/libprojectM:/home/pi/random/sdl_install/SDL-release-2.28.4/build/.libs/:/usr/lib/aarch64-linux-gnu python test_winamp_visual.py
    # PARTIAL BUILD:
        # rm winamp_visual.cpython-311-x86_64-linux-gnu.so; python build_c_module_for_python.py build --build-lib=. && LD_LIBRARY_PATH=src/libprojectM:/home/pi/random/sdl_install/SDL-release-2.28.4/build/.libs/:/usr/lib/aarch64-linux-gnu python test_winamp_visual.py
    # FULL BUILD:
        # rm CMakeCache.txt; cmake -DCMAKE_BUILD_TYPE=Release && cmake --build . -- -j4 && rm winamp_visual.cpython-311-x86_64-linux-gnu.so; python build_c_module_for_python.py build --build-lib=. && LD_LIBRARY_PATH=src/libprojectM:/home/pi/random/sdl_install/SDL-release-2.28.4/build/.libs/ python test_winamp_visual.py



import sys
import time
import pathlib
import random
import threading
import collections

import numpy as np


this_file_directory = pathlib.Path(__file__).parent.resolve()
sys.path.insert(0, str(this_file_directory))
from helpers import *

import winamp_visual
winamp_visual.setup_winamp()



keys_to_proccess = collections.deque([])


def start_listen_keys():
    if is_andrews_main_computer():
        from pynput.keyboard import Listener, KeyCode

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
                keys_to_proccess.append(key_name)

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

        threading.Thread(target=listen_for_keystrokes, args=[], daemon=True).start()
    elif is_doorbell():
        print_red('NOT LISTENING FOR KEYSTROKES, NEED TO IMPLEMENT THIS')


preset_history = collections.deque([])
preset_index = -1
def last_preset():
    global preset_index
    if preset_index <= 0:
        return
    preset_index -= 1
    preset_path = preset_history[preset_index]
    print(f'Preset index is at {preset_index}/{len(preset_history) - 1} now')
    load_preset(preset_path)


def next_preset():
    global preset_index
    if preset_index >= len(preset_history) - 1:
        return
    preset_index += 1
    preset_path = preset_history[preset_index]
    print(f'Preset index is at {preset_index}/{len(preset_history) - 1} now')
    load_preset(preset_path)


def load_preset(preset_path):
    better_print = preset_path.relative_to(presets_directory)
    better_print = better_print.relative_to(better_print.parts[0])
    print_blue(f'Python: loading preset {better_print}')
    winamp_visual.load_preset(str(preset_path))
# load_preset(presets_directory.joinpath('tests', '001-line.milk'))


presets_directory = this_file_directory.joinpath('presets')
all_presets = list(get_all_paths(presets_directory, recursive=True, only_files=True, allowed_extensions=['.milk']))
print_green(f'{len(all_presets):,} milk visualizer presets to choose from')
def random_preset():
    global preset_index
    preset_path = random.choice(all_presets)[1]

    preset_history.append(preset_path)
    preset_index = len(preset_history) - 1
    print(f'Python: randomly loading preset, preset index at {preset_index}/{len(preset_history) - 1} now')

    load_preset(preset_path)



grid = np.array(np.zeros((20, 32, 3)), np.double)
def print_grid_to_terminal():
    to_print_grid = (grid * 2.55).astype(int)
    [print(''.join(f'\033[38;2;{rgb[0]};{rgb[1]};{rgb[2]}m▆\033[0m' for rgb in to_print_grid[x])) for x in range(20)]
    print('\033[F' * 20, end='')


def increase_beat_sensitivity():
    winamp_visual.set_beat_sensitivity(winamp_visual.get_beat_sensitivity() + .01)
    print(f'beat sensitivity: {winamp_visual.get_beat_sensitivity()}')

def decrease_beat_sensitivity():
    winamp_visual.set_beat_sensitivity(winamp_visual.get_beat_sensitivity() - .01)
    print(f'beat sensitivity: {winamp_visual.get_beat_sensitivity()}')


keyboard_dict = {
    'r': lambda: random_preset(),
    'b': lambda: print(winamp_visual.get_beat_sensitivity()),
    'up': lambda: increase_beat_sensitivity(),
    'down': lambda: decrease_beat_sensitivity(),

    'left': lambda: last_preset(),
    'right': lambda: next_preset(),
    # 'left': lambda: restart_show(skip=-skip_time),
    # 'right': lambda: restart_show(skip=skip_time),
    # 'space': 'UV',
}

start_listen_keys()
exit()
while True:
    if len(keys_to_proccess) > 0:
        key = keys_to_proccess.popleft()
        if key in keyboard_dict:
            keyboard_dict[key]()
        else:
            print_red(f'Python: unknown key {key}')
    winamp_visual.render_frame()
    winamp_visual.load_into_numpy_array(grid)
    print_grid_to_terminal()

    # winamp_visual.print_to_terminal_higher_level()
    time.sleep(1/24)
