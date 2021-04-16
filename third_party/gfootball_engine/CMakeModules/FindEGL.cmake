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

# Find the EGL library.
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``EGL::EGL``
#   The EGL library, if found.
#
# ``EGL::OpenGL``
#   The OpenGL library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``EGL_FOUND``
#   System has the EGL library.
# ``EGL_INCLUDE_DIR``
#   The EGL include directory.
# ``EGL_LIBRARY``
#   The libEGL library.
# ``EGL_LIBRARIES``
#   All EGL related libraries, including ``EGL_LIBRARY``.
#
# Hints
# ^^^^^
#
# Set `EGL_ROOT_DIR` to the root directory of an EGL installation.
find_path(EGL_INCLUDE_DIR
  NAMES
    EGL/egl.h
  PATHS
    ${EGL_ROOT_DIR}/include
    /usr/local/include
    /usr/include)

find_library(EGL_LIBRARY
  NAMES
    EGL
  PATHS
    ${EGL_ROOT_DIR}/lib
    /usr/local/lib
    /usr/lib)

find_library(EGL_opengl_LIBRARY
  NAMES
   OpenGL
  PATHS
    ${EGL_ROOT_DIR}/lib
    /usr/local/lib
    /usr/lib)

set(EGL_LIBRARIES ${EGL_LIBRARY} ${EGL_opengl_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  EGL DEFAULT_MSG
  EGL_LIBRARY EGL_opengl_LIBRARY EGL_INCLUDE_DIR)
mark_as_advanced(EGL_ROOT_DIR EGL_INCLUDE_DIR EGL_LIBRARY EGL_opengl_LIBRARY)

if(EGL_FOUND)
  add_library(EGL::OpenGL UNKNOWN IMPORTED)
  set_target_properties(EGL::OpenGL PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${EGL_INCLUDE_DIR}")
  set_target_properties(EGL::OpenGL PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
    IMPORTED_LOCATION "${EGL_opengl_LIBRARY}")

  add_library(EGL::EGL UNKNOWN IMPORTED)
  set_target_properties(EGL::EGL PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${EGL_INCLUDE_DIR}")
  set_target_properties(EGL::EGL PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
    INTERFACE_LINK_LIBRARIES "EGL::OpenGL"
    IMPORTED_LOCATION "${EGL_LIBRARY}")
endif()
