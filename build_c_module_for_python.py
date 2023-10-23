# python build_c_module_for_python.py build --build-lib=.
    # puts so in current directory
# windows?
    # called by: python .\driver\build_c_module_for_python.py build --compiler=mingw32

from setuptools import setup, Extension
import sys
import pathlib

this_file_directory = pathlib.Path(__file__).parent.resolve()

sys.path.insert(0, str(this_file_directory))
from helpers import *

release_mode = 'release'
# if '--debug' in sys.argv:
#     del sys.argv[sys.argv.index('--debug')]
# if '--release' in sys.argv:
#     release_mode = 'release'
#     del sys.argv[sys.argv.index('--release')]

print_cyan(f'building with {release_mode=}, {this_file_directory=}')

extra_link_args = []
src_folder = this_file_directory.joinpath('src')
src_libprojectM_folder = src_folder.joinpath('libprojectM')

# for copying but just using LD_LIBRARY_PATH for now
# for _, path in get_all_paths(src_libprojectM_folder):
#     if '.so' in path.name:
#     # if '.a' in path.name:
#         final_lib_path = this_file_directory.joinpath(path.name)
#         shutil.copy(path, final_lib_path)
#         extra_link_args.append(str(final_lib_path))

vendor_folder = this_file_directory.joinpath('vendor')
# glm_folder = vendor_folder.joinpath('glm')

extra_compile_args=['-std=c++17']
if release_mode == 'debug':
    extra_compile_args += ['-g', '-O0']
# else:
#     extra_compile_args += ['-Ofast']

include_dir_api_1 = this_file_directory.joinpath('src', 'api', 'include')
include_dir_api_2 = this_file_directory.joinpath('src', 'playlist', 'api')
include_dir_api_3 = this_file_directory.joinpath('src', 'playlist', 'include')
include_dir_api_4 = this_file_directory.joinpath('src', 'api', 'include', 'projectM-4')

include_dirs = [
    str(src_folder),
    str(src_libprojectM_folder),
    str(vendor_folder), # for glm
    str(include_dir_api_1),
    str(include_dir_api_2),
    str(include_dir_api_3),
    str(include_dir_api_4),
]
library_dirs = [
    str(src_libprojectM_folder),
]

sources = [
    str(this_file_directory.joinpath('winamp_visualmodule.cpp')),
    str(this_file_directory.joinpath('my_sdl.cpp')),
]

the_module = Extension(
    'winamp_visual',
    sources=sources,
    include_dirs=include_dirs,
    library_dirs=library_dirs,
    # tries to do a .so (dynamic) build with this
    libraries = ['projectM-4'],
    # remove if shared
    # libraries=['GL'],
    extra_compile_args=extra_compile_args,
    extra_link_args=extra_link_args,
)

setup(
    name = 'winamp_visual',
    version = '1.0',
    ext_modules = [the_module]
)
