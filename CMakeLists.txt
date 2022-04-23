cmake_minimum_required(VERSION 3.23)
project(vortex)

add_executable(vortex WIN32
    "src/main.c"
)

option(BUILD_SHARED_LIBS "" ON)
add_subdirectory("extern/SDL2")

target_link_libraries(vortex SDL2 SDL2main)

target_include_directories(vortex PUBLIC
    "extern/SDL2/include"
)

if (MSVC)
    set_target_properties(vortex PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT "vortex")
endif (MSVC)