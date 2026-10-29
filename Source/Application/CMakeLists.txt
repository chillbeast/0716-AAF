
cmake_minimum_required(VERSION 3.20)
project(IgniteDevelopment)
include_guard()
## CMake package management...
include(CPM.cmake)

# Add or Disable optional components
option( USE_VALUE_TREE_DEBUGGER "Value Tree Debugger" OFF )
option( IGNITE_VST_PLUGIN "IGNITE VST Plugin" OFF )
option( TIMESTAMP "Refresh build timestamp on every build" OFF )
option( IGNITE_WARNINGS_AS_ERRORS "Treat MSVC release warnings as errors" OFF )
option( USE_ASAN_IGNITE "Enable AddressSanitizer for the application and tests" OFF )
function(ignite_enable_sanitizer target)
    if(USE_ASAN_IGNITE)
        if(MSVC)
            target_compile_options(${target} PRIVATE /fsanitize=address /Zi)
            target_link_options(${target} PRIVATE /INCREMENTAL:NO /DEBUG)
        else()
            target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE -fsanitize=address)
        endif()
    endif()
endfunction()
if(CMAKE_CROSSCOMPILING)
    set(IGNITE_TESTS_DEFAULT OFF)
else()
    set(IGNITE_TESTS_DEFAULT ON)
endif()
option( IGNITE_DESKTOP_TESTS "Build desktop audio and SoundFont regression checks" ${IGNITE_TESTS_DEFAULT} )
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
#Compile commands, useful for some IDEs like VS-Code
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

if(WIN32)
    include(cmake/FluidSynth.cmake)
endif()

#Adds all the module sources so they appear correctly in the IDE
set_property(GLOBAL PROPERTY USE_FOLDERS YES)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)

if(USE_VALUE_TREE_DEBUGGER)
  add_definitions(-DUSE_VALUE_TREE_DEBUGGER=0)
endif()

set(ENABLE_HARDWARE_TEST TRUE)

if(ENABLE_HARDWARE_TEST)
    add_definitions(-DENABLE_HARDWARE_TEST=1)
else()
    add_definitions(-DENABLE_HARDWARE_TEST=0)
endif()


if("${ENABLE_OPTICK}" STREQUAL "")
    set(USE_OPTICK_VAL 0)
else()
    set(USE_OPTICK_VAL ${ENABLE_OPTICK})
endif()

set(USE_VST 0)
set(USE_AU 0)

if(BUILD_VST)
    set(USE_VST 1)
endif()

# If we are using MSVC we want static runtime linkage
if (MSVC)
    # Adding standards conformance flag
    # Catches errors skipped in regular Windows build
    ## Link https: //learn.microsoft.com/en-us/cpp/build/reference/permissive-standards-conformance?view=msvc-170
    add_compile_options(/permissive-)

    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(
        $<$<CONFIG:Release>:/MT> # Runtime library: Multi-threaded
        $<$<CONFIG:RelWithDebInfo>:/MT> # Runtime library: Multi-threaded                           
        $<$<CONFIG:Debug>:/MTd> # Runtime library: Multi-threaded Debug
	#$<$<CONFIG:Debug>:/MP> # Compile with Multiple Core Debug
        /MP /Gm- /Zi
        /wd4100
    )
endif()

# If we are compiling for macOS we want to target OS versions down to 10.11
if (APPLE)
    set (CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE INTERNAL "")
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

endif()

# Compile with werror in release builds
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
    CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -Wno-unknown-pragmas -ffast-math -Ofast" )
    #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Werror")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    if(IGNITE_WARNINGS_AS_ERRORS)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /WX")
    endif()
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})


# Fetch JUCE from respective git repositories...
# IF you want to instead point it to a local version, you can invoke CMake with  -DCPM_JUCE_SOURCE="Path_To_JUCE_git_repo"
CPMAddPackage( 
    NAME JUCE
    GITHUB_REPOSITORY juce-framework/JUCE
    GIT_TAG 7.0.12
    DOWNLOAD_ONLY YES
)

if(JUCE_ADDED)
    find_package(Git REQUIRED)
    set(JUCE_PATCH "${CMAKE_SOURCE_DIR}/patch/juce_multitouch.patch")
    execute_process(COMMAND "${GIT_EXECUTABLE}" apply --reverse --check "${JUCE_PATCH}"
        WORKING_DIRECTORY "${JUCE_SOURCE_DIR}" RESULT_VARIABLE patch_present
        OUTPUT_QUIET ERROR_QUIET)
    if(NOT patch_present EQUAL 0)
        execute_process(COMMAND "${GIT_EXECUTABLE}" apply "${JUCE_PATCH}"
            WORKING_DIRECTORY "${JUCE_SOURCE_DIR}" RESULT_VARIABLE patch_result)
        if(NOT patch_result EQUAL 0)
            message(FATAL_ERROR "Failed to apply the IGNITE JUCE patch: ${patch_result}")
        endif()
    endif()
    add_subdirectory("${JUCE_SOURCE_DIR}" ./cmake_build_juce)
endif()

# Add JUCE directory...
if(TIMESTAMP)
    add_subdirectory(timestamp)
endif()
add_subdirectory(modules)
add_subdirectory(apps)
if(IGNITE_DESKTOP_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Add CPack support
set(CPACK_GENERATOR "7Z")
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_SOURCE_DIR}/.build_cache/dist)

set(CPACK_STRIP_FILES YES)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS YES)

set(CPACK_SOURCE_IGNORE_FILES
    /.git
    /dist
    /.*build.*
    /\\\\.DS_Store
    /JUCE
)
set(
    CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)

include(CPack)
