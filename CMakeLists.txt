cmake_minimum_required(VERSION 3.16)
project(NAMVST3 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VST3_SDK_PATH_DEFAULT "/home/chopiou/Hermes-Cloud/audio/projects/vst3sdk")
if(DEFINED ENV{VST3_SDK_PATH})
  set(VST3_SDK_PATH $ENV{VST3_SDK_PATH})
else()
  set(VST3_SDK_PATH "${VST3_SDK_PATH_DEFAULT}" CACHE PATH "Path to VST3 SDK root")
endif()

if(NOT EXISTS "${VST3_SDK_PATH}/public.sdk/source/main/pluginfactory.cpp")
  message(FATAL_ERROR "VST3 SDK not found at ${VST3_SDK_PATH}")
endif()

if(APPLE)
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
  message(FATAL_ERROR "Unsupported platform")
endif()

set(PLUGIN_NAME "NAMLoader")
find_package(PkgConfig REQUIRED)
if(LINUX)
  find_package(X11 REQUIRED)
endif()
if(LINUX)
  include_directories(/usr/include/eigen3)
elseif(APPLE)
  include_directories(/opt/homebrew/include/eigen3 /usr/local/include/eigen3)
endif()

# --- VST3 SDK Static Library ---
set(VST3_PLUGINTERFACE_SOURCES
    ${VST3_SDK_PATH}/pluginterfaces/base/funknown.cpp
    ${VST3_SDK_PATH}/pluginterfaces/base/ustring.cpp
    ${VST3_SDK_PATH}/pluginterfaces/base/coreiids.cpp
    ${VST3_SDK_PATH}/pluginterfaces/base/conststringtable.cpp
)
set(VST3_BASE_SOURCES
    ${VST3_SDK_PATH}/base/source/fobject.cpp
    ${VST3_SDK_PATH}/base/source/fstring.cpp
    ${VST3_SDK_PATH}/base/source/fdebug.cpp
    ${VST3_SDK_PATH}/base/source/baseiids.cpp
    ${VST3_SDK_PATH}/base/source/fbuffer.cpp
    ${VST3_SDK_PATH}/base/source/fdynlib.cpp
    ${VST3_SDK_PATH}/base/source/fstreamer.cpp
    ${VST3_SDK_PATH}/base/source/timer.cpp
    ${VST3_SDK_PATH}/base/source/updatehandler.cpp
)
set(VST3_THREAD_SOURCES
    ${VST3_SDK_PATH}/base/thread/source/flock.cpp
    ${VST3_SDK_PATH}/base/thread/source/fcondition.cpp
)
set(VST3_PUBLICSDK_SOURCES
    ${VST3_SDK_PATH}/public.sdk/source/main/pluginfactory.cpp
    ${VST3_SDK_PATH}/public.sdk/source/main/moduleinit.cpp
    ${VST3_PLATFORM_SOURCES}
    ${VST3_COMMON_SOURCES}
    ${VST3_SDK_PATH}/public.sdk/source/vst/vstinitiids.cpp
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

add_library(vst3sdk_static STATIC ${VST3_BASE_SOURCES} ${VST3_THREAD_SOURCES} ${VST3_PLUGINTERFACE_SOURCES} ${VST3_PUBLICSDK_SOURCES})
set_target_properties(vst3sdk_static PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(vst3sdk_static PUBLIC
    ${VST3_SDK_PATH}
    ${VST3_SDK_PATH}/public.sdk/source
    ${VST3_SDK_PATH}/base/thread/include
)
target_compile_definitions(vst3sdk_static PUBLIC VST3_API)
if(APPLE) target_compile_definitions(vst3sdk_static PUBLIC MAC MACOS)
elseif(LINUX) target_compile_definitions(vst3sdk_static PUBLIC __linux__ LINUX) endif()

# ================================================================
# PLUGIN TARGET — CORRECTED FOR macOS VST3
# ================================================================
set(PLUGIN_SOURCES
    src/NAMProcessor.cpp
    src/NAMController.cpp
    src/NAMParameters.cpp
    src/NAMFileParser.cpp
    src/NAMWaveNet.cpp
    src/plugentry.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${VST3_SDK_PATH}
    ${VST3_SDK_PATH}/public.sdk/source
)

add_library(${PLUGIN_NAME} MODULE ${PLUGIN_SOURCES})
target_link_libraries(${PLUGIN_NAME} PRIVATE vst3sdk_static)

if(APPLE)
  # KEY FIX: Build as MACOSX_BUNDLE with proper VST3 structure
  set_target_properties(${PLUGIN_NAME} PROPERTIES
      PREFIX ""
      SUFFIX ".vst3"
      OUTPUT_NAME "NAMLoader"
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
      POSITION_INDEPENDENT_CODE ON
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist
      MACOSX_BUNDLE_BUNDLE_NAME "NAMLoader"
      MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
      MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
      MACOSX_BUNDLE_GUI_IDENTIFIER com.chopiou.namloader
      MACOSX_BUNDLE_LONG_VERSION_STRING "NAM Loader ${PROJECT_VERSION}"
      MACOSX_BUNDLE_COPYRIGHT "Copyright 2025 Chopiou"
  )
  target_compile_definitions(${PLUGIN_NAME} PRIVATE
      VST3_API MAC MACOS _LARGEFILE64_SOURCE NDEBUG
  )
  target_link_libraries(${PLUGIN_NAME} PRIVATE
      "-framework Cocoa"
      "-framework AudioToolbox"
      "-framework CoreFoundation"
  )
  # POST_BUILD: ensure Resources directory exists
  add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory
          $<TARGET_FILE_DIR:${PLUGIN_NAME}>/NAMLoader.vst3/Contents/Resources
      COMMENT "Ensure VST3 Resources directory exists"
  )

elseif(LINUX)
  set_target_properties(${PLUGIN_NAME} PROPERTIES
      PREFIX "" SUFFIX ".so" OUTPUT_NAME "NAMLoader"
      CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON POSITION_INDEPENDENT_CODE ON
  )
  target_compile_definitions(${PLUGIN_NAME} PRIVATE VST3_API __linux__ LINUX _LARGEFILE64_SOURCE NDEBUG)
  target_link_libraries(${PLUGIN_NAME} PRIVATE dl pthread ${X11_LIBRARIES})

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