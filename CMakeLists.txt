cmake_minimum_required(VERSION 3.10)
project(Alcubierre LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Raylib paths
if (WIN32)
    set(RAYLIB_DIR "${CMAKE_SOURCE_DIR}/build/raylib-5.5_win64_mingw-w64")
elseif (APPLE)
    set(RAYLIB_DIR "${CMAKE_SOURCE_DIR}/build/raylib-5.5_macos")
endif()
set(RAYLIB_INCLUDE_DIR "${RAYLIB_DIR}/include")
set(RAYLIB_LIB_DIR "${RAYLIB_DIR}/lib")

# Include directories
include_directories(${RAYLIB_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    lib/rendering/warp.cpp
    lib/window.cpp
    lib/camera.cpp
    lib/laser_beam.cpp
    lib/projectile.cpp
    lib/star.cpp
    lib/light_base.cpp
    lib/lighting_system.cpp
    lib/shader_loader.cpp
    lib/skybox.cpp
    lib/main.cpp
    lib/utils.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link raylib and required system libraries
target_link_directories(${PROJECT_NAME} PRIVATE ${RAYLIB_LIB_DIR})
if (WIN32)
    target_link_libraries(${PROJECT_NAME}
        raylib
        winmm
    )
elseif (APPLE)
    target_link_libraries(${PROJECT_NAME}
        raylib
    )
endif()

# Copy shaders to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
