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
from setuptools.command.install import install
from setuptools.command.build_ext import build_ext

VERSION = '2.10b2'

class CMakeExtension(Extension):

  def __init__(self, name):
    # don't invoke the original build_ext for this special extension
    if platform.system() == 'Windows':
      # TODO: Check if it works the same on Unix. Find better solution?
      super().__init__(name, sources=[''], optional=True)
    else:
      super().__init__(name, sources=[])


class CustomBuild(build_ext):
  """Custom installation script to build the C++ environment."""

  def run(self):
    if platform.system() == 'Windows':
      self.run_windows()
    else:
      self.run_unix()
    super(CustomBuild, self).run()

  def run_unix(self):
    # https://stackoverflow.com/questions/32419594/how-to-create-a-dylib-c-extension-on-mac-os-x-with-distutils-and-or-setuptools
    if sys.platform == 'darwin':
      from distutils import sysconfig
      vars = sysconfig.get_config_vars()
      vars['LDSHARED'] = vars['LDSHARED'].replace('-bundle',
                                                  '-dynamiclib -Wl,-F.')
    if os.path.exists(self.build_lib):
      dest_dir = os.path.join(self.build_lib, 'gfootball_engine')
    else:
      dest_dir = "gfootball_engine"
      # TODO: Is it still required?
      if os.system('cp -r third_party/gfootball_engine/ ' + dest_dir):
        raise OSError("Google Research Football: Could not copy "
                      "engine to %s." % (dest_dir))

    try:
      use_prebuilt_library = int(
          os.environ.get('GFOOTBALL_USE_PREBUILT_SO', '0'))
    except ValueError:
      raise ValueError('Could not parse GFOOTBALL_USE_PREBUILT_SO environment '
                       'variable as int. Please set it to 0 or 1')

    if use_prebuilt_library:
      if os.system(
            'cp third_party/gfootball_engine/lib/prebuilt_gameplayfootball.so ' +
            dest_dir + '/_gameplayfootball.so'):
        raise OSError(
            'Failed to copy pre-built library to a final destination %s.' %
            dest_dir)
    else:
      if (os.system('gfootball/build_game_engine.sh') or
          os.system('cp third_party/gfootball_engine/_gameplayfootball.so ' +
                    dest_dir)):
        raise OSError('Google Research Football compilation failed')

  def run_windows(self):
    try:
      compile_engine = int(os.environ.get('COMPILE_ENGINE', '0'))
    except ValueError:
      raise ValueError('Could not parse COMPILE_ENGINE environment '
                       'variable as int. Please set it to 0 or 1')
    if os.path.exists(self.build_lib):
      dest_dir = os.path.join(self.build_lib, 'gfootball_engine')
    else:
      dest_dir = "gfootball_engine"
    py_major, py_minor, _ = platform.python_version_tuple()

    if not compile_engine:
      # TODO: Automate wheels creation for Windows, until then download from GitHub releases and unzip dlls
      if py_major != '3' or py_minor not in ('7', '8', '9'):
        raise OSError("Unsupported Python version. Try compiling engine instead. " 
                      "Precompiled binaries are available for Python 3.7-3.9 only")
      import urllib.request
      import urllib.error
      import zipfile
      import io
      bit_ver = '64' if sys.maxsize > 2 ** 32 else '32'
      download_link = 'https://github.com/vi3itor/football/' + \
                      f'releases/download/v{VERSION}/' + \
                      f'gfootball-engine-win-py{py_major}{py_minor}-{bit_ver}.zip'
      try:
        with urllib.request.urlopen(download_link) as response, \
            zipfile.ZipFile(io.BytesIO(response.read())) as bin_zip:
          bin_zip.extractall(dest_dir)
      except urllib.error.HTTPError:  # TODO: Handle all possible exceptions
        raise Exception("Unable to download precompiled gfootball engine")
    else:  # compile the engine
      # TODO: Check if it finds VCPKG_ROOT variable correctly
      if not os.environ.get('VCPKG_ROOT'):
        raise OSError('VCPKG_ROOT environment variable is not defined')
      os.environ['GENERATOR_PLATFORM'] = 'x64' if sys.maxsize > 2 ** 32 else 'Win32'
      os.environ['PY_VERSION'] = f'{py_major}.{py_minor}'
      if os.system('gfootball\\build_game_engine.bat'):
        raise OSError('Google Research Football compilation failed')
      # Copy compiled library and its dependencies
      lib_path = 'third_party/gfootball_engine/build_win/Release/'
      libs = glob.glob(f'{lib_path}*.pyd') + glob.glob(f'{lib_path}*.dll')
      for file in libs:
        shutil.copy2(file, dest_dir)


packages = find_packages() + find_packages('third_party')

dir_prefix = "third_party/gfootball_engine/"
data_files = glob.glob(f"{dir_prefix}data/**", recursive=True)
# strip prefix, because setuptools will look inside gfootball_engine directory
data_files = [path[len(dir_prefix):] for path in data_files]
# TODO: Include source files (*.cpp, *.hpp, *.c, *.h, CMakeLists.txt, sources.cmake), LICENSE, and README?

# Copy font files to gfootball_engine directory, so they're included during setup
dst_fonts = "third_party/gfootball_engine/fonts"
if not os.path.exists(dst_fonts):
  shutil.copytree("third_party/fonts", dst_fonts)

try:
  setup(
    name='gfootball',
    version=VERSION,
    description=('Google Research Football - RL environment based on '
                 'open-source game Gameplay Football'),
    author='Google LLC',
    author_email='no-reply@google.com',
    url='https://github.com/google-research/football',
    license='Apache 2.0',
    packages=packages,
    package_dir={'gfootball_engine': 'third_party/gfootball_engine'},
    install_requires=[
      'pygame>=1.9.6',
      'opencv-python',
      'psutil',
      'scipy',
      'gym>=0.11.0',
      'absl-py',
      'wheel',
    ],
    package_data={
      'gfootball': ['build_game_engine.sh', 'build_game_engine.bat'],
      'gfootball_engine': ['fonts/**'] + data_files,
    },
    keywords='gfootball reinforcement-learning python machine learning',
    ext_modules=[CMakeExtension('brainball_cpp_engine')],
    cmdclass={'build_ext': CustomBuild},
  )
finally:
  # Remove font files from the gfootball_engine directory
  shutil.rmtree(dst_fonts)
