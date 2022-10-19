cmake_minimum_required(VERSION 3.22)
project(vortex)

file(GLOB SHADERS "shaders/*.glsl")

find_package(Vulkan QUIET)

if (TARGET Vulkan::Vulkan)
    set(SHADER_COMPILER Vulkan::glslangValidator)
elseif (DEFINED ENV{VULKAN_SDK})
    set(SHADER_COMPILER "$ENV{VULKAN_SDK}/Bin/glslangValidator")
else()
    message(FATAL_ERROR "Failed to find installed Vulkan SDK")
endif ()

set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/assets/shaders")

foreach(SHADER IN LISTS SHADERS)
    get_filename_component(FILENAME ${SHADER} NAME_WLE)
    add_custom_command(OUTPUT ${SHADER_OUTPUT_DIR}/${FILENAME}.spv
            COMMAND ${SHADER_COMPILER} -V -o ${SHADER_OUTPUT_DIR}/${FILENAME}.spv ${SHADER}
            DEPENDS ${SHADER}
            COMMENT "Compiling ${FILENAME}.glsl")
    list(APPEND SPV_SHADERS ${SHADER_OUTPUT_DIR}/${FILENAME}.spv)
endForeach()

add_library(vortex STATIC
        "src/vortex.c"
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
        "src/render/mesh.c"
        "src/render/mesh.h"
        "src/ecs/ecs.c"
        "src/ecs/globals.c"
        "src/ecs/globals.h"
        "src/ecs/builtin.c"
        "src/ecs/builtin.h"
        "src/audio/audio.c"
        "src/audio/audio.h"
        "src/audio/globals.c"
        "src/audio/globals.h"
        "src/input/input.c"
        "src/input/input.h"
        ${SHADERS}
        )

add_dependencies(vortex shaders)

target_include_directories(vortex PRIVATE
        "extern/SDL2/include"
        "extern/stb"
        "extern/openal-soft/include"
        "extern/ogg/include"
        "extern/vorbis/include"
        "extern/assimp/include"
        volk_headers
        )

target_include_directories(vortex PUBLIC
    "extern/cglm/include"
    "include"
    )

target_link_libraries(vortex PRIVATE
        volk
        cglm
        spirv-reflect-static
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

option(BUILD_SHARED_LIBS "" OFF)

option(CGLM_SHARED "Shared build" OFF)
option(CGLM_STATIC "Static build" OFF)
option(CGLM_USE_C99 "" OFF)
option(CGLM_USE_TEST "Enable Tests" OFF)
add_subdirectory("extern/cglm/")

option(SPIRV_REFLECT_EXECUTABLE     "Build spirv-reflect executable" OFF)
option(SPIRV_REFLECT_EXAMPLES       "Build stripper examples" OFF)
option(SPIRV_REFLECT_STATIC_LIB     "Build a SPIRV-Reflect static library" ON)
add_subdirectory("extern/SPIRV-Reflect")

set(SDL_RENDER OFF CACHE BOOL "")
set(SDL_TIMER ON CACHE BOOL "")
set(SDL_HAPTICS OFF CACHE BOOL "")
set(SDL_FILESYSTEM OFF CACHE BOOL "")
set(SDL_SENSOR OFF CACHE BOOL "")
set(SDL_DLOPEN ON CACHE BOOL "")
set(SDL_AUDIO OFF CACHE BOOL "")
set(VIDEO_DUMMY OFF CACHE BOOL "")
set(DISKAUDIO OFF CACHE BOOL "")
set(DUMMYAUDIO OFF CACHE BOOL "")
set(HIDAPI OFF CACHE BOOL "")
set(VIDEO_OPENGL OFF CACHE BOOL "")
set(VIDEO_OPENGLES OFF CACHE BOOL "")
set(DIRECTX OFF CACHE BOOL "")
set(RENDER_D3D OFF CACHE BOOL "")
set(RENDER_METAL OFF CACHE BOOL "")
set(VIDEO_VIVANTE OFF CACHE BOOL "")
set(VIDEO_METAL OFF CACHE BOOL "")
set(VIDEO_VULKAN ON CACHE BOOL "")
set(VIDEO_WAYLAND OFF CACHE BOOL "")


target_link_libraries(vortex PRIVATE
    SDL2-static
    OpenAL
    ogg
    vorbis
    vorbisfile
    assimp
    )

add_subdirectory("extern/SDL2")

add_subdirectory("extern/ogg")

set(BUILD_TESTING OFF)
add_subdirectory("extern/vorbis")

target_compile_definitions(vortex PUBLIC AL_LIBTYPE_STATIC)
set(LIBTYPE "STATIC")
option(ALSOFT_EXAMPLES  "Build example programs"  OFF)
option(ALSOFT_UTILS "Build utility programs"  OFF)
option(ALSOFT_NO_CONFIG_UTIL "Disable building the alsoft-config utility" ON)
add_subdirectory("extern/openal-soft")

set(DISABLED_IMPORTERS "GLTF" "AMF" "3DS" "AC" "ASE" "ASSBIN" "B3D" "BVH" "COLLADA" "DXF" "CSM" "HMP" "IRRMESH" "IRR" "LWO" "LWS" "M3D" "MD2" "MD3" "MD5" "MDC" "MDL" "NFF" "NDO" "OFF" "OGRE" "OPENGEX" "PLY" "MS3D" "COB" "BLEND" "IFC" "XGL" "FBX" "Q3D" "Q3BSP" "RAW" "SIB" "SMD" "STL" "TERRAGEN" "3D" "X" "X3D" "3MF" "MMD")
foreach(IMPORTER ${DISABLED_IMPORTERS})
    set(ASSIMP_BUILD_${IMPORTER}_IMPORTER OFF)
endforeach()
OPTION(ASSIMP_BUILD_ASSIMP_TOOLS  "If the supplementary tools for Assimp are built in addition to the library."  OFF)
OPTION(ASSIMP_BUILD_TESTS  "If the test suite for Assimp is built in addition to the library."  OFF)
OPTION(ASSIMP_INSTALL  "Disable this if you want to use assimp as a submodule."  OFF)
OPTION(ASSIMP_NO_EXPORT  "Disable Assimp's export functionality."  ON)
add_subdirectory("extern/assimp")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")

if (TARGET Vulkan::Vulkan)
    target_link_libraries(vortex PUBLIC Vulkan::Vulkan)
elseif (DEFINED ENV{VULKAN_SDK})
    target_include_directories(vortex PUBLIC "$ENV{VULKAN_SDK}/include")
endif ()
