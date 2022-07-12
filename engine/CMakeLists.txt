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

set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")

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

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
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

set(ASSIMP_BUILD_AMF_IMPORTER OFF)
set(ASSIMP_BUILD_3DS_IMPORTER OFF)
set(ASSIMP_BUILD_AC_IMPORTER OFF)
set(ASSIMP_BUILD_ASE_IMPORTER OFF)
set(ASSIMP_BUILD_ASSBIN_IMPORTER OFF)
set(ASSIMP_BUILD_B3D_IMPORTER OFF)
set(ASSIMP_BUILD_BVH_IMPORTER OFF)
set(ASSIMP_BUILD_COLLADA_IMPORTER OFF)
set(ASSIMP_BUILD_DXF_IMPORTER OFF)
set(ASSIMP_BUILD_CSM_IMPORTER OFF)
set(ASSIMP_BUILD_HMP_IMPORTER OFF)
set(ASSIMP_BUILD_IRRMESH_IMPORTER OFF)
set(ASSIMP_BUILD_IRR_IMPORTER OFF)
set(ASSIMP_BUILD_LWO_IMPORTER OFF)
set(ASSIMP_BUILD_LWS_IMPORTER OFF)
set(ASSIMP_BUILD_M3D_IMPORTER OFF)
set(ASSIMP_BUILD_MD2_IMPORTER OFF)
set(ASSIMP_BUILD_MD3_IMPORTER OFF)
set(ASSIMP_BUILD_MD5_IMPORTER OFF)
set(ASSIMP_BUILD_MDC_IMPORTER OFF)
set(ASSIMP_BUILD_MDL_IMPORTER OFF)
set(ASSIMP_BUILD_NFF_IMPORTER OFF)
set(ASSIMP_BUILD_NDO_IMPORTER OFF)
set(ASSIMP_BUILD_OFF_IMPORTER OFF)
set(ASSIMP_BUILD_OGRE_IMPORTER OFF)
set(ASSIMP_BUILD_OPENGEX_IMPORTER OFF)
set(ASSIMP_BUILD_PLY_IMPORTER OFF)
set(ASSIMP_BUILD_MS3D_IMPORTER OFF)
set(ASSIMP_BUILD_COB_IMPORTER OFF)
set(ASSIMP_BUILD_BLEND_IMPORTER OFF)
set(ASSIMP_BUILD_IFC_IMPORTER OFF)
set(ASSIMP_BUILD_XGL_IMPORTER OFF)
set(ASSIMP_BUILD_FBX_IMPORTER OFF)
set(ASSIMP_BUILD_Q3D_IMPORTER OFF)
set(ASSIMP_BUILD_Q3BSP_IMPORTER OFF)
set(ASSIMP_BUILD_RAW_IMPORTER OFF)
set(ASSIMP_BUILD_SIB_IMPORTER OFF)
set(ASSIMP_BUILD_SMD_IMPORTER OFF)
set(ASSIMP_BUILD_STL_IMPORTER OFF)
set(ASSIMP_BUILD_TERRAGEN_IMPORTER OFF)
set(ASSIMP_BUILD_3D_IMPORTER OFF)
set(ASSIMP_BUILD_X_IMPORTER OFF)
set(ASSIMP_BUILD_X3D_IMPORTER OFF)
set(ASSIMP_BUILD_3MF_IMPORTER OFF)
set(ASSIMP_BUILD_MMD_IMPORTER OFF)
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
