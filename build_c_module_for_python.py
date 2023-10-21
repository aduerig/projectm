# called by: python .\driver\build_c_module_for_python.py build --compiler=mingw32

from setuptools import setup, Extension
import sys
import pathlib

this_file_directory = pathlib.Path(__file__).parent.resolve()
root_of_project_directory = this_file_directory.parent

sys.path.insert(0, str(root_of_project_directory))
from helpers import *

release_mode = 'release'
if '--release' in sys.argv:
    del sys.argv[sys.argv.index('--release')]
if '--debug' in sys.argv:
    release_mode = 'debug'
    del sys.argv[sys.argv.index('--debug')]

print_cyan(f'building with {release_mode=}, {root_of_project_directory=}, {this_file_directory=}')

lib_dir = root_of_project_directory.joinpath('lib')
extra_link_args = [str(lib_dir.joinpath('libShumiChess.a'))]
extra_compile_args=['-std=c++17']

if release_mode == 'debug':
    extra_compile_args += ['-g', '-O0']
else:
    extra_compile_args += ['-Ofast']

include_dirs = [str(root_of_project_directory.joinpath('include'))]
library_dirs = [str(root_of_project_directory.joinpath('src'))]



the_module = Extension(
    'winamp_visual',
    sources = [str(this_file_directory.joinpath('winamp_visual.cpp'))],
    include_dirs=include_dirs,
    library_dirs=library_dirs,
    # tries to do a .so (dynamic) build with this
    # libraries = ['ShumiChess'],
    extra_compile_args=extra_compile_args,
    extra_link_args=extra_link_args,
)

setup(
    name = 'winamp_visual',
    version = '1.0',
    ext_modules = [the_module]
)
