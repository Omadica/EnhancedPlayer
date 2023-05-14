cmake_minimum_required(VERSION 3.5)

project(EnhancedPlayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}../Qt/6.5.0/msvc2019_64/lib/cmake")
else()
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/../Qt/6.5.0/gcc_64/lib/cmake")
endif()

if(WIN32)
    set(NV_VIDEO_CODEC_SDK_LIB_CUVID "${CMAKE_CURRENT_LIST_DIR}/artifacts/Video_Codec_SDK_12.0.16/Lib/x64/nvcuvid.lib")
    set(NV_VIDEO_CODEC_SDK_LIB_ENCOD  "${CMAKE_CURRENT_LIST_DIR}/artifacts/Video_Codec_SDK_12.0.16/Lib/x64/nvencodeapi.lib")
else()
    set(NV_VIDEO_CODEC_SDK_LIB_CUVID "${CMAKE_CURRENT_LIST_DIR}/artifacts/Video_Codec_SDK_12.0.16/Lib/linux/stubs/x86_64/libnvcuvid.so")
    set(NV_VIDEO_CODEC_SDK_LIB_ENCOD  "${CMAKE_CURRENT_LIST_DIR}/artifacts/Video_Codec_SDK_12.0.16/Lib/linux/stubs/x86_64/libnvidia-encode.so")
endif()

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets LinguistTools Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Multimedia)

find_package(OpenGL REQUIRED)
find_package(CUDA REQUIRED)
enable_language(CUDA)


find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavcodec
    libavformat
    libavfilter
    libavdevice
    libavutil
    libswresample
    libswscale
)


set(TS_FILES EnhancedPlayer_it_IT.ts )

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        custom_view.cpp
        FFmpegLog.cpp
        FFmpegVideoDecoder.cpp
        NvDecoder.cpp
        glrender.cpp
        mainwindow.h
        custom_view.h
        FFmpegLog.h
        FFmpegVideoDecoder.h
        NvDecoder.h
        glrender.h
        Logger.h
        cuviddec.h
        nvcuvid.h
        nvEncodeAPI.h
        NvCodecUtils.h
        AppDecUtils.h
        mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(EnhancedPlayer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(EnhancedPlayer SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(EnhancedPlayer
            ${PROJECT_SOURCES}
        )
    endif()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()


target_include_directories(EnhancedPlayer BEFORE PRIVATE
    ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)

target_link_libraries(EnhancedPlayer PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    PkgConfig::LIBAV
    OpenGL::GL
    OpenGL::GLU
    ${NV_VIDEO_CODEC_SDK_LIB_CUVID}
    ${NV_VIDEO_CODEC_SDK_LIB_ENCOD}
    cuda
    cudart
)


set_target_properties(EnhancedPlayer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS EnhancedPlayer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

message("INSTALL DIR:" ${CMAKE_INSTALL_LIBDIR})


install(CODE [[
  file(GET_RUNTIME_DEPENDENCIES
    PRE_INCLUDE_REGEXES "^[Mm][Yy][Ll][Ii][Bb][Rr][Aa][Rr][Yy]\\.[Dd][Ll][Ll]$"
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia
    )
  ]])

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(EnhancedPlayer)
endif()
