# Compiling Google Research Football Engine #

This guide is intended to contain detailed information on building the environment
on different platforms and architectures. So far, it covers:
* [Windows](#windows),
* [macOS](#macos),
* [Linux](#linux).

Additionally, [Development mode](#development-mode) section explains how to install the game
in the development (aka, editable) mode.

## Windows
Install prerequisites:
- [Git](https://git-scm.com/download/win),
- [CMake](https://cmake.org/download/),
- [Visual Studio 2019 Community Edition](https://visualstudio.microsoft.com/downloads/) (make sure to
  select "Desktop development with C++" component),
- [Python 3](https://www.python.org/downloads/).

Finally, install [vcpkg](https://github.com/microsoft/vcpkg) by following a
  [Quick Start Guide](https://github.com/microsoft/vcpkg#quick-start-windows) or simply by creating a directory,
  e.g., `C:\dev`, opening Command Prompt and running the following commands:
```commandline
:: Navigate to the created directory
cd C:\dev
:: Clone vckpg
git clone https://github.com/microsoft/vcpkg.git
cd vckpg
:: Run installation script
.\bootstrap-vcpkg.bat
:: Return to previous directory
cd ..
```

If you have `vcpkg` already installed, consider updating it to the latest commit and running `.\bootstrap-vcpkg.bat`.

As for the C++ engine dependencies (`boost`, `SDL`, etc.), you don't have to install them manually, `vcpkg's`
[manifest mode](https://github.com/microsoft/vcpkg/blob/master/docs/users/manifests.md) will handle it.

Next, run the following lines in the command prompt to install Google Research Football environment:
```commandline
:: Clone the repository
git clone https://github.com/google-research/football.git
cd football
:: Set VCPKG_ROOT environment variable that points to vcpkg installation
set VCPKG_ROOT=C:\dev\vcpkg\

:: Create and activate virtual environment
python -m venv football-env
football-env\Scripts\activate.bat
:: For PowerShell users: football-env\Scripts\activate.ps1

:: Upgrade pip and install additional packages
python -m pip install --upgrade pip setuptools wheel
python -m pip install psutil

:: Run the installation. It installs vcpkg dependencies and compiles the engine
pip install . --use-feature=in-tree-build
```


## macOS

First, install [brew](https://brew.sh/). It should automatically download Command Line Tools.
Next, install the required packages:
```shell
brew install git python3 cmake sdl2 sdl2_image sdl2_ttf sdl2_gfx boost boost-python3

python3 -m pip install --upgrade pip setuptools wheel
python3 -m pip install psutil
```
Clone the repository and navigate to the directory:
```shell
git clone https://github.com/google-research/football.git
cd football
```

### Intel processor
#### Installation with brew version of Python
It is recommended to use Python shipped with `brew`, because `boost-python3` is compiled against the same version.
To check which Python 3 is used by default on your setup, execute `which python3`.
For Intel-based Macs it should be `/usr/local/bin/python3`.
If you have a different path, and you don't want to change symlinks, create a virtual environment with
`/usr/local/bin/python3 -m venv football-env` or `$(brew --prefix python3)/bin/python3.9 -m venv football-env`.

Use [virtual environment](https://docs.python.org/3/tutorial/venv.html) to avoid messing up with global dependencies:

```shell
python3 -m venv football-env
source football-env/bin/activate
# update pip and setuptools
python3 -m pip install --upgrade pip setuptools wheel
python3 -m pip install psutil
```

Finally, build the game environment:

```shell
python3 -m pip install . --use-feature=in-tree-build
```

#### Installation with conda

If you installed the engine using `conda`, you might encounter the following error:
`TypeError: __init__() should return None, not 'NoneType'` when trying to run the game.
It may happen because the `boost-python3` installed with `brew` is compiled against a different
version of Python (see the [discussion](https://github.com/google-research/football/issues/156)).
If you successfully installed and ran Google Research Football using `conda` please update this guide.
For now, the easiest way is to deactivate `conda` environment and install GRF with `brew` version of Python:
```shell
conda deactivate
$(brew --prefix python3)/bin/python3.9 -m venv football-env
source football-env/bin/activate
python3 -m pip install --upgrade pip setuptools psutil wheel
python3 -m pip install . --use-feature=in-tree-build
```

### Apple Silicon
The environment can be compiled and run on Apple Silicon. Until some dependencies (`opencv-python`, `numpy`, etc.)
fully support new architecture, the required components should be installed manually beforehand,
and Google Research Football should be installed without dependencies (`--no-deps`).

#### Installation with conda
Python dependencies can be installed via `conda's` fork [miniforge](https://github.com/conda-forge/miniforge).
But you may encounter a `TypeError` when running the game if you use `boost-python3` from `brew`.

## Linux
Install required packages:

```shell
sudo apt-get install git cmake build-essential libgl1-mesa-dev libsdl2-dev \
libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev libboost-all-dev \
libdirectfb-dev libst-dev mesa-utils xvfb x11vnc python3-pip

python3 -m pip install --upgrade pip setuptools wheel
python3 -m pip install psutil
```
Use [virtual environment](https://docs.python.org/3/tutorial/venv.html) to avoid messing up with global dependencies:

```shell
python3 -m venv football-env
source football-env/bin/activate
# update pip and setuptools
python3 -m pip install --upgrade pip setuptools wheel
python3 -m pip install psutil
```

Finally, build the game environment:

```shell
python3 -m pip install . --use-feature=in-tree-build
```

## Development mode

You can install Google Research Football
in the [development](https://packaging.python.org/guides/distributing-packages-using-setuptools/#id66)
(aka editable) mode by running:

```shell
python3 -m pip install -e . --use-feature=in-tree-build
```

In such case, Python source files in projects can be edited in-place without reinstallation,
the changes will be reflected the next time an interpreter process is started.
