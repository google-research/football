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

# - Locate SDL2_gfx library
# This module defines:
# SDL2_GFX_LIBRARIES, the name of the library to link against
# SDL2_GFX_INCLUDE_DIRS, where to find the headers
# SDL2_GFX_FOUND, if false, do not try to link against
# SDL2_GFX_VERSION_STRING - human-readable string containing the version of SDL2_gfx
#
# For backward compatiblity the following variables are also set:
# SDL2GFX_LIBRARY (same value as SDL2_GFX_LIBRARIES)
# SDL2GFX_INCLUDE_DIR (same value as SDL2_GFX_INCLUDE_DIRS)
# SDL2GFX_FOUND (same value as SDL2_GFX_FOUND)
#
# $SDL2DIR is an environment variable that would
# correspond to the ./configure --prefix=$SDL2DIR
# used in building SDL2.
#
# Created by Eric Wing. This was influenced by the FindSDL2.cmake
# module, but with modifications to recognize OS X frameworks and
# additional Unix paths (FreeBSD, etc).

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
# Copyright 2012 Benjamin Eikel
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
# License text for the above reference.)

if(NOT SDL2_GFX_INCLUDE_DIR AND SDL2GFX_INCLUDE_DIR)
  set(SDL2_GFX_INCLUDE_DIR ${SDL2GFX_INCLUDE_DIR} CACHE PATH "directory cache
entry initialized from old variable name")
endif()
find_path(SDL2_GFX_INCLUDE_DIR SDL2_gfxPrimitives.h
  HINTS
    ENV SDL2GFXDIR
    ENV SDL2DIR
  PATH_SUFFIXES include/SDL2 include
  PATHS
  ${CMAKE_SOURCE_DIR}/deps
)

if(NOT SDL2_GFX_LIBRARY AND SDL2GFX_LIBRARY)
  set(SDL2_GFX_LIBRARY ${SDL2GFX_LIBRARY} CACHE FILEPATH "file cache entry
initialized from old variable name")
endif()
find_library(SDL2_GFX_LIBRARY
  NAMES SDL2_gfx
  HINTS
    ENV SDL2GFXDIR
    ENV SDL2DIR
  PATH_SUFFIXES lib
  PATHS
  ${CMAKE_SOURCE_DIR}/deps
)

if(SDL2_GFX_INCLUDE_DIR AND EXISTS "${SDL2_GFX_INCLUDE_DIR}/SDL2_gfxPrimitives.h")
  file(STRINGS "${SDL2_GFX_INCLUDE_DIR}/SDL2_gfxPrimitives.h" SDL2_GFX_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL2_GFXPRIMITIVES_MAJOR[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_GFX_INCLUDE_DIR}/SDL2_gfxPrimitives.h" SDL2_GFX_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL2_GFXPRIMITIVES_MINOR[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_GFX_INCLUDE_DIR}/SDL2_gfxPrimitives.h" SDL2_GFX_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL2_GFXPRIMITIVES_MICRO[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL2_GFXPRIMITIVES_MAJOR[ \t]+([0-9]+)$" "\\1" SDL2_GFX_VERSION_MAJOR "${SDL2_GFX_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL2_GFXPRIMITIVES_MINOR[ \t]+([0-9]+)$" "\\1" SDL2_GFX_VERSION_MINOR "${SDL2_GFX_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL2_GFXPRIMITIVES_MICRO[ \t]+([0-9]+)$" "\\1" SDL2_GFX_VERSION_PATCH "${SDL2_GFX_VERSION_PATCH_LINE}")
  set(SDL2_GFX_VERSION_STRING ${SDL2_GFX_VERSION_MAJOR}.${SDL2_GFX_VERSION_MINOR}.${SDL2_GFX_VERSION_PATCH})
  unset(SDL2_GFX_VERSION_MAJOR_LINE)
  unset(SDL2_GFX_VERSION_MINOR_LINE)
  unset(SDL2_GFX_VERSION_PATCH_LINE)
  unset(SDL2_GFX_VERSION_MAJOR)
  unset(SDL2_GFX_VERSION_MINOR)
  unset(SDL2_GFX_VERSION_PATCH)
endif()

set(SDL2_GFX_LIBRARIES ${SDL2_GFX_LIBRARY})
set(SDL2_GFX_INCLUDE_DIRS ${SDL2_GFX_INCLUDE_DIR})

# for backward compatiblity
set(SDL2GFX_LIBRARY ${SDL2_GFX_LIBRARIES})
set(SDL2GFX_INCLUDE_DIR ${SDL2_GFX_INCLUDE_DIRS})
set(SDL2GFX_FOUND ${SDL2_GFX_FOUND})

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2_gfx
                                  REQUIRED_VARS SDL2_GFX_LIBRARIES SDL2_GFX_INCLUDE_DIRS
                                  VERSION_VAR SDL2_GFX_VERSION_STRING)



mark_as_advanced(SDL2_GFX_LIBRARY SDL2_GFX_INCLUDE_DIR)
