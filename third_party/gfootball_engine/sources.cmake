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

set(BASE_HEADERS
   src/base/log.hpp
   src/base/utils.hpp
   src/base/properties.hpp
   src/base/sdl_surface.hpp
)

set(BASE_GEOMETRY_HEADERS
   src/base/geometry/aabb.hpp
   src/base/geometry/trianglemeshutils.hpp
   src/base/geometry/plane.hpp
   src/base/geometry/triangle.hpp
   src/base/geometry/line.hpp
)

set(BASE_MATH_HEADERS
   src/base/math/quaternion.hpp
   src/base/math/matrix3.hpp
   src/base/math/matrix4.hpp
   src/base/math/vector3.hpp
   src/base/math/bluntmath.hpp
)

set(BASE_SOURCES
   src/base/sdl_surface.cpp
   src/base/utils.cpp
   src/base/properties.cpp
   src/base/log.cpp
   src/base/geometry/triangle.cpp
   src/base/geometry/line.cpp
   src/base/geometry/trianglemeshutils.cpp
   src/base/geometry/aabb.cpp
   src/base/geometry/plane.cpp
   src/base/math/vector3.cpp
   src/base/math/matrix3.cpp
   src/base/math/bluntmath.cpp
   src/base/math/quaternion.cpp
   src/base/math/matrix4.cpp
)

set(SYSTEMS_COMMON_HEADERS
   src/systems/isystem.hpp
   src/systems/isystemobject.hpp
)

set(SYSTEMS_GRAPHICS_HEADERS
   src/systems/graphics/graphics_task.hpp
   src/systems/graphics/graphics_scene.hpp
   src/systems/graphics/graphics_object.hpp
   src/systems/graphics/graphics_system.hpp
)

set(SYSTEMS_GRAPHICS_OBJECTS_HEADERS
   src/systems/graphics/objects/graphics_overlay2d.hpp
   src/systems/graphics/objects/graphics_camera.hpp
   src/systems/graphics/objects/graphics_light.hpp
   src/systems/graphics/objects/graphics_geometry.hpp
)

set(SYSTEMS_GRAPHICS_RESOURCES_HEADERS
   src/systems/graphics/resources/vertexbuffer.hpp
   src/systems/graphics/resources/texture.hpp
)

set(SYSTEMS_GRAPHICS_RENDERING_HEADERS
   src/systems/graphics/rendering/interface_renderer3d.hpp
   src/systems/graphics/rendering/opengl_renderer3d.hpp
)

set(SYSTEMS_GRAPHICS_SOURCES
   src/systems/graphics/graphics_object.cpp
   src/systems/graphics/graphics_task.cpp
   src/systems/graphics/objects/graphics_geometry.cpp
   src/systems/graphics/objects/graphics_camera.cpp
   src/systems/graphics/objects/graphics_light.cpp
   src/systems/graphics/objects/graphics_overlay2d.cpp
   src/systems/graphics/graphics_scene.cpp
   src/systems/graphics/resources/vertexbuffer.cpp
   src/systems/graphics/resources/texture.cpp
   src/systems/graphics/rendering/opengl_renderer3d.cpp
   src/systems/graphics/graphics_system.cpp
)

set(LOADERS_HEADERS
   src/loaders/aseloader.hpp
   src/loaders/imageloader.hpp
)

set(LOADERS_SOURCES
   src/loaders/imageloader.cpp
   src/loaders/aseloader.cpp
)

set(TYPES_HEADERS
   src/types/subject.hpp
   src/types/spatial.hpp
   src/types/resource.hpp
   src/types/material.hpp
   src/types/observer.hpp
   src/types/interpreter.hpp
   src/types/refcounted.hpp
   src/types/command.hpp
   src/types/loader.hpp
   src/types/messagequeue.hpp
)

set(TYPES_SOURCES
   src/types/spatial.cpp
   src/types/refcounted.cpp
   src/types/observer.cpp
   src/types/command.cpp
)

set(SCENE_HEADERS
   src/scene/scene.hpp
   src/scene/iscene.hpp
   src/scene/object.hpp
   src/scene/objectfactory.hpp
)

set(SCENE2D_HEADERS
   src/scene/scene2d/scene2d.hpp
)

set(SCENE_OBJECTS_HEADERS
   src/scene/objects/skybox.hpp
   src/scene/objects/geometry.hpp
   src/scene/objects/light.hpp
   src/scene/objects/image2d.hpp
   src/scene/objects/camera.hpp
)

set(SCENE3D_HEADERS
   src/scene/scene3d/scene3d.hpp
   src/scene/scene3d/node.hpp
)

set(SCENE_RESOURCES_HEADERS
   src/scene/resources/geometrydata.hpp
   src/scene/resources/surface.hpp
)

set(SCENE_SOURCES
   src/scene/objectfactory.cpp
   src/scene/scene2d/scene2d.cpp
   src/scene/scene.cpp
   src/scene/objects/image2d.cpp
   src/scene/objects/light.cpp
   src/scene/objects/geometry.cpp
   src/scene/objects/skybox.cpp
   src/scene/objects/camera.cpp
   src/scene/scene3d/scene3d.cpp
   src/scene/scene3d/node.cpp
   src/scene/object.cpp
   src/scene/resources/surface.cpp
   src/scene/resources/geometrydata.cpp
)

set(MANAGERS_HEADERS
   src/managers/resourcemanager.hpp
)

set(UTILS_HEADERS
   src/utils/animation.hpp
   src/utils/objectloader.hpp
   src/utils/xmlloader.hpp
   src/utils/splitgeometry.hpp
   src/utils/orbitcamera.hpp
)

set(UTILS_EXT_HEADERS
   src/utils/animationextensions/animationextension.hpp
   src/utils/animationextensions/footballanimationextension.hpp
)

set(UTILS_SOURCES
   src/utils/orbitcamera.cpp
   src/utils/animation.cpp
   src/utils/splitgeometry.cpp
   src/utils/objectloader.cpp
   src/utils/xmlloader.cpp
   src/utils/animationextensions/footballanimationextension.cpp
)

set(UTILS_GUI2_HEADERS
   src/utils/gui2/windowmanager.hpp
   src/utils/gui2/page.hpp
   src/utils/gui2/style.hpp
   src/utils/gui2/guitask.hpp
   src/utils/gui2/view.hpp
)

set(UTILS_GUI2_WIDGETS_HEADERS
   src/utils/gui2/widgets/image.hpp
   src/utils/gui2/widgets/caption.hpp
   src/utils/gui2/widgets/frame.hpp
   src/utils/gui2/widgets/root.hpp
)

set(UTILS_GUI2_SOURCES
   src/utils/gui2/style.cpp
   src/utils/gui2/widgets/caption.cpp
   src/utils/gui2/widgets/image.cpp
   src/utils/gui2/widgets/root.cpp
   src/utils/gui2/widgets/frame.cpp
   src/utils/gui2/view.cpp
   src/utils/gui2/windowmanager.cpp
   src/utils/gui2/guitask.cpp
   src/utils/gui2/page.cpp
)

set(BLUNTED_CORE_HEADERS
   src/defines.hpp
   src/blunted.hpp
)

set(BLUNTED_CORE_SOURCES
   src/blunted.cpp
)


###### SEPARATION

set(AI_HEADERS
  ai.cpp
  src/ai/ai_keyboard.hpp
  src/game_env.cpp
)

set(AI_SOURCES
  ai.hpp
  src/ai/ai_keyboard.cpp
  src/game_env.hpp
)

set(CLIENT_SOURCES
   src/client.cpp
   src/game_env.hpp
)

set(CORE_HEADERS
   src/cmake/backtrace.h
   src/cmake/file.h
   src/gamedefines.hpp
   src/utils.hpp
   src/main.hpp
   src/gametask.hpp
   src/misc/hungarian.h
)

set(CORE_SOURCES
   src/cmake/backtrace.cpp
   src/cmake/file.cpp
   src/misc/perlin.cpp
   src/misc/hungarian.cpp
   src/gametask.cpp
   src/utils.cpp
   src/main.cpp
   src/gamedefines.cpp
   src/defines.cpp
)

set(GAME_HEADERS
   src/onthepitch/humangamer.hpp
   src/onthepitch/officials.hpp
   src/onthepitch/player/humanoid/humanoidbase.hpp
   src/onthepitch/player/humanoid/humanoid.hpp
   src/onthepitch/player/humanoid/animcollection.hpp
   src/onthepitch/player/humanoid/humanoid_utils.hpp
   src/onthepitch/player/playerofficial.hpp
   src/onthepitch/player/playerbase.hpp
   src/onthepitch/player/player.hpp
   src/onthepitch/player/controller/icontroller.hpp
   src/onthepitch/player/controller/elizacontroller.hpp
   src/onthepitch/player/controller/humancontroller.hpp
   src/onthepitch/player/controller/playercontroller.hpp
   src/onthepitch/player/controller/strategies/strategy.hpp
   src/onthepitch/player/controller/strategies/offtheball/default_off.hpp
   src/onthepitch/player/controller/strategies/offtheball/default_def.hpp
   src/onthepitch/player/controller/strategies/offtheball/default_mid.hpp
   src/onthepitch/player/controller/strategies/offtheball/goalie_default.hpp
   src/onthepitch/player/controller/refereecontroller.hpp
   src/onthepitch/referee.hpp
   src/onthepitch/ball.hpp
   src/onthepitch/team.hpp
   src/onthepitch/match.hpp
   src/onthepitch/AIsupport/AIfunctions.hpp
   src/onthepitch/AIsupport/mentalimage.hpp
   src/onthepitch/teamAIcontroller.hpp
   src/onthepitch/proceduralpitch.hpp
)

set(GAME_SOURCES
   src/onthepitch/officials.cpp
   src/onthepitch/player/humanoid/humanoid_utils.cpp
   src/onthepitch/player/humanoid/animcollection.cpp
   src/onthepitch/player/humanoid/humanoidbase.cpp
   src/onthepitch/player/humanoid/humanoid.cpp
   src/onthepitch/player/playerofficial.cpp
   src/onthepitch/player/player.cpp
   src/onthepitch/player/playerbase.cpp
   src/onthepitch/player/controller/playercontroller.cpp
   src/onthepitch/player/controller/humancontroller.cpp
   src/onthepitch/player/controller/icontroller.cpp
   src/onthepitch/player/controller/refereecontroller.cpp
   src/onthepitch/player/controller/elizacontroller.cpp
   src/onthepitch/player/controller/strategies/strategy.cpp
   src/onthepitch/player/controller/strategies/offtheball/default_mid.cpp
   src/onthepitch/player/controller/strategies/offtheball/default_off.cpp
   src/onthepitch/player/controller/strategies/offtheball/default_def.cpp
   src/onthepitch/player/controller/strategies/offtheball/goalie_default.cpp
   src/onthepitch/humangamer.cpp
   src/onthepitch/ball.cpp
   src/onthepitch/match.cpp
   src/onthepitch/referee.cpp
   src/onthepitch/AIsupport/mentalimage.cpp
   src/onthepitch/AIsupport/AIfunctions.cpp
   src/onthepitch/proceduralpitch.cpp
   src/onthepitch/team.cpp
   src/onthepitch/teamAIcontroller.cpp
)

set(MENU_HEADERS
   src/menu/pagefactory.hpp
   src/menu/startmatch/loadingmatch.hpp
   src/menu/menutask.hpp
   src/menu/ingame/gamepage.hpp
   src/menu/ingame/scoreboard.hpp
   src/menu/ingame/radar.hpp
)

set(MENU_SOURCES
   src/menu/startmatch/loadingmatch.cpp
   src/menu/pagefactory.cpp
   src/menu/menutask.cpp
   src/menu/ingame/radar.cpp
   src/menu/ingame/gamepage.cpp
   src/menu/ingame/scoreboard.cpp
)

set(DATA_HEADERS
   src/data/matchdata.hpp
   src/data/teamdata.hpp
   src/data/playerdata.hpp
)

set(DATA_SOURCES
   src/data/matchdata.cpp
   src/data/playerdata.cpp
   src/data/teamdata.cpp
)
