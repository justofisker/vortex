cmake_minimum_required(VERSION 3.22)
project(vortex)

set(SHADER_DIR "shaders")
file(GLOB SHADERS ${SHADER_DIR}/*.glsl)

find_package(Vulkan QUIET)

if (TARGET Vulkan::Vulkan)
    set(SHADER_COMPILER Vulkan::glslangValidator)
elseif (DEFINED ENV{VULKAN_SDK})
    set(SHADER_COMPILER "$ENV{VULKAN_SDK}/Bin/glslangValidator")
endif ()

set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

foreach(SHADER IN LISTS SHADERS)
    get_filename_component(FILENAME ${SHADER} NAME_WLE)
    add_custom_command(OUTPUT ${SHADER_OUTPUT_DIR}/${FILENAME}.spv
            COMMAND ${SHADER_COMPILER} -V -o ${SHADER_OUTPUT_DIR}/${FILENAME}.spv ${SHADER}
            DEPENDS ${SHADER}
            COMMENT "Compiling ${FILENAME}.glsl")
    list(APPEND SPV_SHADERS ${SHADER_OUTPUT_DIR}/${FILENAME}.spv)
endForeach()

add_executable(vortex
        "src/main.c"
        "src/render/render.c"
        "src/render/render.h"
        "src/render/init.c"
        "src/render/init.h"
        "src/render/globals.c"
        "src/render/globals.h"
        "src/render/shader.c"
        "src/render/shader.h"
        "src/render/util.c"
        "src/render/util.h"
        "src/render/texture.c"
        "src/render/texture.h"
        "src/ecs/ecs.c"
        "src/ecs/ecs.h"
        "src/ecs/globals.c"
        "src/ecs/globals.h"
        "src/ecs/builtin.c"
        "src/ecs/builtin.h"
        "src/audio/audio.c"
        "src/audio/audio.h"
        "src/audio/globals.c"
        "src/audio/globals.h"
        "src/render/mesh.c"
        "src/render/mesh.h"
        ${SHADERS}
        )

add_dependencies(vortex shaders)

target_include_directories(vortex PRIVATE
        "extern/SDL2/include"
        "extern/stb"
        "extern/openal-soft/include"
        "extern/ogg/include"
        "extern/vorbis/include"
        volk_headers
        )

target_link_libraries(vortex PRIVATE
        SDL2
        SDL2main
        volk
        cglm
        spirv-reflect-static
        OpenAL
        ogg
        vorbis
        vorbisfile
        )

if (UNIX)
    target_link_libraries(vortex PUBLIC m)
endif (UNIX)

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})

if (WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif (APPLE)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_MACOS_MVK)
elseif (UNIX)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
endif ()
add_subdirectory("extern/volk")

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
option(BUILD_SHARED_LIBS "" ON)

add_subdirectory("extern/SDL2")

option(CGLM_SHARED "Shared build" OFF)
option(CGLM_STATIC "Static build" OFF)
option(CGLM_USE_C99 "" OFF)
option(CGLM_USE_TEST "Enable Tests" OFF)
add_subdirectory("extern/cglm/")

option(SPIRV_REFLECT_EXECUTABLE     "Build spirv-reflect executable" OFF)
option(SPIRV_REFLECT_EXAMPLES       "Build stripper examples" OFF)
option(SPIRV_REFLECT_STATIC_LIB     "Build a SPIRV-Reflect static library" ON)
add_subdirectory("extern/SPIRV-Reflect")

add_subdirectory("extern/ogg")
add_subdirectory("extern/vorbis")

add_subdirectory("extern/openal-soft")

if (TARGET Vulkan::Vulkan)
    target_link_libraries(vortex PUBLIC Vulkan::Vulkan)
elseif (DEFINED ENV{VULKAN_SDK})
    target_include_directories(vortex PUBLIC "$ENV{VULKAN_SDK}/include")
endif ()

if (MSVC)
    set_target_properties(vortex PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT "vortex")
endif (MSVC)