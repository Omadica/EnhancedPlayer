cmake_minimum_required(VERSION 3.5)

project(EnhancedPlayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#cmake_policy(SET CMP0167 NEW)

set(ENV{PKG_CONFIG_PATH} "D:/Source/Repos/EXERCISE/MediaServices/cmake/pkgconfig")

list(APPEND CMAKE_PREFIX_PATH "/home/algebrato/Qt/6.6.0/gcc_64/lib/cmake")
if(WIN32)
    set(PKG_CONFIG_EXECUTABLE "C:/Strawberry/perl/bin/pkg-config.bat")
elseif(UNIX)
    set(PKG_CONFIG_EXECUTABLE "/usr/bin/pkg-config")
endif(WIN32)

list(APPEND CMAKE_PREFIX_PATH "D:/boost-1.84.0/stage/lib/cmake/Boost-1.84.0")

add_subdirectory(artifacts/sigslot)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets LinguistTools Multimedia Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Multimedia Charts)

find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Boost REQUIRED)
#find_package(CURL REQUIRED)

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

file (GLOB PROJECT_SOURCES PROJECT_HEADERS PROJECT_FORMS PROJECT_TR
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ts
)


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(EnhancedPlayer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
        ${PROJECT_FORMS}
        ${PROJECT_TR}

        myqttreewidget.h myqttreewidget.cpp
        chartviewjitter.h chartviewjitter.cpp

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


target_include_directories(EnhancedPlayer PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/artifacts/MediaServices/build/Debug"
    "${CMAKE_CURRENT_SOURCE_DIR}/artifacts/MediaServices/build/Debug/include"
    "${OpenCV_INCLUDE_DIRS}"
    "${LIBAV_INCLUDE_DIRS}"
    "${Boost_INCLUDE_DIRS}"
    "${CMAKE_CURRENT_SOURCE_DIR}/artifacts/sigslot/include/sigslot"
    "${CMAKE_CURRENT_SOURCE_DIR}/artifacts/MediaServices/dependencies/curlpp/include"
    "D:/Source/Repos/EXERCISE/MediaServices/dependencies/curl/include"
)

target_link_directories(EnhancedPlayer PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/artifacts/MediaServices/build/Debug/lib"
    "D:/Source/Repos/EXERCISE/MediaServices/dependencies/curlpp/build/Debug"
    "D:/Source/Repos/EXERCISE/MediaServices/dependencies/curl/build/lib/Debug"
)

target_link_libraries(EnhancedPlayer PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Charts
    task_manager
    task_processor
    media_reader
    media_decoder
    media_wrap
    PkgConfig::LIBAV
    libcurl-d_imp
    curlpp
)

install(TARGETS EnhancedPlayer
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
