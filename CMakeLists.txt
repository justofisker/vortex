cmake_minimum_required(VERSION 3.23)
project(vortex)

add_executable(vortex WIN32
    "src/main.c"
    "src/render/render.c"
    "src/render/render.h"
    "src/render/util.c"
    "src/render/util.h"
    "src/render/globals.c"
    "src/render/globals.h"
)

option(BUILD_SHARED_LIBS "" ON)
add_subdirectory("extern/SDL2")

if (WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif (APPLE)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_MACOS_MVK)
elseif (UNIX)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
endif ()
add_subdirectory("extern/volk")

target_link_libraries(vortex PRIVATE SDL2 SDL2main volk)

# If CMake has the FindVulkan module and it works, use it.
# Otherwise try the environment variable.
find_package(Vulkan QUIET)
if(TARGET Vulkan::Vulkan)
    message(" Vulkan as target")
  target_link_libraries(vortex PUBLIC Vulkan::Vulkan)
elseif(DEFINED ENV{VULKAN_SDK})
    message(" Vulkan as path")
  target_include_directories(vortex PUBLIC "$ENV{VULKAN_SDK}/include")
endif()

target_include_directories(vortex PRIVATE
    "extern/SDL2/include"
    volk_headers
)

if (MSVC)
    set_target_properties(vortex PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT "vortex")
endif (MSVC)