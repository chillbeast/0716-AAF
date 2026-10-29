cmake_minimum_required(VERSION 3.15)

## Read version information from the file...
file(READ "version.txt" ver)

## Parse version info, and read variables...
string(REGEX MATCH "VERSION_MAJOR ([0-9]*)" _ ${ver})
set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1})

string(REGEX MATCH "VERSION_MINOR ([0-9]*)" _ ${ver})
set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_1})

string(REGEX MATCH "VERSION_PATCH ([0-9]*)" _ ${ver})
set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_1})

## Setup target configuration...
set(TargetName      Ignite)         # Project name
set(TargetCompany  MusicTribe)
set(TargetVersion  ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

project(${TargetName} VERSION ${TargetVersion})

## Assets Directory
file(COPY "${CMAKE_SOURCE_DIR}/assets" DESTINATION "${CMAKE_BINARY_DIR}")
set(TargetSrc_DIR ".")

## Generate configuration file...
configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

message( "## Project: ${TargetName}" ) 
message( "## Version: ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" )


set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(APPLICATION_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../.. )
set(COMMON_DIRECTORY ${APPLICATION_DIRECTORY}/modules/Common)
#set(TSF_DIRECTORY ${COMMON_DIRECTORY}/Thirdparty/TinySoundFont)
set(BOARD_DIRECTORY ${COMMON_DIRECTORY}/Board)
set(MODULE_DIRECTORY ${COMMON_DIRECTORY}/Modules)
set(FIRMWARE_DIRECTORY ${APPLICATION_DIRECTORY}/modules/Firmware)

juce_add_gui_app(${TargetName} 
        PRODUCT_NAME ${TargetName}
        COMPANY_NAME ${TargetCompany}
        REQUIRES_FULL_SCREEN TRUE
)

juce_generate_juce_header(${TargetName})

set_property(TARGET ${TargetName} 
        PROPERTY CXX_STANDARD 17 
)

set_target_properties(${TargetName} PROPERTIES 
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_include_directories(${TargetName}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
	#${TSF_DIRECTORY}
        ${BOARD_DIRECTORY}
        ${MODULE_DIRECTORY}
        ${FIRMWARE_DIRECTORY}/Application/Model

)

target_sources(${TargetName} 
    PRIVATE
        Main.cpp
)

target_compile_definitions(${TargetName} 
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:${TargetName},JUCE_PROJECT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:${TargetName},JUCE_VERSION>"

        JUCER_ENABLE_GPL_MODE=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_DISPLAY_SPLASH_SCREEN=0

        JUCE_MODAL_LOOPS_PERMITTED=1
        JUCE_STRICT_REFCOUNTEDPOINTER=1

        JUCE_ASIO=0

        JUCE_PLUGINHOST_LADSPA=0
        JUCE_PLUGINHOST_VST=0
        JUCE_PLUGINHOST_VST3=${USE_VST}
        JUCE_PLUGINHOST_AU=${USE_AU}
        JUCE_ENABLE_REPAINT_DEBUGGING=0
        THPAR_SQLITE=1
        USE_OPTICK=${USE_OPTICK_VAL}        
)

target_link_libraries(${TargetName} 
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags

    PRIVATE

        juce::juce_gui_extra
        juce::juce_audio_basics
#juce_audio_formats - enable only for testing with sample file, reffer SongManager.cpp
        #juce::juce_audio_formats
        juce::juce_audio_devices
        juce::juce_audio_utils
        juce::juce_dsp
        common_defs
        common_ui_controls
        ignite_ui
        Common
	    Firmware
	    #tsf
)

if(TIMESTAMP)
    target_link_libraries(${TargetName} PRIVATE timestamp)
endif()

ignite_enable_sanitizer(${TargetName})
if(WIN32)
    target_link_libraries(${TargetName} PRIVATE IgniteSoundFontRuntime)
endif()

## CPack variables...
set(CPACK_PACKAGE_NAME ${PROJECT_NAME} )
set(CPACK_PACKAGE_VENDOR "MusicTribe Innovation Pvt. Ltd.")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

## Create Install package...
install(
    TARGETS ${TargetName}
    EXPORT "${TargetName}Targets" DESTINATION "."
    COMPONENT ${TargetName} # must be here, not any line lower
)
## Add resource directory...
install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/assets" DESTINATION "."
    PATTERN "${CMAKE_SOURCE_DIR}/assets/*"
)

# Copy assets to executable directory post generating build files...
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/../../assets" "$<TARGET_FILE_DIR:Ignite>/assets"
)

# The SoundFont is not part of the open-source release; copy it only if one was supplied.
set(IGNITE_SOUNDFONT "${CMAKE_CURRENT_SOURCE_DIR}/../../sf/Ignite.sf2" CACHE FILEPATH "Optional SoundFont to copy beside Ignite")
if(EXISTS ${IGNITE_SOUNDFONT})
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${IGNITE_SOUNDFONT}" "$<TARGET_FILE_DIR:Ignite>/Ignite.sf2"
    )
else()
    message(STATUS "No SoundFont at ${IGNITE_SOUNDFONT}; the app will start without internal sounds.")
endif()
