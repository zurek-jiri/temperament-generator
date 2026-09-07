cmake_minimum_required(VERSION 3.22)
project(TemperamentGenerator VERSION 1.4.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(JUCE_PATH "" CACHE PATH "Optional path to an existing JUCE 8.0.12 checkout")

add_library(temperament_core Source/Temperament.cpp Source/Expression.cpp Source/Automatic.cpp Source/Harmony.cpp)
target_include_directories(temperament_core PUBLIC Source)
if(MSVC)
    target_compile_options(temperament_core PUBLIC /EHsc)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(TemperamentTests Tests/TemperamentTests.cpp)
    target_link_libraries(TemperamentTests PRIVATE temperament_core)
    add_test(NAME TemperamentMath COMMAND TemperamentTests)
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
    juce_add_binary_data(TemperamentLegal HEADER_NAME AppLegal.h NAMESPACE AppLegal
        SOURCES LICENSE Licenses/About.txt Licenses/ThirdPartyNotices.txt)
    juce_add_gui_app(TemperamentGenerator
        PRODUCT_NAME "Temperament Generator"
        COMPANY_NAME "zurek-jiri"
        ICON_BIG "Source/TuningFork.svg"
        ICON_SMALL "Source/TuningFork.svg"
        BUNDLE_ID "io.github.zurek-jiri.temperament-generator")
    target_sources(TemperamentGenerator PRIVATE Source/Main.cpp Source/MainComponent.cpp Source/CircleEditor.cpp Source/HarmonyView.cpp)
    target_compile_definitions(TemperamentGenerator PRIVATE
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:TemperamentGenerator,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="${PROJECT_VERSION}")
    target_link_libraries(TemperamentGenerator PRIVATE temperament_core TemperamentAssets TemperamentLegal juce::juce_gui_extra
        juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
endif()
