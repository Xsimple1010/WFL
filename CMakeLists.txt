cmake_minimum_required(VERSION 3.10)

project(WFS)

add_executable(WFS 
    src/main.cpp
    src/CpuFeatures.cpp
    src/glad.c
    src/libretroClass.cpp
    src/debug.cpp
    src/audio.cpp
    src/video.cpp
    
    #headers
    include/videoDefs.hpp
    include/libretro.h
    
)

find_package(SDL2 REQUIRED)

target_include_directories(WFS
    PUBLIC include/
    PUBLIC ${SDL2_INCLUDE_DIRS}
)

link_libraries(include/glad/glad.h /src/glad.c)
target_link_libraries(WFS PUBLIC ${SDL2_LIBRARIES} SDL2::SDL2 SDL2::SDL2main)