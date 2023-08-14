cmake_minimum_required(VERSION 3.5)

project(EnhancedPlayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EMSDK "C:/Users/smandelli/Source/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")

if(WIN32)
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}../Qt/6.5.1/msvc2019_64/lib/cmake")
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}../Qt/6.5.1/msvc2019_64/lib/cmake")
    list(APPEND CMAKE_PREFIX_PATH "D:/opencv/build")
    set(QT_CHAINLOAD_TOOLCHAIN_FILE "C:/Users/smandelli/Desktop/emscripten/cmake/Modules/Platform/Emscripten.cmake")
else()
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/../Qt/6.5.1/gcc_64/lib/cmake")
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/../Qt/6.5.1/gcc_64/lib/cmake")
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/artifacts/opencv/build")
endif()

set(FISH_EYE_DC_INCS  "${CMAKE_CURRENT_LIST_DIR}/artifacts/Fishye_SDK_V1003/include")
set(MNDEWARP_INCS  "${CMAKE_CURRENT_LIST_DIR}/artifacts/mndewarp/include")

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets LinguistTools Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Multimedia)

find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)


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
    ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
    ${OpenCV_INCLUDE_DIRS}
    ${FISH_EYE_DC_INCS}
    ${MNDEWARP_INCS}
)

target_link_libraries(EnhancedPlayer PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    PkgConfig::LIBAV
    ${OpenCV_LIBS}
    ${FISH_EYE_DC_LIB}
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
