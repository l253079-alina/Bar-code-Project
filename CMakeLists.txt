cmake_minimum_required(VERSION 3.21)
project(BarcodeSix LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)   # needed for CameraScanner (signals)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)

add_executable(barcode_six
    main.cpp
    main_window.cpp
    main_window.h
    camera_scanner.cpp
    camera_scanner.h
    barcode_utils.cpp
    barcode_utils.h
    code128_patterns.h
    code128_encoder.h
    code128_decoder.h
)
target_link_libraries(barcode_six PRIVATE Qt6::Widgets Qt6::Multimedia)
