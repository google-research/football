cmake_minimum_required(VERSION 3.1)

if(TRUE) #(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
   # TODO: read from environment, e.g. "$ENV{VCPKG_ROOT}/..."
   set(VCPKG_ROOT "c:/repo/vcpkg_latest")
   set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
   set(CMAKE_GENERATOR_PLATFORM x64)
   set(TRIPLET x64-windows)
   set(PY_VER_MAJ 3)
   set(PY_VER_MIN 9)
   set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

set(PACKAGE gameplayfootball)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 2)
set(VERSION_PATCH 0)
if(${VERSION_PATCH})
   set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
else(${VERSION_PATCH})
   set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR})
endif(${VERSION_PATCH})

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules")

project(gameplayfootball)

set (CMAKE_CXX_STANDARD 14)

if(UNIX)
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -O3")
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -g -O3")
endif(UNIX)
#SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} -fPIC -g -O3")
#SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fPIC -O3 -g")

# Find needed libraries
FIND_PACKAGE(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

FIND_PACKAGE(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

if(UNIX AND NOT APPLE)
   FIND_PACKAGE(EGL REQUIRED)
   include_directories(${EGL_INCLUDE_DIR})
endif()

FIND_PACKAGE(SDL2-image REQUIRED)
include_directories(${SDL2_IMAGE_DIRS})

FIND_PACKAGE(SDL2-ttf REQUIRED)
include_directories(${SDL2_TTF_DIRS})

FIND_PACKAGE(SDL2-gfx REQUIRED)
include_directories(${SDL2_GFX_DIRS})

#find_package(PythonLibs 3 REQUIRED)
#message("Using Python: ${PYTHONLIBS_VERSION_STRING}")
#find_package (Python COMPONENTS Development) # Interpreter Development)
set(Python_LIBRARIES "${VCPKG_ROOT}/installed/${TRIPLET}/lib/python${PY_VER_MAJ}${PY_VER_MIN}.lib")
set(Python_INCLUDE_DIR "${VCPKG_ROOT}/installed/${TRIPLET}/include/python${PY_VER_MAJ}.${PY_VER_MIN}" )

FIND_PACKAGE(Boost REQUIRED COMPONENTS thread system filesystem python${PY_VER_MAJ}${PY_VER_MIN})
include_directories(${Boost_INCLUDE_DIR})

include_directories(${Python_INCLUDE_DIR})

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/cmake)

# Include the sources
include(sources.cmake)

# Join all created static libraries into the single static or share one.

# For shared library, uncomment bellow:

#set_property(TARGET baselib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET systemscommonlib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET systemsgraphicslib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET loaderslib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET typeslib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET scenelib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET managerslib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET utilslib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET gui2lib PROPERTY POSITION_INDEPENDENT_CODE 1)
#add_library(blunted2 SHARED ${CORE_SOURCES} ${CORE_HEADERS} ${OWN_LIBRARIES})

#set_target_properties(blunted2 PROPERTIES VERSION ${VERSION}
#   SOVERSION ${VERSION_MAJOR} )
#add_library(blunted2 SHARED ${CORE_SOURCES} ${LIBS_SOURCES} ${CORE_HEADERS}
#   ${ALL_LIBS_HEADERS} ${OWN_LIBRARIES})

add_library(baselib OBJECT ${BASE_SOURCES} ${BASE_HEADERS}
   ${BASE_GEOMETRY_HEADERS} ${BASE_MATH_HEADERS})
add_library(systemscommonlib OBJECT ${SYSTEMS_COMMON_SOURCES}
   ${SYSTEMS_COMMON_HEADERS})
set_target_properties(systemscommonlib PROPERTIES LINKER_LANGUAGE CXX)
add_library(systemsgraphicslib OBJECT ${SYSTEMS_GRAPHICS_SOURCES}
   ${SYSTEMS_GRAPHICS_HEADERS} ${SYSTEMS_GRAPHICS_OBJECTS_HEADERS}
   ${SYSTEMS_GRAPHICS_RESOURCES_HEADERS} ${SYSTEMS_GRAPHICS_RENDERING_HEADERS})
add_library(loaderslib OBJECT ${LOADERS_SOURCES} ${LOADERS_HEADERS})
add_library(typeslib OBJECT ${TYPES_SOURCES} ${TYPES_HEADERS})
add_library(scenelib OBJECT ${SCENE_SOURCES} ${SCENE_HEADERS}
   ${SCENE2D_HEADERS} ${SCENE3D_HEADERS} ${SCENE_OBJECTS_HEADERS}
   ${SCENE_RESOURCES_HEADERS})
add_library(managerslib OBJECT ${MANAGERS_SOURCES} ${MANAGERS_HEADERS})
set_target_properties(managerslib PROPERTIES LINKER_LANGUAGE CXX)
add_library(utilslib OBJECT ${UTILS_SOURCES} ${UTILS_HEADERS}
   ${UTILS_EXT_HEADERS})
add_library(gui2lib OBJECT ${UTILS_GUI2_SOURCES} ${UTILS_GUI2_HEADERS}
   ${UTILS_GUI2_WIDGETS_HEADERS})

set(OWN_LIBRARIES $<TARGET_OBJECTS:baselib> $<TARGET_OBJECTS:systemscommonlib>
   $<TARGET_OBJECTS:systemsgraphicslib> $<TARGET_OBJECTS:loaderslib>
   $<TARGET_OBJECTS:typeslib> $<TARGET_OBJECTS:scenelib>
   $<TARGET_OBJECTS:managerslib> $<TARGET_OBJECTS:utilslib>
   $<TARGET_OBJECTS:gui2lib>)

# For static library.
add_library(blunted2 ${BLUNTED_CORE_SOURCES} ${LIBS_SOURCES} ${BLUNTED_CORE_HEADERS}
   ${ALL_LIBS_HEADERS} ${OWN_LIBRARIES})


# Compile it as multiple static libraries (note: not compiling physics,
# as not used by gameplayfootball)
add_library(gamelib ${GAME_SOURCES} ${GAME_HEADERS})
add_library(menulib ${MENU_SOURCES} ${MENU_HEADERS})
add_library(datalib ${DATA_SOURCES} ${DATA_HEADERS})

set(LIBRARIES gamelib menulib datalib blunted2
   Boost::filesystem Boost::system Boost::thread Boost::python${PY_VER_MAJ}${PY_VER_MIN} ${Python_LIBRARIES}
   SDL2::SDL2 SDL2::SDL2_image SDL2::SDL2_ttf SDL2::SDL2_gfx SDL2::SDL2main
   ${OPENGL_LIBRARY})


add_library(_gameplayfootball SHARED ${CORE_SOURCES} ${CORE_HEADERS} ${AI_HEADERS} ${AI_SOURCES})
set_target_properties(_gameplayfootball PROPERTIES SUFFIX ".pyd")
target_link_libraries(_gameplayfootball ${LIBRARIES})
