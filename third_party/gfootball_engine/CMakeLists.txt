cmake_minimum_required(VERSION 3.4)

if(WIN32)
  # Check for required Environment variables
  if(NOT DEFINED CMAKE_GENERATOR_PLATFORM)
    message(FATAL_ERROR "CMAKE_GENERATOR_PLATFORM is not defined. Add -Ax64 or -AWin32 to cmake command")
  elseif(NOT DEFINED ENV{VCPKG_ROOT})
    message(FATAL_ERROR "VCPKG_ROOT is not defined. Set the location of vcpkg")
  elseif(NOT DEFINED ENV{PY_VERSION})  # TODO: Check if PY_VERSION has correct format
    message(FATAL_ERROR "PY_VERSION is not defined. Define Python version that is available in vcpkg, e.g. 3.9")
  endif()

  if(${CMAKE_GENERATOR_PLATFORM} STREQUAL Win32)
    set(TRIPLET x86-windows)
  elseif(${CMAKE_GENERATOR_PLATFORM} STREQUAL x64)
    set(TRIPLET x64-windows)
  else()
    message(FATAL_ERROR "Unsupported Generator Platform, only Win32 or x64 are supported")
  endif()

  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
else()  # UNIX
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules")
endif()

project(gameplayfootball)
set(PACKAGE gameplayfootball)

set (CMAKE_CXX_STANDARD 14)

if(UNIX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} -fPIC -g -O3")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -O3 -g")
endif(UNIX)

# Find needed libraries
FIND_PACKAGE(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

FIND_PACKAGE(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

if(UNIX AND NOT APPLE)
  FIND_PACKAGE(EGL REQUIRED)
  include_directories(${EGL_INCLUDE_DIR})
endif()

if(WIN32)
  FIND_PACKAGE(SDL2-image REQUIRED)
  FIND_PACKAGE(SDL2-ttf REQUIRED)
  FIND_PACKAGE(SDL2-gfx REQUIRED)
else()  # Unix
  FIND_PACKAGE(SDL2_image REQUIRED)
  FIND_PACKAGE(SDL2_ttf REQUIRED)
  FIND_PACKAGE(SDL2_gfx REQUIRED)
endif()

include_directories(${SDL2_IMAGE_DIRS})
include_directories(${SDL2_TTF_DIRS})
include_directories(${SDL2_GFX_DIRS})

if(UNIX)
  # TODO: Discuss if it is better to replace deprecated FindPythonLibs with FindPython3 (released since CMake 3.12 - July 2018)
  find_package(PythonLibs 3 REQUIRED)
else()
  set(PYTHONLIBS_VERSION_STRING $ENV{PY_VERSION})
endif()

message("Using Python: ${PYTHONLIBS_VERSION_STRING}")
string(REPLACE "." ";" VERSION_LIST ${PYTHONLIBS_VERSION_STRING})
list(GET VERSION_LIST 0 PYTHON_MAJOR)
list(GET VERSION_LIST 1 PYTHON_MINOR)

if(WIN32)
  # TODO: Find better fix.
  # find_package (Python COMPONENTS Development)
  # FindPython package searches for system-wide python installations first.
  # It was improved only recently (December 2020), so we can't use it for earlier py versions TODO: add GitHub issue link
  # To avoid version mismatch we point to vcpkg's libs directly
  set(PYTHON_LIBRARIES "$ENV{VCPKG_ROOT}/installed/${TRIPLET}/lib/python${PYTHON_MAJOR}${PYTHON_MINOR}.lib")
  set(PYTHON_INCLUDE_DIR "$ENV{VCPKG_ROOT}/installed/${TRIPLET}/include/python${PYTHON_MAJOR}.${PYTHON_MINOR}")
endif()

# Include the python boost.
# Unfortunately different linux distributions are using different naming.

if(WIN32 OR APPLE)
  set(BOOST_PYTHON_VERSION python${PYTHON_MAJOR}${PYTHON_MINOR})
else()
  # TODO: write a function to find boost-python in the list of possible names for linux
  # Ubuntu
  FIND_PACKAGE(Boost COMPONENTS python3-py${PYTHON_MAJOR}${PYTHON_MINOR})
  if (Boost_PYTHON3-PY${PYTHON_MAJOR}${PYTHON_MINOR}_FOUND)
    set(BOOST_PYTHON_VERSION python3-py${PYTHON_MAJOR}${PYTHON_MINOR})
  else()
    message("python3-py${PYTHON_MAJOR}${PYTHON_MINOR} not found. Trying other names.")
    # Debian
    FIND_PACKAGE(Boost COMPONENTS python-py${PYTHON_MAJOR}${PYTHON_MINOR})
    if (Boost_PYTHON-PY${PYTHON_MAJOR}${PYTHON_MINOR}_FOUND)
      set(BOOST_PYTHON_VERSION python-py${PYTHON_MAJOR}${PYTHON_MINOR})
    else()
      message("python-py${PYTHON_MAJOR}${PYTHON_MINOR} not found. Trying other names.")

      # Some other distributions.
      FIND_PACKAGE(Boost COMPONENTS python${PYTHON_MAJOR}${PYTHON_MINOR})
      if (Boost_PYTHON${PYTHON_MAJOR}${PYTHON_MINOR}_FOUND)
        set(BOOST_PYTHON_VERSION python${PYTHON_MAJOR}${PYTHON_MINOR})
      else()
        message(FATAL_ERROR "Python boost not found")
      endif()
    endif()
  endif()
endif()
message("Using python_boost: ${BOOST_PYTHON_VERSION}")

FIND_PACKAGE(Boost REQUIRED COMPONENTS thread system filesystem ${BOOST_PYTHON_VERSION})
include_directories(${Boost_INCLUDE_DIR})

include_directories(${PYTHON_INCLUDE_DIR})

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/cmake)

# Include the sources
include(sources.cmake)

add_library(baselib OBJECT ${BASE_SOURCES} ${BASE_HEADERS}
   ${BASE_GEOMETRY_HEADERS} ${BASE_MATH_HEADERS})
add_library(systemsgraphicslib OBJECT ${SYSTEMS_GRAPHICS_SOURCES} ${SYSTEMS_COMMON_HEADERS}
   ${SYSTEMS_GRAPHICS_HEADERS} ${SYSTEMS_GRAPHICS_OBJECTS_HEADERS}
   ${SYSTEMS_GRAPHICS_RESOURCES_HEADERS} ${SYSTEMS_GRAPHICS_RENDERING_HEADERS})
add_library(loaderslib OBJECT ${LOADERS_SOURCES} ${LOADERS_HEADERS} ${MANAGERS_HEADERS})
add_library(typeslib OBJECT ${TYPES_SOURCES} ${TYPES_HEADERS})
add_library(scenelib OBJECT ${SCENE_SOURCES} ${SCENE_HEADERS}
   ${SCENE2D_HEADERS} ${SCENE3D_HEADERS} ${SCENE_OBJECTS_HEADERS}
   ${SCENE_RESOURCES_HEADERS})
add_library(utilslib OBJECT ${UTILS_SOURCES} ${UTILS_HEADERS}
   ${UTILS_EXT_HEADERS})
add_library(gui2lib OBJECT ${UTILS_GUI2_SOURCES} ${UTILS_GUI2_HEADERS}
   ${UTILS_GUI2_WIDGETS_HEADERS})

# Join all created static libraries into a single static or shared one.
# vk: it is cleaner to set OWN_LIBRARIES after defining target_objects
set(OWN_LIBRARIES $<TARGET_OBJECTS:baselib> # $<TARGET_OBJECTS:systemscommonlib>
   $<TARGET_OBJECTS:systemsgraphicslib> $<TARGET_OBJECTS:loaderslib>
   $<TARGET_OBJECTS:typeslib> $<TARGET_OBJECTS:scenelib>
   $<TARGET_OBJECTS:utilslib> $<TARGET_OBJECTS:gui2lib>)

# TODO: Check if it is required to have multiple libs, or it is enough to combine them into OWN_LIBRARIES
# Compile it as multiple static libraries
add_library(blunted2 ${BLUNTED_CORE_SOURCES} ${BLUNTED_CORE_HEADERS} ${OWN_LIBRARIES})
add_library(gamelib ${GAME_SOURCES} ${GAME_HEADERS})
add_library(menulib ${MENU_SOURCES} ${MENU_HEADERS})
add_library(datalib ${DATA_SOURCES} ${DATA_HEADERS})

if(UNIX)
  set(SDL2_LIBS_ALL ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${SDL2_GFX_LIBRARIES} ${SDL2_LIBRARIES})
elseif(WIN32)
  set(SDL2_LIBS_ALL SDL2::SDL2 SDL2::SDL2_image SDL2::SDL2_ttf SDL2::SDL2_gfx SDL2::SDL2main)
endif()

set(LIBRARIES gamelib menulib datalib blunted2
   Boost::filesystem Boost::system Boost::thread Boost::${BOOST_PYTHON_VERSION}
   ${PYTHON_LIBRARIES} ${SDL2_LIBS_ALL} ${EGL_LIBRARIES} ${OPENGL_LIBRARY})

if(UNIX)
  set(LIB_NAME game)
else()  # WIN32
  # Windows requires the library to have the same name as the module
  set(LIB_NAME _gameplayfootball)
endif()

add_library(${LIB_NAME} SHARED ${CORE_SOURCES} ${CORE_HEADERS} ${AI_HEADERS} ${AI_SOURCES})
target_link_libraries(${LIB_NAME} ${LIBRARIES})

if(WIN32)
  set_target_properties(${LIB_NAME} PROPERTIES SUFFIX ".pyd")
endif()
