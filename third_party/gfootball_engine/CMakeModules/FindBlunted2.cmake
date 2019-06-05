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

# Locate Blunted library
# This module defines
# BLUNTED2_LIBRARY, the name of the library to link against
# BLUNTED2_FOUND, if false, do not try to link to BLUNTED2
# 
# BLUNTED2_INCLUDE_DIR, where to find blunted2config.h
#
# $BLUNTED2DIR - enviroment variable
#
# Created by Farrer. This was influenced by the FindSDL_image.cmake module.

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

FIND_PATH(BLUNTED2_INCLUDE_DIR NAMES blunted.hpp
  HINTS
  $ENV{BLUNTED2DIR}
  PATH_SUFFIXES include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local/include/blunted2
  /usr/include/blunted2
  /usr/local/include
  /usr/include
  /sw/include/blunted2 # Fink
  /sw/include
  /opt/local/include/blunted2 # DarwinPorts
  /opt/local/include
  /opt/csw/include/kodold # Blastwave
  /opt/csw/include 
  /opt/include/blunted2
  /opt/include
)

FIND_LIBRARY(BLUNTED2_LIBRARY 
  NAMES blunted2
  HINTS
  $ENV{BLUNTED2DIR}
  PATH_SUFFIXES lib64 lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(BLUNTED2
   REQUIRED_VARS BLUNTED2_LIBRARY BLUNTED2_INCLUDE_DIR)
