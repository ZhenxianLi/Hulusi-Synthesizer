cmake_minimum_required(VERSION 3.22)
project(HulusiVST VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
if(DEFINED JUCE_SOURCE_DIR AND EXISTS "${JUCE_SOURCE_DIR}/CMakeLists.txt")
  message(STATUS "Using local JUCE from JUCE_SOURCE_DIR=${JUCE_SOURCE_DIR}")
  add_subdirectory("${JUCE_SOURCE_DIR}" juce)
else()
  FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.3
  )
  FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(HulusiVST
    COMPANY_NAME "HulusiSynth"
    BUNDLE_ID "com.hulusi.synth"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Hlsi
    PLUGIN_CODE Hls1
    FORMATS VST3
    PRODUCT_NAME "HulusiVST"
)

target_sources(HulusiVST
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
)

target_include_directories(HulusiVST PRIVATE include)

target_compile_definitions(HulusiVST
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(HulusiVST
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

include(CPack)
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "hulusi-vst")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
