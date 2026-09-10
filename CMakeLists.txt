cmake_minimum_required(VERSION 3.22)
project(TemperamentGenerator VERSION 1.6.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(JUCE_PATH "" CACHE PATH "Optional path to an existing JUCE 8.0.12 checkout")

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Source/temperaments.csv")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/Source/temperaments.csv" TEMPERAMENT_CATALOGUE)
configure_file(Source/KnownTemperamentsData.h.in generated/KnownTemperamentsData.h @ONLY)
add_library(temperament_core Source/Temperament.cpp Source/Expression.cpp Source/Automatic.cpp Source/Harmony.cpp Source/Reconstruction.cpp Source/KnownTemperaments.cpp)
target_include_directories(temperament_core PUBLIC Source PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
if(MSVC)
    target_compile_options(temperament_core PUBLIC /EHsc)
endif()

include(CTest)
add_library(chord_renderer Source/ChordRenderer.cpp)
target_include_directories(chord_renderer PUBLIC Source)
if(MSVC)
    target_compile_options(chord_renderer PUBLIC /EHsc)
endif()
if(BUILD_TESTING)
    add_executable(TemperamentTests Tests/TemperamentTests.cpp)
    target_link_libraries(TemperamentTests PRIVATE temperament_core)
    add_test(NAME TemperamentMath COMMAND TemperamentTests)
    add_executable(ReconstructionTests Tests/ReconstructionTests.cpp)
    target_link_libraries(ReconstructionTests PRIVATE temperament_core)
    add_test(NAME ReconstructionAndClosure COMMAND ReconstructionTests)
    add_executable(ChordRendererTests Tests/ChordRendererTests.cpp)
    target_link_libraries(ChordRendererTests PRIVATE chord_renderer)
    add_test(NAME SampledChordRendering COMMAND ChordRendererTests)
endif()

option(BUILD_GUI "Build the JUCE desktop application" ON)
if(BUILD_GUI)
    if(JUCE_PATH)
        add_subdirectory("${JUCE_PATH}" "${CMAKE_BINARY_DIR}/JUCE" EXCLUDE_FROM_ALL)
    else()
        include(FetchContent)
        FetchContent_Declare(JUCE
            GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG 8.0.12
            GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(JUCE)
    endif()
    juce_add_binary_data(TemperamentAssets HEADER_NAME AppAssets.h NAMESPACE AppAssets
        SOURCES Source/TuningFork.svg)
    file(GLOB PRINCIPAL_SAMPLES CONFIGURE_DEPENDS Source/PrincipalSamples/note*.flac)
    juce_add_binary_data(PrincipalAssets HEADER_NAME PrincipalData.h NAMESPACE PrincipalData SOURCES ${PRINCIPAL_SAMPLES})
    juce_add_binary_data(TemperamentLegal HEADER_NAME AppLegal.h NAMESPACE AppLegal
        SOURCES LICENSE Licenses/About.txt Licenses/ThirdPartyNotices.txt)
    juce_add_gui_app(TemperamentGenerator
        PRODUCT_NAME "Temperament Generator"
        COMPANY_NAME "zurek-jiri"
        ICON_BIG "Source/TuningFork.svg"
        ICON_SMALL "Source/TuningFork.svg"
        BUNDLE_ID "io.github.zurek-jiri.temperament-generator")
    target_sources(TemperamentGenerator PRIVATE Source/Main.cpp Source/MainComponent.cpp Source/CircleEditor.cpp Source/CentDeviationView.cpp Source/HarmonyView.cpp Source/ClosingPanel.cpp Source/ChordPlayback.cpp Source/AudioChecks.cpp)
    target_compile_definitions(TemperamentGenerator PRIVATE
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_USE_OGGVORBIS=0 JUCE_USE_MP3AUDIOFORMAT=0 JUCE_ASIO=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:TemperamentGenerator,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="${PROJECT_VERSION}")
    target_link_libraries(TemperamentGenerator PRIVATE temperament_core chord_renderer PrincipalAssets TemperamentAssets TemperamentLegal juce::juce_gui_extra juce::juce_audio_devices juce::juce_audio_formats
        juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
endif()
