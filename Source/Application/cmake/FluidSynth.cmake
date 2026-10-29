# The desktop uses the full SF2 engine; no audio/MIDI drivers are created.
set(IGNITE_FLUIDSYNTH_COMMIT 71e85b2ca6bf48641ba7e2261d8f2640ae473773)
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/fluidsynth/CMakeLists.txt")
    set(CPM_IgniteFluidSynth_SOURCE "${CMAKE_SOURCE_DIR}/third_party/fluidsynth")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/build/deps/fluidsynth/CMakeLists.txt")
    set(CPM_IgniteFluidSynth_SOURCE "${CMAKE_SOURCE_DIR}/build/deps/fluidsynth")
endif()
CPMAddPackage(NAME IgniteFluidSynth GITHUB_REPOSITORY FluidSynth/fluidsynth
    GIT_TAG ${IGNITE_FLUIDSYNTH_COMMIT} DOWNLOAD_ONLY YES)
# Upstream always registers its optional tests. Keep them out of Ignite's CTest
# project; the distribution includes the patched library's corresponding source.
file(READ "${IgniteFluidSynth_SOURCE_DIR}/CMakeLists.txt" fluid_cmake)
if(NOT fluid_cmake MATCHES "IGNITE_EMBEDDED_SOUNDFONT")
string(REPLACE "add_subdirectory ( test )\nadd_subdirectory ( doc )"
    "if(NOT IGNITE_EMBEDDED_SOUNDFONT)\nadd_subdirectory ( test )\nadd_subdirectory ( doc )\nendif()"
    patched_fluid_cmake "${fluid_cmake}")
if(NOT fluid_cmake STREQUAL patched_fluid_cmake)
    file(WRITE "${IgniteFluidSynth_SOURCE_DIR}/CMakeLists.txt" "${patched_fluid_cmake}")
endif()
endif()
set(IGNITE_EMBEDDED_SOUNDFONT ON)
set(BUILD_SHARED_LIBS OFF)
foreach(feature native-dls signalsmith libsndfile aufile network readline openmp
        dsound wasapi waveout winmidi sdl3 ladspa alsa jack dbus pipewire pulseaudio oss)
    set(enable-${feature} OFF CACHE BOOL "Unused in Ignite's embedded SoundFont engine" FORCE)
endforeach()
add_subdirectory("${IgniteFluidSynth_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/soundfont-engine" EXCLUDE_FROM_ALL)
if(MSVC)
    set_property(TARGET libfluidsynth-OBJ libfluidsynth PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
ignite_enable_sanitizer(libfluidsynth-OBJ)
add_library(IgniteSoundFontRuntime INTERFACE)
target_link_libraries(IgniteSoundFontRuntime INTERFACE libfluidsynth)
target_include_directories(IgniteSoundFontRuntime INTERFACE
    "${IgniteFluidSynth_SOURCE_DIR}/include" "${CMAKE_BINARY_DIR}/soundfont-engine/include")
target_compile_definitions(IgniteSoundFontRuntime INTERFACE IGNITE_FULL_SF2=1 FLUIDSYNTH_NOT_A_DLL)
set(IGNITE_FLUIDSYNTH_SOURCE_DIR "${IgniteFluidSynth_SOURCE_DIR}" CACHE PATH "Corresponding SoundFont engine source" FORCE)
