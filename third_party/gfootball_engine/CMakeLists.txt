
cmake_minimum_required(VERSION 3.1)

set(PACKAGE gameplayfootball)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 2)
set(VERSION_PATCH 0)
if(${VERSION_PATCH})
   set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
else(${VERSION_PATCH})
   set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR})
endif(${VERSION_PATCH})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules")

project(gameplayfootball)

set (CMAKE_CXX_STANDARD 14)

if(UNIX)
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -O3 -no-pie -L/usr/X11/lib -lX11 -lSDL2")
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -g -O3 -no-pie -L/usr/X11/lib -lX11 -lSDL2")
endif(UNIX)
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} -fPIC -g -O3 -no-pie -L/usr/X11/lib -lX11 -lSDL2")
SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fPIC -O3 -g -no-pie -L/usr/X11/lib -lX11 -lSDL2")

# Find needed libraries
FIND_PACKAGE(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

FIND_PACKAGE(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

FIND_PACKAGE(SDL2_image REQUIRED)
include_directories(${SDL2_IMAGE_DIRS})

FIND_PACKAGE(SDL2_ttf REQUIRED)
include_directories(${SDL2_TTF_DIRS})

FIND_PACKAGE(SDL2_gfx REQUIRED)
include_directories(${SDL2_GFX_DIRS})


find_package(PythonLibs 3 REQUIRED)
message("Using Python: ${PYTHONLIBS_VERSION_STRING}")
string(REPLACE "." ";" VERSION_LIST ${PYTHONLIBS_VERSION_STRING})
list(GET VERSION_LIST 0 PYTHON_MAJOR)
list(GET VERSION_LIST 1 PYTHON_MINOR)

# Include the python boost.
# Unfortunately different linux distributions are using different naming.

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

message("Using python_boost: ${BOOST_PYTHON_VERSION}")

FIND_PACKAGE(Boost REQUIRED COMPONENTS system thread filesystem ${BOOST_PYTHON_VERSION})
include_directories(${Boost_INCLUDE_DIR})

include_directories(${PYTHON_INCLUDE_DIR})

FIND_PACKAGE(SGE REQUIRED)
include_directories(${SGE_INCLUDE_DIR})

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/cmake)

# Include the sources
include(sources.cmake)


set(OWN_LIBRARIES $<TARGET_OBJECTS:baselib> $<TARGET_OBJECTS:systemscommonlib>
   $<TARGET_OBJECTS:systemsgraphicslib> $<TARGET_OBJECTS:loaderslib>
   $<TARGET_OBJECTS:typeslib> $<TARGET_OBJECTS:frameworklib>
   $<TARGET_OBJECTS:scenelib> $<TARGET_OBJECTS:managerslib>
   $<TARGET_OBJECTS:utilslib> $<TARGET_OBJECTS:gui2lib>)

# Join all created static libraries into the single static or share one.

# For shared library, uncomment bellow:

#set_property(TARGET baselib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET systemscommonlib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET systemsgraphicslib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET loaderslib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET typeslib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET frameworklib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET scenelib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET managerslib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET utilslib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET gui2lib PROPERTY POSITION_INDEPENDENT_CODE 1)
#add_library(blunted2 SHARED ${CORE_SOURCES} ${CORE_HEADERS} ${OWN_LIBRARIES})

#set_target_properties(blunted2 PROPERTIES VERSION ${VERSION}
#   SOVERSION ${VERSION_MAJOR} )
#add_library(blunted2 SHARED ${CORE_SOURCES} ${LIBS_SOURCES} ${CORE_HEADERS}
#   ${ALL_LIBS_HEADERS} ${OWN_LIBRARIES})

# For static library.
add_library(blunted2 ${BLUNTED_CORE_SOURCES} ${LIBS_SOURCES} ${BLUNTED_CORE_HEADERS}
   ${ALL_LIBS_HEADERS} ${OWN_LIBRARIES})


# Compile it as multiple static libraries (note: not compiling physics,
# as not used by gameplayfootball)
add_library(gamelib ${GAME_SOURCES} ${GAME_HEADERS})
add_library(menulib ${MENU_SOURCES} ${MENU_HEADERS})
add_library(datalib ${DATA_SOURCES} ${DATA_HEADERS})

set(LIBRARIES gamelib menulib datalib glee
   blunted2 rt ${Boost_LIBRARIES} ${PYTHON_LIBRARY}
   ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${SDL2_GFX_LIBRARIES}
   ${SDL2_LIBRARY} ${SGE_LIBRARY} dl m sqlite3 ${OPENGL_LIBRARIES})
add_library(baselib OBJECT ${BASE_SOURCES} ${BASE_HEADERS}
   ${BASE_GEOMETRY_HEADERS} ${BASE_MATH_HEADERS})
add_library(systemscommonlib OBJECT ${SYSTEMS_COMMON_SOURCES}
   ${SYSTEMS_COMMON_HEADERS})
add_library(systemsgraphicslib OBJECT ${SYSTEMS_GRAPHICS_SOURCES}
   ${SYSTEMS_GRAPHICS_HEADERS} ${SYSTEMS_GRAPHICS_OBJECTS_HEADERS}
   ${SYSTEMS_GRAPHICS_RESOURCES_HEADERS} ${SYSTEMS_GRAPHICS_RENDERING_HEADERS})
add_library(loaderslib OBJECT ${LOADERS_SOURCES} ${LOADERS_HEADERS})
add_library(typeslib OBJECT ${TYPES_SOURCES} ${TYPES_HEADERS})
add_library(frameworklib OBJECT ${FRAMEWORK_SOURCES} ${FRAMEWORK_HEADERS})
add_library(scenelib OBJECT ${SCENE_SOURCES} ${SCENE_HEADERS}
   ${SCENE2D_HEADERS} ${SCENE3D_HEADERS} ${SCENE_OBJECTS_HEADERS}
   ${SCENE_RESOURCES_HEADERS})
add_library(managerslib OBJECT ${MANAGERS_SOURCES} ${MANAGERS_HEADERS})
add_library(utilslib OBJECT ${UTILS_SOURCES} ${UTILS_HEADERS}
   ${UTILS_EXT_HEADERS})
add_library(gui2lib OBJECT ${UTILS_GUI2_SOURCES} ${UTILS_GUI2_HEADERS}
   ${UTILS_GUI2_WIDGETS_HEADERS})


# This is the binary that can be run directly.
# add_executable(gameplayfootball WIN32 ${CORE_SOURCES} ${CORE_HEADERS} ${AI_HEADERS} ${AI_SOURCES})
# target_link_libraries(gameplayfootball ${LIBRARIES})

# This will be the library that can be included from python.
add_library(game SHARED ${CORE_SOURCES} ${CORE_HEADERS} ${AI_HEADERS} ${AI_SOURCES})
target_link_libraries(game ${LIBRARIES})
