cmake_minimum_required(VERSION 3.10)
project(Alcubierre LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Raylib paths
if (WIN32)
    set(RAYLIB_DIR "${CMAKE_SOURCE_DIR}/build/raylib-5.5_win64_mingw-w64")
elseif (LINUX)
    set(RAYLIB_DIR "${CMAKE_SOURCE_DIR}/build/raylib-5.5_linux_amd64")
elseif (APPLE)
    set(RAYLIB_DIR "${CMAKE_SOURCE_DIR}/build/raylib-5.5_macos")
endif()
set(RAYLIB_INCLUDE_DIR "${RAYLIB_DIR}/include")
set(RAYLIB_LIB_DIR "${RAYLIB_DIR}/lib")

# Include directories
include_directories(${RAYLIB_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    lib/object/laser_beam.cpp
    lib/object/projectile.cpp
    lib/object/star.cpp
    lib/ui/menu.cpp
    lib/rendering/bloom.cpp
    lib/rendering/frame_graph.cpp
    lib/rendering/warp.cpp
    lib/rendering/fxaa.cpp
    lib/rendering/render_pass.cpp
    lib/rendering/render_target.cpp
    lib/rendering/smaa.cpp
    lib/rendering/texture_handle.cpp
    lib/utils/draw_texture.cpp
    lib/utils/material_manager.cpp
    lib/utils/shader_manager.cpp
    lib/window.cpp
    lib/camera.cpp
    lib/light_base.cpp
    lib/lighting_system.cpp
    lib/skybox.cpp
    lib/main.cpp
)

# Create executable
if (WIN32)
    add_executable(${PROJECT_NAME} ${SOURCES} ${CMAKE_SOURCE_DIR}/assets/logo/icon.rc)
else()
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()

# Link raylib and required system libraries
target_link_directories(${PROJECT_NAME} PRIVATE ${RAYLIB_LIB_DIR})
if (WIN32)
    target_link_libraries(${PROJECT_NAME}
        raylib
        winmm
    )
elseif (LINUX)
    target_link_libraries(${PROJECT_NAME}
        raylib
    )
elseif (APPLE)
    target_link_libraries(${PROJECT_NAME}
        raylib
    )
endif()

# Copy shaders to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
