# CMake generated Testfile for 
# Source directory: D:/ClearCore-Motion-Stack/MotionBench
# Build directory: D:/ClearCore-Motion-Stack/MotionBench/build-vs
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[MotionBenchProtocolSmokeTest]=] "D:/ClearCore-Motion-Stack/MotionBench/build-vs/Debug/MotionBenchProtocolSmokeTest.exe")
  set_tests_properties([=[MotionBenchProtocolSmokeTest]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/ClearCore-Motion-Stack/MotionBench/CMakeLists.txt;93;add_test;D:/ClearCore-Motion-Stack/MotionBench/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[MotionBenchProtocolSmokeTest]=] "D:/ClearCore-Motion-Stack/MotionBench/build-vs/Release/MotionBenchProtocolSmokeTest.exe")
  set_tests_properties([=[MotionBenchProtocolSmokeTest]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/ClearCore-Motion-Stack/MotionBench/CMakeLists.txt;93;add_test;D:/ClearCore-Motion-Stack/MotionBench/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[MotionBenchProtocolSmokeTest]=] "D:/ClearCore-Motion-Stack/MotionBench/build-vs/MinSizeRel/MotionBenchProtocolSmokeTest.exe")
  set_tests_properties([=[MotionBenchProtocolSmokeTest]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/ClearCore-Motion-Stack/MotionBench/CMakeLists.txt;93;add_test;D:/ClearCore-Motion-Stack/MotionBench/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[MotionBenchProtocolSmokeTest]=] "D:/ClearCore-Motion-Stack/MotionBench/build-vs/RelWithDebInfo/MotionBenchProtocolSmokeTest.exe")
  set_tests_properties([=[MotionBenchProtocolSmokeTest]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/ClearCore-Motion-Stack/MotionBench/CMakeLists.txt;93;add_test;D:/ClearCore-Motion-Stack/MotionBench/CMakeLists.txt;0;")
else()
  add_test([=[MotionBenchProtocolSmokeTest]=] NOT_AVAILABLE)
endif()
