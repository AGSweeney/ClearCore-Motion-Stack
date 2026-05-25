cmake_minimum_required(VERSION 3.24)

project(EtherCATMasterQt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(EC_MASTER_FETCH_SOEM "Fetch SOEM from GitHub during configure" ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Svg)

if(EC_MASTER_FETCH_SOEM)
  include(FetchContent)
  FetchContent_Declare(
    soem
    GIT_REPOSITORY https://github.com/OpenEtherCATsociety/SOEM.git
    GIT_TAG master
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(soem)
  set(SOEM_TARGET soem)
else()
  find_package(SOEM REQUIRED)
  if(TARGET SOEM::soem)
    set(SOEM_TARGET SOEM::soem)
  elseif(TARGET soem)
    set(SOEM_TARGET soem)
  else()
    message(FATAL_ERROR "SOEM target not found. Expected SOEM::soem or soem.")
  endif()
endif()

add_executable(EtherCATMasterQt WIN32
  src/main.cpp
  src/MainWindow.h
  src/MainWindow.cpp
  src/EthercatMasterThread.h
  src/EthercatMasterThread.cpp
  src/EthercatTypes.h
  src/resources.qrc
  src/app_icon.rc
)

target_link_libraries(EtherCATMasterQt
  PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Svg
    ${SOEM_TARGET}
)

target_compile_definitions(EtherCATMasterQt
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>
)

install(TARGETS EtherCATMasterQt RUNTIME DESTINATION bin)
