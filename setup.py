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
from setuptools import find_packages
from setuptools import setup, Extension
from setuptools.command.install import install
from setuptools.command.build_ext import build_ext

class CMakeExtension(Extension):
  def __init__(self, name):
    # don't invoke the original build_ext for this special extension
    super().__init__(name, sources=[])

class CustomBuild(build_ext):
  """Custom installation script to build the C++ environment."""
  def run(self):
    # https://stackoverflow.com/questions/32419594/how-to-create-a-dylib-c-extension-on-mac-os-x-with-distutils-and-or-setuptools
    if sys.platform == 'darwin':
        from distutils import sysconfig
        vars = sysconfig.get_config_vars()
        vars['LDSHARED'] = vars['LDSHARED'].replace('-bundle', '-dynamiclib -Wl,-F.')


    dest_dir = os.path.join(self.build_lib, 'gfootball_engine')

    if os.system('cp -r third_party/fonts ' + dest_dir):
      raise OSError("Google Research Football: Could not copy "
                    "fonts to %s." % (dest_dir))

    try:
      use_prebuilt_library = int(os.environ.get("GFOOTBALL_USE_PREBUILT_SO", "0"))
    except ValueError:
      raise ValueError("Could not parse GFOOTBALL_USE_PREBUILT_SO environment "
                       "variable as int. Please set it to 0 or 1")

    if use_prebuilt_library:
      if os.system("cp third_party/gfootball_engine/lib/prebuilt_gameplayfootball.so " + dest_dir + "/_gameplayfootball.so"):
        raise OSError("Failed to copy pre-built library to a final destination %s." % dest_dir)
    else:
      if (os.system('gfootball/build_game_engine.sh') or
          os.system("cp third_party/gfootball_engine/_gameplayfootball.so " + dest_dir)):
        raise OSError("Google Research Football compilation failed")

    super(CustomBuild, self).run()

packages = find_packages() + find_packages('third_party')
setup(
    name='gfootball',
    version='2.0.4',
    description=('Google Research Football - RL environment based on '
                 'open-source game Gameplay Football'),
    author='Google LLC',
    author_email='no-reply@google.com',
    url='https://github.com/google-research/football',

    license='Apache 2.0',
    packages=packages,
    package_dir={'gfootball_engine': 'third_party/gfootball_engine'},
    install_requires=[
        'pygame==1.9.6',
        'opencv-python',
        'scipy',
        'gym>=0.11.0',
        'absl-py',
    ],
    include_package_data=True,
    keywords='gfootball reinforcement-learning python machine learning',
    ext_modules=[CMakeExtension('brainball_cpp_engine')],
    cmdclass={'build_ext': CustomBuild},
)
