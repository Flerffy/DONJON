cmake_minimum_required(VERSION 3.21)
project(DONJON LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(raylib CONFIG QUIET)

if(NOT raylib_FOUND)
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
endif()

add_executable(donjon
    src/main.cpp
    src/game.cpp
    src/engine.cpp
    src/entities.cpp
    src/data.cpp
    src/util.cpp
)
target_link_libraries(donjon PRIVATE raylib)

if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
    add_custom_command(TARGET donjon POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:donjon>/assets")
endif()

if(MSVC)
    target_compile_options(donjon PRIVATE /W4 /permissive-)
else()
    target_compile_options(donjon PRIVATE -Wall -Wextra -Wpedantic)
endif()
