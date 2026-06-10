cmake_minimum_required(VERSION 3.16)
project(NAMVST3 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# SDK paths - use the full VST3 SDK with submodules
set(VST3_SDK_PATH "/home/chopiou/vst3sdk" CACHE PATH "Path to VST3 SDK root (must contain pluginterfaces, base, public.sdk)")

# Plugin configuration
set(PLUGIN_NAME "NAMLoader")

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(X11 REQUIRED)

# Eigen3 - use include directories directly
include_directories(/usr/include/eigen3)

# VST3 SDK core sources (excluding vstguieditor which requires VSTGUI)
set(VST3_SOURCES
    # Main entry points
    ${VST3_SDK_PATH}/public.sdk/source/main/pluginfactory.cpp
    ${VST3_SDK_PATH}/public.sdk/source/main/moduleinit.cpp
    ${VST3_SDK_PATH}/public.sdk/source/main/linuxmain.cpp
    
    # Common utilities
    ${VST3_SDK_PATH}/public.sdk/source/common/memorystream.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/commonstringconvert.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/pluginview.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/readfile.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/systemclipboard_linux.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/threadchecker_linux.cpp
    
    # VST base classes (excluding vstguieditor.cpp)
    ${VST3_SDK_PATH}/public.sdk/source/vst/vstcomponent.cpp
    ${VST3_SDK_PATH}/public.sdk/source/vst/vstcomponentbase.cpp
    ${VST3_SDK_PATH}/public.sdk/source/vst/vstbus.cpp
    ${VST3_SDK_PATH}/public.sdk/source/vst/vstaudioeffect.cpp
    ${VST3_SDK_PATH}/public.sdk/source/vst/vsteditcontroller.cpp
    ${VST3_SDK_PATH}/public.sdk/source/vst/vstnoteexpressiontypes.cpp
    ${VST3_SDK_PATH}/public.sdk/source/vst/vstparameters.cpp
    ${VST3_SDK_PATH}/public.sdk/source/vst/vstpresetfile.cpp
    ${VST3_SDK_PATH}/public.sdk/source/vst/vstrepresentation.cpp
)

# VST3 Base sources (from submodule)
set(VST3_BASE_SOURCES
    ${VST3_SDK_PATH}/base/source/fobject.cpp
    ${VST3_SDK_PATH}/base/source/fstring.cpp
    ${VST3_SDK_PATH}/base/source/fdebug.cpp
)

# Plugin sources
set(PLUGIN_SOURCES
    src/NAMProcessor.cpp
    src/NAMController.cpp
    src/NAMParameters.cpp
    src/NAMFileParser.cpp
    src/NAMWaveNet.cpp
    src/plugentry.cpp
)

# All sources
set(ALL_SOURCES ${VST3_SOURCES} ${VST3_BASE_SOURCES} ${PLUGIN_SOURCES})

# Include directories - must include SDK root for pluginterfaces and base
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${VST3_SDK_PATH}                    # For pluginterfaces/, base/
    ${VST3_SDK_PATH}/public.sdk/source  # For public.sdk/source/vst/...
    /usr/include/eigen3
)

# Create plugin as shared library
add_library(${PLUGIN_NAME} MODULE ${ALL_SOURCES})

# Set output properties for VST3 on Linux
set_target_properties(${PLUGIN_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    OUTPUT_NAME "NAMLoader"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Compile definitions
target_compile_definitions(${PLUGIN_NAME} PRIVATE
    VST3_API
    __linux__
    LINUX
    _LARGEFILE64_SOURCE
    NDEBUG
)

# Link libraries
target_link_libraries(${PLUGIN_NAME} PRIVATE
    dl
    pthread
    ${X11_LIBRARIES}
)

# Create VST3 bundle structure after build
add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PLUGIN_NAME}>/NAMLoader.vst3/Contents/x86_64-linux
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:${PLUGIN_NAME}>
        $<TARGET_FILE_DIR:${PLUGIN_NAME}>/NAMLoader.vst3/Contents/x86_64-linux/NAMLoader.so
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist
        $<TARGET_FILE_DIR:${PLUGIN_NAME}>/NAMLoader.vst3/Contents/Info.plist
    COMMENT "Creating VST3 bundle"
)