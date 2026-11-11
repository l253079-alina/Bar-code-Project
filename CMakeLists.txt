cmake_minimum_required(VERSION 3.21)
project(BarcodeSix LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)  

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(barcode_six
    main.cpp
    mainwindow.cpp
    mainwindow.h
    barcodeUtils.cpp
    barcodeUtils.h
    code128patterns.h
    code128encoder.h
    code128decoder.h
)
target_link_libraries(barcode_six PRIVATE Qt6::Widgets)
