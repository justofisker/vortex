cmake_minimum_required(VERSION 3.22)
project(ExampleVortex)

add_executable(ExampleVortex
    "example/src/main.c"
    )

target_link_libraries(ExampleVortex PRIVATE vortex)

target_include_directories(ExampleVortex PRIVATE
    "engine/src"
    "engine/extern/SDL2/include"
    ) # TODO: Remove all of these includes by removing them from public API

add_subdirectory("engine")

file(COPY "example/assets" DESTINATION "${CMAKE_BINARY_DIR}")

if (UNIX)
    target_compile_options(vortex PUBLIC "-fsanitize=undefined")
    target_compile_options(ExampleVortex PUBLIC "-fsanitize=undefined")
    target_link_options(ExampleVortex PUBLIC "-fsanitize=undefined")
endif (UNIX)