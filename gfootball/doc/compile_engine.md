# Compiling GFootball Engine #

## Windows
Prerequisites:
- [Git](https://git-scm.com/download/win),
- [CMake](https://cmake.org/download/),
- [Visual Studio 2019 Community Edition](https://visualstudio.microsoft.com/downloads/) make sure to 
  select "Desktop development with C++" component.
- [Python 3](https://www.python.org/downloads/),
- finally, install [vcpkg](https://github.com/microsoft/vcpkg) as explained in
  [Quick Start Guide](https://github.com/microsoft/vcpkg#quick-start-windows) or simply by creating a directory,
  e.g. `C:\dev`, opening Command Prompt and running the following commands:
```commandline
:: Navigate to the created directory
cd C:\dev
:: Clone vckpg
git clone https://github.com/microsoft/vcpkg
cd vckpg
```

The `HEAD` commit is recommended, but it will build the latest version of Python 3 libraries.
If you have to run `GFootball` on different version of Python, checkout the corresponding commits:
- For Python 3.8: `git checkout e803bf112`
- For Python 3.7: `git checkout ca52d429b`

```commandline
:: Run installation script
.\bootstrap-vcpkg.bat
```

Next, install required dependencies (it might take 20-30 min for `vcpkg` to compile `boost` and other libraries):
```commandline
:: If you use 64-bit version of Python
.\vcpkg.exe install --triplet x64-windows boost sdl2 sdl2-image[libjpeg-turbo] sdl2-ttf sdl2-gfx opengl
:: If you use 32-bit version of Python
.\vcpkg.exe install --triplet x86-windows boost:x86-windows sdl2 sdl2-image[libjpeg-turbo] sdl2-ttf sdl2-gfx opengl
```

Next, install `GFootball`:
```commandline
:: Upgrade pip and additional packages
python -m pip install --upgrade pip setuptools psutil wheel
:: Clone the repository
git clone https://github.com/google-research/football.git
cd football
:: Set VCPKG_ROOT environment variable that points to vcpkg installation
set VCPKG_ROOT=C:\dev\vcpkg\

:: Create and activate virtual environment
python -m venv football-env
football-env\Scripts\activate.bat
:: For PowerShell users: football-env\Scripts\activate.ps1

:: Upgrade pip inside the environment
python -m pip install --upgrade pip setuptools psutil wheel

:: Run compilation and installation
pip install . --use-feature=in-tree-build 
```


## macOS

First install [brew](https://brew.sh/). It should automatically download Command Line Tools.
Next install required packages:

```shell
brew install git python3 cmake sdl2 sdl2_image sdl2_ttf sdl2_gfx boost boost-python3

python3 -m pip install --upgrade pip setuptools psutil wheel
```

### Intel processor
#### Installation with brew version of Python
It is recommended to use Python shipped with `brew`, because `boost-python3` is compiled against the same version.
To check which Python 3 is used by default on your set up execute `which python3`.
For Intel-based Macs it should be `/usr/local/bin/python3`.
If you have a different path and you don't want to change symlinks, create virtual environment with
`/usr/local/bin/python3 -m venv football-env` or `$(brew --prefix python3)/bin/python3.9 -m venv football-env`.

We highly recommend using [virtual environment](https://docs.python.org/3/tutorial/venv.html) to avoid messing up global dependencies:

```shell
python3 -m venv football-env
source football-env/bin/activate
# update pip and setuptools
python3 -m pip install --upgrade pip setuptools psutil wheel
```

Next, build the game environment:

```shell
python3 -m pip install . --use-feature=in-tree-build
```

#### Installation with conda
TODO: Write a guide for conda installation 

### Apple Silicon (M1)
GFootball can be compiled and run on Apple Silicon. Until some dependencies (opencv-python, numpy) fully support
new architecture, the required components should be installed manually, and the installation of GFootball itself
should be run without dependencies (`--no-deps`). 

TODO: Describe the process in details

## Linux
TODO


## Common problems
TODO

