cmake_minimum_required (VERSION 3.8)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Habilite a Recarga Din�mica para compiladores MSVC, se houver suporte.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(WFL)

set(WFL_SOURCE 
  src/WFL.cpp
  src/CpuFeatures.cpp
  
  src/glad.c
  src/libretroClass.cpp
  src/debug.cpp
  src/gameLoop.cpp
  src/audio.cpp
  src/video.cpp
  src/controller.cpp
  src/threads/threadIoEvents.cpp
  src/threads/threadGameLoop.cpp
  
  #OPENGL
  src/WFL_GL/wflGl.cpp
  
  #headers
  include/WFL.h
  include/gamecontrollerdb.txt
  include/videoDefs.hpp
  include/WFLdefs.hpp
  include/libretro.h
)

# set(MAKE_WFL_LIB  true)

if(MAKE_WFL_LIB)
add_library(WFL SHARED
  ${WFL_SOURCE}
)
else()
add_executable(WFL
  src/main.cpp
  ${WFL_SOURCE}
)
endif()


find_package(SDL2 REQUIRED)

target_include_directories(WFL
  PUBLIC include/
  PUBLIC ${SDL2_INCLUDE_DIRS}
)

link_libraries(include/glad/glad.h /src/glad.c)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET WFL PROPERTY CXX_STANDARD 20)
endif()


target_link_libraries(WFL PUBLIC ${SDL2_LIBRARIES} SDL2::SDL2 SDL2::SDL2main)