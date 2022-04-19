# coding=utf-8
# Copyright 2019 Google LLC
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Install Google Research Football."""

import os
import sys
import glob
import platform
import shutil
from setuptools import find_packages
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


class CMakeExtension(Extension):

  def __init__(self, name):
    # don't invoke the original build_ext for this special extension
    sources = ['third_party/gfootball_engine/src/misc/empty.cpp']
    super().__init__(name, sources=sources, optional=True)


class CustomBuild(build_ext):
  """Custom installation script to build the C++ environment."""

  def run(self):
    if platform.system() == 'Windows':
      self.run_windows()
    else:
      self.run_unix()
    super(CustomBuild, self).run()

  def run_unix(self):
    if os.path.exists(self.build_lib):
      dest_dir = os.path.join(self.build_lib, 'gfootball_engine')
    else:
      # For the development install (pip install -e .)
      # gfootball_engine module has to be located in the project root directory.
      dest_dir = "gfootball_engine"
      if not os.path.exists(dest_dir):
        try:
          os.symlink(os.path.abspath('third_party/gfootball_engine'), dest_dir)
        except:
          raise OSError("Google Research Football: Could not create symlink to %s"
                        "for the development install." % dest_dir)

    try:
      use_prebuilt_lib = int(os.environ.get('GFOOTBALL_USE_PREBUILT_SO', '0'))
    except ValueError:
      raise ValueError('Could not parse GFOOTBALL_USE_PREBUILT_SO environment '
                       'variable as int. Please set it to 0 or 1')

    if use_prebuilt_lib:
      if os.system(
            'cp third_party/gfootball_engine/lib/prebuilt_gameplayfootball.so ' +
            dest_dir + '/_gameplayfootball.so'):
        raise OSError(
            'Failed to copy pre-built library to a final destination %s.' %
            dest_dir)
    else:
      # Compile the engine
      if os.system('gfootball/build_game_engine.sh'):
        raise OSError('Google Research Football compilation failed')
      # There might be multiple compiled modules (e.g. for different python versions)
      # Copy them all
      libs = glob.glob(f'third_party/gfootball_engine/_gameplayfootball*.so')
      copy_compiled_libs(libs, dest_dir)
    copy_fonts(dest_dir)

  def run_windows(self):
    guide_message = 'Please follow the guide on how to install prerequisites: ' \
                    'https://github.com/google-research/football/blob/master/gfootball/doc/compile_engine.md#windows'
    if not os.environ.get('VCPKG_ROOT'):
      raise OSError('VCPKG_ROOT environment variable is not defined.\n' + guide_message)

    if os.path.exists(self.build_lib):
      dest_dir = os.path.join(self.build_lib, 'gfootball_engine')
    else:
      # For the development install (pip install -e .)
      # gfootball_engine module has to be located in the project root directory.
      dest_dir = "gfootball_engine"
      if not os.path.exists(dest_dir):
        try:
          os.symlink(os.path.abspath('third_party/gfootball_engine'), dest_dir, target_is_directory=True)
        except OSError:
          # Windows doesn't support symlinks for unprivileged users
          # Fall back to copying the files
          os.mkdir(dest_dir)
          shutil.copy2('third_party/gfootball_engine/__init__.py', dest_dir)
          data_dir = os.path.join(dest_dir, 'data')
          if not os.path.exists(data_dir):
            shutil.copytree('third_party/gfootball_engine/data', data_dir)

    os.environ['GENERATOR_PLATFORM'] = 'x64' if sys.maxsize > 2 ** 32 else 'Win32'
    py_major, py_minor, _ = platform.python_version_tuple()
    os.environ['PY_VERSION'] = f'{py_major}.{py_minor}'
    if os.system('gfootball\\build_game_engine.bat'):
      raise OSError('Google Research Football compilation failed.\n' + guide_message)
    # Copy compiled library and its dependencies
    lib_path = 'third_party/gfootball_engine/build_win/Release/'
    libs = glob.glob(f'{lib_path}*.pyd') + glob.glob(f'{lib_path}*.dll')
    copy_compiled_libs(libs, dest_dir)
    copy_fonts(dest_dir)


def copy_compiled_libs(libs, dest_dir):
  """Copy compiled libraries to the destination directory."""
  for lib in libs:
    try:
      shutil.copy2(lib, dest_dir)
    except shutil.SameFileError:
      # In case the file is the same do nothing
      pass


def copy_fonts(dest_dir):
  """Copy fonts to the destination directory."""
  dst_fonts = os.path.join(dest_dir, "fonts")
  if not os.path.exists(dst_fonts):
    shutil.copytree("third_party/fonts", dst_fonts)


def process_develop_setup():
  """
  Clean up (if necessary) some directories before or after running
  setup in development (a.k.a. editable) mode (`pip install -e .`).
  """
  if 'develop' in sys.argv and os.path.exists('build'):
    # Remove `build` directory created by a regular installation
    shutil.rmtree('build')
  elif 'develop' not in sys.argv and os.path.exists('gfootball_engine'):
    # If `pip install .` is called after development mode,
    # remove the 'fonts' directory copied by a `develop` setup
    copied_fonts = 'third_party/gfootball_engine/fonts'
    if os.path.exists(copied_fonts):
      shutil.rmtree(copied_fonts)
    # Remove .so files (.pyd on Windows)
    for empty_lib in glob.glob("brainball_cpp_engine*"):
      os.remove(empty_lib)
    # Finally, remove symlink to the gfootball_engine directory
    if not os.path.exists('gfootball_engine'):
      return
    if os.path.islink('gfootball_engine'):
      if platform.system() == 'Windows':
        os.remove('gfootball_engine')
      else:
        os.unlink('gfootball_engine')
    else:
      shutil.rmtree('gfootball_engine')


# TODO: Add CI tests for develop setup on all platforms
process_develop_setup()
packages = find_packages() + find_packages('third_party')

setup(
    name='gfootball',
    version='2.10.3',
    description=('Google Research Football - RL environment based on '
                 'open-source game Gameplay Football'),
    long_description=('Please see [our GitHub page](https://github.com/google-research/football) '
                      'for details.'),
    long_description_content_type='text/markdown',
    author='Google LLC',
    author_email='no-reply@google.com',
    url='https://github.com/google-research/football',
    license='Apache 2.0',
    packages=packages,
    package_dir={'gfootball_engine': 'third_party/gfootball_engine'},
    # If you change the requirements here please don't forget to change the requirements.txt too
    install_requires=[
        'pygame>=1.9.6',
        'opencv-python',
        'psutil',
        'numpy',
        'gym<=0.21.0',
        'absl-py',
        'wheel',
    ],
    include_package_data=True,
    keywords='gfootball reinforcement-learning python machine learning',
    ext_modules=[CMakeExtension('brainball_cpp_engine')],
    cmdclass={'build_ext': CustomBuild},
)
