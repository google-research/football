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
    # https://stackoverflow.com/questions/32419594/how-to-create-a-dylib-c-extension-on-mac-os-x-with-distutils-and-or-setuptools
    if sys.platform == 'darwin':
      from distutils import sysconfig
      vars = sysconfig.get_config_vars()
      vars['LDSHARED'] = vars['LDSHARED'].replace('-bundle',
                                                  '-dynamiclib -Wl,-F.')
    if os.path.exists(self.build_lib):
      dest_dir = os.path.join(self.build_lib, 'gfootball_engine')
    else:
      # Development install (pip install -e .)
      dest_dir = "gfootball_engine"
      if os.system('cp -r third_party/gfootball_engine/ ' + dest_dir):
        raise OSError("Google Research Football: Could not copy "
                      "engine to %s." % dest_dir)
    if os.system('cp -r third_party/fonts ' + dest_dir):
      raise OSError('Google Research Football: Could not copy '
                    'fonts to %s.' % dest_dir)

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
    if os.path.exists(self.build_lib):
      dest_dir = os.path.join(self.build_lib, 'gfootball_engine')
    else:  # Development install
      dest_dir = "gfootball_engine"
      if not os.path.exists(dest_dir):
        os.mkdir(dest_dir)
      # For development mode (pip install -e .) gfootball_engine module has to be located
      # in the project root directory. So we copy only __init__.py and `data` directory
      shutil.copy2('third_party/gfootball_engine/__init__.py', dest_dir)
      data_dir = os.path.join(dest_dir, 'data')
      if not os.path.exists(data_dir):
        shutil.copytree('third_party/gfootball_engine/data', data_dir)

    # Copy fonts
    dst_fonts = os.path.join(dest_dir, "fonts")
    if not os.path.exists(dst_fonts):
      shutil.copytree("third_party/fonts", dst_fonts)

    py_major, py_minor, _ = platform.python_version_tuple()
    guide_message = 'Please follow the guide on how to install prerequisites: ' \
                  'https://github.com/google-research/football/blob/master/gfootball/doc/compile_engine.md#windows'
    if not os.environ.get('VCPKG_ROOT'):
      raise OSError('VCPKG_ROOT environment variable is not defined.\n' + guide_message)
    os.environ['GENERATOR_PLATFORM'] = 'x64' if sys.maxsize > 2 ** 32 else 'Win32'
    os.environ['PY_VERSION'] = f'{py_major}.{py_minor}'
    if os.system('gfootball\\build_game_engine.bat'):
      raise OSError('Google Research Football compilation failed.\n' + guide_message)
    # Copy compiled library and its dependencies
    lib_path = 'third_party/gfootball_engine/build_win/Release/'
    libs = glob.glob(f'{lib_path}*.pyd') + glob.glob(f'{lib_path}*.dll')
    for file in libs:
      shutil.copy2(file, dest_dir)


# To support development (a.k.a. editable) install (`pip install -e .` or `python3 setup.py develop`),
# we remove `build` directory created by a regular setup
if 'develop' in sys.argv and os.path.exists('build'):
  shutil.rmtree('build')

packages = find_packages() + find_packages('third_party')

setup(
    name='gfootball',
    version='2.10.2',
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
        'scipy',
        'gym>=0.11.0',
        'absl-py',
        'wheel',
    ],
    include_package_data=True,
    keywords='gfootball reinforcement-learning python machine learning',
    ext_modules=[CMakeExtension('brainball_cpp_engine')],
    cmdclass={'build_ext': CustomBuild},
)
