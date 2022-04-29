cmake_minimum_required(VERSION 3.22)
project(vortex)

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
        )

target_include_directories(vortex PRIVATE
        "extern/SDL2/include"
        volk_headers
        )

target_link_libraries(vortex PRIVATE
        SDL2
        SDL2main
        volk
        cglm
        )

if (WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif (APPLE)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_MACOS_MVK)
elseif (UNIX)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
endif ()
add_subdirectory("extern/volk")

option(BUILD_SHARED_LIBS "" ON)
add_subdirectory("extern/SDL2")

option(CGLM_SHARED "Shared build" OFF)
option(CGLM_STATIC "Static build" ON)
option(CGLM_USE_C99 "" OFF)
option(CGLM_USE_TEST "Enable Tests" OFF)
add_subdirectory("extern/cglm/")

find_package(Vulkan QUIET)
if (TARGET Vulkan::Vulkan)
    target_link_libraries(vortex PUBLIC Vulkan::Vulkan)
elseif (DEFINED ENV{VULKAN_SDK})
    target_include_directories(vortex PUBLIC "$ENV{VULKAN_SDK}/include")
endif ()

if (MSVC)
    set_target_properties(vortex PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT "vortex")
endif (MSVC)