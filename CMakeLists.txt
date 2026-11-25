cmake_minimum_required(VERSION 3.21)
project(BarcodeSix LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)

add_executable(barcode_six
    main.cpp
    mainwindow.cpp
    mainwindow.h
    standardnames.h
    orderconstants.h
    orderdetailrecord.h
    existingorderinfo.h
    orderreader.h
    orderrecordparser.cpp
    orderrecordparser.h
    heldorderservice.cpp
    heldorderservice.h
    heldordersdialog.cpp
    heldordersdialog.h
    product.h
    productlineparser.cpp
    productlineparser.h
    productrepository.h
    fileproductrepository.cpp
    fileproductrepository.h
    productcatalog.cpp
    productcatalog.h
    orderitem.h
    order.h
    shoppingcart.cpp
    shoppingcart.h
    filefieldformatter.cpp
    filefieldformatter.h
    textfilelines.cpp
    textfilelines.h
    orderrecordformatter.cpp
    orderrecordformatter.h
    orderrepository.h
    fileorderrepository.cpp
    fileorderrepository.h
    bill.h
    billcalculator.cpp
    billcalculator.h
    billformatter.cpp
    billformatter.h
    billrepository.h
    filebillrepository.cpp
    filebillrepository.h
    checkoutservice.cpp
    checkoutservice.h
    orderscreen.cpp
    orderscreen.h
    checkoutdialog.cpp
    checkoutdialog.h
    billdialog.cpp
    billdialog.h
    scanpanel.cpp
    scanpanel.h
    productdescriptionformatter.cpp
    productdescriptionformatter.h
    productinfoscreen.cpp
    productinfoscreen.h
    barcodegeneratorscreen.cpp
    barcodegeneratorscreen.h
    datafolderlocator.cpp
    datafolderlocator.h
    camerascanner.cpp
    camerascanner.h
    barcodeUtils.cpp
    barcodeUtils.h
    code128patterns.h
    code128encoder.h
    code128decoder.h
    category.h
    categoryrepository.h
    filecategoryrepository.cpp
    filecategoryrepository.h
    categorycatalog.cpp
    categorycatalog.h
    season.h
    seasonrepository.h
    fileseasonrepository.cpp
    fileseasonrepository.h
    seasoncatalog.cpp
    seasoncatalog.h
    productrecordformatter.cpp
    productrecordformatter.h
    billrecordparser.cpp
    billrecordparser.h
    billpdfwriter.cpp
    billpdfwriter.h
    managerapprovalservice.cpp
    managerapprovalservice.h
    reportservice.cpp
    reportservice.h
    categoryscreen.cpp
    categoryscreen.h
    productdialog.cpp
    productdialog.h
    productmanagementscreen.cpp
    productmanagementscreen.h
    seasonscreen.cpp
    seasonscreen.h
    billsscreen.cpp
    billsscreen.h
    reportsscreen.cpp
    reportsscreen.h
)
target_link_libraries(barcode_six PRIVATE Qt6::Widgets Qt6::Multimedia)

