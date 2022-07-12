cmake_minimum_required(VERSION 3.22)
project(ExampleVortex)

add_executable(ExampleVortex
    "src/main.c"
    )

target_link_libraries(ExampleVortex PRIVATE vortex)

target_include_directories(ExampleVortex PRIVATE
    "../engine/src"
    "../engine/extern/SDL2/include"
    "../engine/extern/stb"
    "../engine/extern/openal-soft/include"
    "../engine/extern/ogg/include"
    "../engine/extern/vorbis/include"
    "../engine/extern/assimp/include"
    ) # TODO: Remove all of these includes by removing them from public API

add_subdirectory("../engine" "${CMAKE_CURRENT_BINARY_DIR}/engine")

file(COPY "assets" DESTINATION "${CMAKE_BINARY_DIR}")

if (UNIX)
    target_compile_options(vortex PUBLIC "-fsanitize=undefined")
    target_compile_options(ExampleVortex PUBLIC "-fsanitize=undefined")
    target_link_options(ExampleVortex PUBLIC "-fsanitize=undefined")
endif (UNIX)