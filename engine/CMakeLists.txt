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

    "include/vortex/audio.h"
    "include/vortex/components.h"
    "include/vortex/ecs.h"
    "include/vortex/types.h"
    "include/vortex/vortex.h"
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
        
target_link_libraries(vortex PRIVATE
    SDL2-static
    OpenAL
    ogg
    vorbis
    vorbisfile
    assimp
    )

if (UNIX)
    target_link_libraries(vortex PUBLIC m)
endif (UNIX)

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")

if (TARGET Vulkan::Vulkan)
    target_link_libraries(vortex PUBLIC Vulkan::Vulkan)
elseif (DEFINED ENV{VULKAN_SDK})
    target_include_directories(vortex PRIVATE "$ENV{VULKAN_SDK}/include")
endif ()

option(BUILD_SHARED_LIBS "" OFF)

if (WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif (APPLE)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_MACOS_MVK)
elseif (UNIX)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
endif ()
add_subdirectory("extern/volk")

option(CGLM_SHARED                  "Shared build" OFF)
option(CGLM_STATIC                  "Static build" OFF)
option(CGLM_USE_C99                 "" OFF)
option(CGLM_USE_TEST                "Enable Tests" OFF)
add_subdirectory("extern/cglm/")

option(SPIRV_REFLECT_EXECUTABLE     "Build spirv-reflect executable" OFF)
option(SPIRV_REFLECT_EXAMPLES       "Build stripper examples" OFF)
option(SPIRV_REFLECT_STATIC_LIB     "Build a SPIRV-Reflect static library" ON)
add_subdirectory("extern/SPIRV-Reflect")

option(SDL_ATOMIC                   "Enabled the Atomic subsystem" OFF)
option(SDL_AUDIO                    "Enabled the Audio subsystem" OFF)
option(SDL_VIDEO                    "Enabled the Video subsystem" ON)
option(SDL_RENDER                   "Enabled the Render subsystem" OFF)
option(SDL_EVENTS                   "Enabled the Events subsystem" ON)
option(SDL_JOYSTICK                 "Enabled the Joystick subsystem" ON)
option(SDL_HAPTIC                   "Enabled the Haptic subsystem" ON)
option(SDL_HIDAPI                   "Enabled the Hidapi subsystem" ON)
option(SDL_POWER                    "Enabled the Power subsystem" OFF)
option(SDL_THREADS                  "Enabled the Threads subsystem" OFF)
option(SDL_TIMERS                   "Enabled the Timers subsystem" ON)
option(SDL_FILE                     "Enabled the File subsystem" OFF)
option(SDL_LOADSO                   "Enabled the Loadso subsystem" ON)
option(SDL_CPUINFO                  "Enabled the CPUinfo subsystem" OFF)
option(SDL_FILESYSTEM               "Enabled the Filesystem subsystem" OFF)
option(SDL_DLOPEN                   "Enabled the Dlopen subsystem" OFF)
option(SDL_SENSOR                   "Enabled the Sensor subsystem" OFF)
option(SDL_LOCALE                   "Enabled the Locale subsystem" OFF)
option(SDL_ARMSIMD                  "use SIMD assembly blitters on ARM" OFF)
option(SDL_ARMNEON                  "use NEON assembly blitters on ARM" OFF)
option(SDL_DISKAUDIO                "Support the disk writer audio driver" ON)
option(SDL_DUMMYAUDIO               "Support the dummy audio driver" ON)
option(SDL_DIRECTFB                 "Use DirectFB video driver" OFF)
option(SDL_DUMMYVIDEO               "Use dummy video driver" OFF)
option(SDL_OPENGL                   "Include OpenGL support" OFF)
option(SDL_OPENGLES                 "Include OpenGL ES support" OFF)
option(SDL_PTHREADS                 "Use POSIX threads for multi-threading" OFF)
option(SDL_PTHREADS_SEM             "Use pthread semaphores" OFF)
option(SDL_OSS                      "Support the OSS audio API" OFF)
option(SDL_ALSA                     "Support the ALSA audio API" OFF)
option(SDL_JACK                     "Support the JACK audio API" OFF)
option(SDL_ESD                      "Support the Enlightened Sound Daemon" OFF)
option(SDL_PIPEWIRE                 "Use Pipewire audio" OFF)
option(SDL_PULSEAUDIO               "Use PulseAudio" OFF)
option(SDL_ARTS                     "Support the Analog Real Time Synthesizer" OFF)
option(SDL_NAS                      "Support the NAS audio API" OFF)
option(SDL_SNDIO                    "Support the sndio audio API" OFF)
option(SDL_FUSIONSOUND              "Use FusionSound audio driver" OFF)
option(SDL_LIBSAMPLERATE            "Use libsamplerate for audio rate conversion" OFF)
option(SDL_RPATH                    "Use an rpath when linking SDL" ${UNIX_SYS})
option(SDL_CLOCK_GETTIME            "Use clock_gettime() instead of gettimeofday()" ${UNIX_SYS})
option(SDL_X11                      "Use X11 video driver" ${UNIX_SYS})
option(SDL_X11_SHARED               "Dynamically load X11 support" ON)
option(SDL_WAYLAND                  "Use Wayland video driver" ${UNIX_SYS})
option(SDL_WAYLAND_SHARED           "Dynamically load Wayland support" ON)
option(SDL_RPI                      "Use Raspberry Pi video driver" OFF)
option(SDL_COCOA                    "Use Cocoa video driver" OFF)
option(SDL_DIRECTX                  "Use DirectX for Windows audio/video" OFF)
option(SDL_XINPUT                   "Use Xinput for Windows" ON)
option(SDL_WASAPI                   "Use the Windows WASAPI audio driver" OFF)
option(SDL_RENDER_D3D               "Enable the Direct3D render driver" OFF)
option(SDL_RENDER_METAL             "Enable the Metal render driver" OFF)
option(SDL_VIVANTE                  "Use Vivante EGL video driver" OFF)
option(SDL_VULKAN                   "Enable Vulkan support" ON)
option(SDL_METAL                    "Enable Metal support" OFF)
option(SDL_KMSDRM                   "Use KMS DRM video driver" OFF)
option(SDL_OFFSCREEN                "Use offscreen video driver" OFF)
option(SDL_HIDAPI_JOYSTICK          "Use HIDAPI for low level joystick drivers" ON)
option(SDL_VIRTUAL_JOYSTICK         "Enable the virtual-joystick driver" ON)
option(SDL_ASAN                     "Use AddressSanitizer to detect memory errors" OFF)
add_subdirectory("extern/SDL2")

add_subdirectory("extern/ogg")

set(BUILD_TESTING OFF)
add_subdirectory("extern/vorbis")

target_compile_definitions(vortex PUBLIC AL_LIBTYPE_STATIC)
set(LIBTYPE "STATIC")
option(ALSOFT_EXAMPLES              "Build example programs"  OFF)
option(ALSOFT_UTILS                 "Build utility programs"  OFF)
option(ALSOFT_NO_CONFIG_UTIL        "Disable building the alsoft-config utility" ON)
add_subdirectory("extern/openal-soft")

set(DISABLED_IMPORTERS "GLTF" "AMF" "3DS" "AC" "ASE" "ASSBIN" "B3D" "BVH" "COLLADA" "DXF" "CSM" "HMP" "IRRMESH" "IRR" "LWO" "LWS" "M3D" "MD2" "MD3" "MD5" "MDC" "MDL" "NFF" "NDO" "OFF" "OGRE" "OPENGEX" "PLY" "MS3D" "COB" "BLEND" "IFC" "XGL" "FBX" "Q3D" "Q3BSP" "RAW" "SIB" "SMD" "STL" "TERRAGEN" "3D" "X" "X3D" "3MF" "MMD")
foreach(IMPORTER ${DISABLED_IMPORTERS})
    set(ASSIMP_BUILD_${IMPORTER}_IMPORTER OFF)
endforeach()
OPTION(ASSIMP_BUILD_ASSIMP_TOOLS    "If the supplementary tools for Assimp are built in addition to the library."  OFF)
OPTION(ASSIMP_BUILD_TESTS           "If the test suite for Assimp is built in addition to the library."  OFF)
OPTION(ASSIMP_INSTALL               "Disable this if you want to use assimp as a submodule."  OFF)
OPTION(ASSIMP_NO_EXPORT             "Disable Assimp's export functionality."  ON)
add_subdirectory("extern/assimp")
