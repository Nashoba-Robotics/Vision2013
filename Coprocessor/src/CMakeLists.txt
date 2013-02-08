#CMakeLists.txt
cmake_minimum_required(VERSION 2.8)

SET(CMAKE_BUILD_TYPE profile)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2 -march=native")  
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O2 -march=native")  

set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_DEBUG} -O2 -march=native")  
set(CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_DEBUG} -O2 -march=native")  

project(vision)
find_package(PkgConfig)
find_package(OpenCV REQUIRED)
add_executable(vision Arguments.cxx ImageUtils.cxx  RectTargetMeasured.cxx RectTarget.cxx Messaging.cxx TimeUtils.cxx VisionNew2.cxx)
target_link_libraries(vision ${OpenCV_LIBS})
