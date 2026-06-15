cmake_minimum_required(VERSION 3.16)
project(NAMVST3 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# SDK paths - use the full VST3 SDK with submodules
# Default to local path, can be overridden via -DVST3_SDK_PATH=... or env var
set(VST3_SDK_PATH_DEFAULT "/home/chopiou/Hermes-Cloud/audio/projects/vst3sdk")
if(DEFINED ENV{VST3_SDK_PATH})
  set(VST3_SDK_PATH $ENV{VST3_SDK_PATH})
else()
  set(VST3_SDK_PATH "${VST3_SDK_PATH_DEFAULT}" CACHE PATH "Path to VST3 SDK root (must contain pluginterfaces, base, public.sdk)")
endif()

# Verify SDK exists
if(NOT EXISTS "${VST3_SDK_PATH}/public.sdk/source/main/pluginfactory.cpp")
  message(FATAL_ERROR "VST3 SDK not found at ${VST3_SDK_PATH}. Set VST3_SDK_PATH or VST3_SDK_PATH env var.")
endif()

# Platform-specific VST3 SDK sources
if(APPLE)
  # macOS: exclude linuxmain.cpp, include macmain.cpp
  set(VST3_PLATFORM_SOURCES
    ${VST3_SDK_PATH}/public.sdk/source/main/macmain.cpp
  )
  set(VST3_COMMON_SOURCES
    ${VST3_SDK_PATH}/public.sdk/source/common/memorystream.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/commonstringconvert.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/commoniids.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/pluginview.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/readfile.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/openurl.cpp
    # macOS platform-specific (ObjC++)
    ${VST3_SDK_PATH}/public.sdk/source/common/threadchecker_mac.mm
    ${VST3_SDK_PATH}/public.sdk/source/common/systemclipboard_mac.mm
  )
elseif(LINUX)
  set(VST3_PLATFORM_SOURCES
    ${VST3_SDK_PATH}/public.sdk/source/main/linuxmain.cpp
  )
  set(VST3_COMMON_SOURCES
    ${VST3_SDK_PATH}/public.sdk/source/common/memorystream.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/commonstringconvert.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/commoniids.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/pluginview.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/readfile.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/openurl.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/threadchecker_linux.cpp
    ${VST3_SDK_PATH}/public.sdk/source/common/systemclipboard_linux.cpp
  )
else()
  message(FATAL_ERROR "Unsupported platform for VST3 SDK sources")
endif()

# Plugin configuration
set(PLUGIN_NAME "NAMLoader")

# Find required packages
find_package(PkgConfig REQUIRED)
if(LINUX)
  find_package(X11 REQUIRED)
endif()

# Eigen3 - use include directories directly
if(LINUX)
  include_directories(/usr/include/eigen3)
elseif(APPLE)
  include_directories(/opt/homebrew/include/eigen3 /usr/local/include/eigen3)
endif()

# ============================================================================
# VST3 SDK Static Library (base + public.sdk sources)
# ============================================================================

# ============================================================================
# VST3 SDK — All required source files for a successful link on macOS
# ============================================================================

# --- pluginterfaces/base/ sources (CRITICAL — missing = FUID/UString linker errors) ---
set(VST3_PLUGINTERFACE_SOURCES
    ${VST3_SDK_PATH}/pluginterfaces/base/funknown.cpp       # FUID::getLong1/2/3/4(), FUnknown impl
    ${VST3_SDK_PATH}/pluginterfaces/base/ustring.cpp        # UString::copyTo, scanInt, scanFloat, assign
    ${VST3_SDK_PATH}/pluginterfaces/base/coreiids.cpp       # DEF_CLASS_IID: FUnknown, IBStream, IPluginBase
    ${VST3_SDK_PATH}/pluginterfaces/base/conststringtable.cpp # ConstStringTable support
)

# --- base/source/ sources ---
set(VST3_BASE_SOURCES
    ${VST3_SDK_PATH}/base/source/fobject.cpp                # FObject base class
    ${VST3_SDK_PATH}/base/source/fstring.cpp                # ConstString, string ops
    ${VST3_SDK_PATH}/base/source/fdebug.cpp                 # Debug/trace utilities
    ${VST3_SDK_PATH}/base/source/baseiids.cpp               # DEF_CLASS_IID: IString, IPersistent, IAttributes
    ${VST3_SDK_PATH}/base/source/fbuffer.cpp                # Buffer utilities
    ${VST3_SDK_PATH}/base/source/fdynlib.cpp                # Dynamic library loading
    ${VST3_SDK_PATH}/base/source/fstreamer.cpp              # Stream I/O utilities
    ${VST3_SDK_PATH}/base/source/timer.cpp                  # Timer support (component lifecycle)
    ${VST3_SDK_PATH}/base/source/updatehandler.cpp          # Update handler (IDependent)
)

# --- public.sdk/source/ core sources ---
set(VST3_PUBLICSDK_SOURCES
    # Main entry points
    ${VST3_SDK_PATH}/public.sdk/source/main/pluginfactory.cpp
    ${VST3_SDK_PATH}/public.sdk/source/main/moduleinit.cpp
    ${VST3_PLATFORM_SOURCES}
    # Common utilities
    ${VST3_COMMON_SOURCES}
    # VST interface IIDs (CRITICAL -- missing = VST interface IID linker errors)
    ${VST3_SDK_PATH}/public.sdk/source/vst/vstinitiids.cpp
    # VST base classes (excluding vstguieditor.cpp for minimal build)
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

# Create VST3 SDK static library
add_library(vst3sdk_static STATIC ${VST3_BASE_SOURCES} ${VST3_PLUGINTERFACE_SOURCES} ${VST3_PUBLICSDK_SOURCES})

# Must be compiled with PIC since it's linked into a shared MODULE (.vst3 / .so)
set_target_properties(vst3sdk_static PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(vst3sdk_static PUBLIC
    ${VST3_SDK_PATH}
    ${VST3_SDK_PATH}/public.sdk/source
)

target_compile_definitions(vst3sdk_static PUBLIC
    VST3_API
)

if(APPLE)
  target_compile_definitions(vst3sdk_static PUBLIC MAC MACOS)
elseif(LINUX)
  target_compile_definitions(vst3sdk_static PUBLIC __linux__ LINUX)
endif()

# ============================================================================
# Plugin sources
# ============================================================================
set(PLUGIN_SOURCES
    src/NAMProcessor.cpp
    src/NAMController.cpp
    src/NAMParameters.cpp
    src/NAMFileParser.cpp
    src/NAMWaveNet.cpp
    src/plugentry.cpp
)

# Include directories for plugin
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${VST3_SDK_PATH}
    ${VST3_SDK_PATH}/public.sdk/source
)

# Create plugin as shared library
add_library(${PLUGIN_NAME} MODULE ${PLUGIN_SOURCES})

# Link against VST3 SDK static library
target_link_libraries(${PLUGIN_NAME} PRIVATE vst3sdk_static)

# Platform-specific target properties and compile definitions
if(APPLE)
  # macOS bundle properties
  set_target_properties(${PLUGIN_NAME} PROPERTIES
      PREFIX ""
      SUFFIX ".vst3"
      OUTPUT_NAME "NAMLoader"
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
      POSITION_INDEPENDENT_CODE ON
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist
  )
  target_compile_definitions(${PLUGIN_NAME} PRIVATE
      VST3_API
      MAC
      MACOS
      _LARGEFILE64_SOURCE
      NDEBUG
  )
  target_link_libraries(${PLUGIN_NAME} PRIVATE
      "-framework Cocoa"
      "-framework AudioToolbox"
      "-framework CoreFoundation"
  )
elseif(LINUX)
  # Linux VST3 properties
  set_target_properties(${PLUGIN_NAME} PROPERTIES
      PREFIX ""
      SUFFIX ".so"
      OUTPUT_NAME "NAMLoader"
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
      POSITION_INDEPENDENT_CODE ON
  )
  target_compile_definitions(${PLUGIN_NAME} PRIVATE
      VST3_API
      __linux__
      LINUX
      _LARGEFILE64_SOURCE
      NDEBUG
  )
  target_link_libraries(${PLUGIN_NAME} PRIVATE
      dl
      pthread
      ${X11_LIBRARIES}
  )
endif()

# Create VST3 bundle structure after build
if(APPLE)
  add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory
          $<TARGET_FILE_DIR:${PLUGIN_NAME}>/NAMLoader.vst3/Contents/MacOS
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_FILE:${PLUGIN_NAME}>
          $<TARGET_FILE_DIR:${PLUGIN_NAME}>/NAMLoader.vst3/Contents/MacOS/NAMLoader
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist
          $<TARGET_FILE_DIR:${PLUGIN_NAME}>/NAMLoader.vst3/Contents/Info.plist
      COMMENT "Creating macOS VST3 bundle"
  )
elseif(LINUX)
  add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory
          $<TARGET_FILE_DIR:${PLUGIN_NAME}>/NAMLoader.vst3/Contents/x86_64-linux
      COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE:${PLUGIN_NAME}>
          $<TARGET_FILE_DIR:${PLUGIN_NAME}>/NAMLoader.vst3/Contents/x86_64-linux/NAMLoader.so
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist
          $<TARGET_FILE_DIR:${PLUGIN_NAME}>/NAMLoader.vst3/Contents/Info.plist
      COMMENT "Creating Linux VST3 bundle"
  )
endif()