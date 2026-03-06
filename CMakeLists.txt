cmake_minimum_required(VERSION 3.21)
project(DONJON LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# Keep the raylib build lean for this project.
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)

add_executable(donjon src/main.cpp)
target_link_libraries(donjon PRIVATE raylib)

if(MSVC)
    target_compile_options(donjon PRIVATE /W4 /permissive-)
else()
    target_compile_options(donjon PRIVATE -Wall -Wextra -Wpedantic)
endif()
