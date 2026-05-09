cmake_minimum_required(VERSION 3.21)

project(MotionBench VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Quick Qml Network QuickControls2)

qt_standard_project_setup()

qt_add_executable(MotionBenchApp
    WIN32
    src/main.cpp
    src/protocol/CipPath.cpp
    src/protocol/EtherNetIpClient.cpp
    src/discovery/ListIdentityService.cpp
    src/device/ClearLinkObjectMap.cpp
    src/device/DeviceIoWorker.cpp
    src/device/DeviceService.cpp
    src/ui/viewmodels/AppViewModel.cpp
    resources/MotionBenchIcons.qrc
    resources/MotionBenchApp.rc
)

target_include_directories(MotionBenchApp PRIVATE src)

qt_add_qml_module(MotionBenchApp
    URI MotionBench
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

target_link_libraries(MotionBenchApp
    PRIVATE
        Qt6::Core
        Qt6::Quick
        Qt6::Qml
        Qt6::Network
        Qt6::QuickControls2
)
set_target_properties(MotionBenchApp PROPERTIES OUTPUT_NAME "MotionBench")

qt_add_executable(BoardModeToolApp
    WIN32
    src/boardmode/main.cpp
    src/boardmode/BoardModeToolService.cpp
    src/protocol/CipPath.cpp
    src/protocol/EtherNetIpClient.cpp
    src/discovery/ListIdentityService.cpp
    resources/BoardModeToolIcons.qrc
    resources/BoardModeToolApp.rc
)

target_include_directories(BoardModeToolApp PRIVATE src)

qt_add_qml_module(BoardModeToolApp
    URI BoardModeTool
    VERSION 1.0
    QML_FILES
        qml/BoardModeTool.qml
)

target_link_libraries(BoardModeToolApp
    PRIVATE
        Qt6::Core
        Qt6::Quick
        Qt6::Qml
        Qt6::Network
        Qt6::QuickControls2
)
set_target_properties(BoardModeToolApp PROPERTIES OUTPUT_NAME "BoardModeTool")

include(cmake/DeployQt.cmake)
setup_qt_deploy(MotionBenchApp)
setup_qt_deploy(BoardModeToolApp)

include(CTest)
if(BUILD_TESTING)
    add_executable(MotionBenchProtocolSmokeTest
        src/tests/ProtocolSmokeTest.cpp
        src/protocol/CipPath.cpp
        src/device/ClearLinkObjectMap.cpp
    )
    target_include_directories(MotionBenchProtocolSmokeTest PRIVATE src)
    target_link_libraries(MotionBenchProtocolSmokeTest PRIVATE Qt6::Core)
    add_test(NAME MotionBenchProtocolSmokeTest COMMAND MotionBenchProtocolSmokeTest)
endif()
