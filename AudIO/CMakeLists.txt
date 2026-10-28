# CMake simple example

## [main]

# Almost all CMake files should start with this
# You should always specify a range with the newest
# and oldest tested versions of CMake. This will ensure
# you pick up the best policies.
cmake_minimum_required(VERSION 3.1...3.25)

# This is your project statement. You should always list languages;
# Listing the version is nice here since it sets lots of useful variables
project( AudIO
  VERSION 1.0
  LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(UNIX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -Wno-unknown-pragmas -ffast-math -Ofast")
endif(UNIX)

if(MSVC)
   add_compile_options(
    $<$<CONFIG:Release>:/MT> # Runtime library: Multi-threaded
    $<$<CONFIG:RelWithDebInfo>:/MT> # Runtime library: Multi-threaded                           
    $<$<CONFIG:Debug>:/MTd> # Runtime library: Multi-threaded Debug
    /MP /Gm- /Zi
    /permissive-
    /fp:fast
   )   
endif(MSVC)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Ensures that we do an out of source build
MACRO(ENSURE_OUT_OF_SOURCE_BUILD MSG)
     STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}"
     "${CMAKE_BINARY_DIR}" insource)
     GET_FILENAME_COMPONENT(PARENTDIR ${CMAKE_SOURCE_DIR} PATH)
     STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}"
     "${PARENTDIR}" insourcesubdir)
    IF(insource OR insourcesubdir)
        MESSAGE(FATAL_ERROR "${MSG}")
    ENDIF(insource OR insourcesubdir)
ENDMACRO(ENSURE_OUT_OF_SOURCE_BUILD)

ENSURE_OUT_OF_SOURCE_BUILD(
    "${CMAKE_PROJECT_NAME} requires an out of source build."
)

# If you set any CMAKE_ variables, that can go here.
# (But usually don't do this, except maybe for C++ standard)
file(GLOB_RECURSE Includes "AudIO/Includes/*.h" "AudIO/Includes/*.hpp")
file(GLOB_RECURSE 3rdParty "AudIO/3rdparty/*.cpp" "AudIO/3rdparty/*.h" "AudIO/3rdparty/*.hpp")
file(GLOB_RECURSE core "AudIO/core/*.cpp" "AudIO/core/*.h")
file(GLOB_RECURSE daw "AudIO/Daw/*.cpp" "AudIO/Daw/*.h")
file(GLOB_RECURSE devices "AudIO/Devices/*.cpp" "AudIO/Devices/*.c" "AudIO/Devices/*.h")
file(GLOB_RECURSE dsp "AudIO/Dsp/*.cpp" "AudIO/Dsp/*.h")
file(GLOB_RECURSE objects "AudIO/Objects/*.cpp" "AudIO/Objects/*.h")

file(GLOB_RECURSE airwindows "AudIO/airwindows/airwindows_base.h" "AudIO/airwindows/audioeffectx.h" "AudIO/airwindows/airwindows.cpp" "AudIO/airwindows/airwindows2.cpp")

####
#### AudIO CORE...
####
add_library(AudIO_Core 
    ${Includes}
    ${3rdParty}
    ${core}
    ${devices}

    ${dsp}
    ${objects}
)

target_include_directories(AudIO_Core PUBLIC "AudIO/Includes" "AudIO/3rdparty" "AudIO/Devices" "AudIO/Objects" "AudIO/Dsp" )
target_include_directories(AudIO_Core PRIVATE "AudIO/core" )

####
#### DAW CORE...
####
add_library(DAW_Core 
    ${Includes}
    ${3rdParty}
    ${daw}
)

target_include_directories(DAW_Core PUBLIC "AudIO/Daw" )
target_include_directories(DAW_Core PRIVATE "AudIO/Includes" "AudIO/3rdparty" )

####
#### Airwindows Objects Library...
####
add_library(AirWindows_plugins 
    ${Includes}
    ${airwindows}
)

target_include_directories(AirWindows_plugins PUBLIC "" )
target_include_directories(AirWindows_plugins PRIVATE "AudIO/airwindows" "AudIO/Includes" )

####
#### <<< GROUPING >>>.
####
get_target_property(AUDIO_CORE_SOURCES AudIO_Core SOURCES)
get_target_property(DAW_CORE_SOURCES DAW_Core SOURCES)
get_target_property(AIRWINDOWS_SOURCES AirWindows_plugins SOURCES)
source_group(TREE ${CMAKE_SOURCE_DIR}/AudIO FILES ${AUDIO_CORE_SOURCES})
source_group(TREE ${CMAKE_SOURCE_DIR}/AudIO FILES ${DAW_CORE_SOURCES})
source_group(TREE ${CMAKE_SOURCE_DIR}/AudIO FILES ${AIRWINDOWS_SOURCES})

set_target_properties(AudIO_Core PROPERTIES FOLDER "Libraries" )
set_target_properties(DAW_Core PROPERTIES FOLDER "Libraries" )
set_target_properties(AirWindows_plugins PROPERTIES FOLDER "Libraries" )

target_compile_features(AudIO_Core PRIVATE cxx_std_17)
target_compile_features(DAW_Core PRIVATE cxx_std_17)
target_compile_features(AirWindows_plugins PRIVATE cxx_std_17)

# Find packages go here.
# Add 
option(AUDIO_BUILD_EXAMPLES "Build the standalone AudIO example programs" OFF)
if(AUDIO_BUILD_EXAMPLES)
    add_subdirectory(AudIO/Example)
endif()
