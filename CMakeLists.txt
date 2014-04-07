#CMakeLists.txt
cmake_minimum_required(VERSION 2.8)

project(vision)
find_package(PkgConfig)
find_package(OpenCV REQUIRED)
find_package(Qt4 REQUIRED)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pthread -g")  
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -pthread -g")  
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pthread -O2 -march=native")  
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pthread -O2 -march=native")  

set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -pthread -g -pg")  
set(CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_PROFILE} -pthread -g -pg")  


SET(vision_SOURCES OptionsProcess.cxx ImageUtils.cxx  RectTargetMeasured.cxx RectTarget.cxx Messaging.cxx TimeUtils.cxx Main.cxx ProcessRectTarget.cxx ProcessNoTarget.cxx PoleTarget.cxx PoleTargetMeasured.cxx ProcessPoleTarget.cxx Semaphore.cxx ImageBufferManager.cxx ImageCapture.cxx Mutex.cxx Process.cxx ProcessTargetBase.cxx MainWindow.cxx)
SET(vision_HEADERS MainWindow.h HsvThresholdQt.h)
SET(vision_FORMS MainWindow.ui)
SET(vision_RESOURCES )

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})
QT4_WRAP_CPP(vision_HEADERS_MOC ${vision_HEADERS})
QT4_WRAP_UI(vision_FORMS_HEADERS ${vision_FORMS})
QT4_ADD_RESOURCES(vision_RESOURCES_RCC ${vision_RESOURCES})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

add_executable(vision ${vision_SOURCES} ${vision_HEADERS_MOC} ${vision_FORMS_HEADERS} ${vision_RESOURCES_RCC})
target_link_libraries(vision ${OpenCV_LIBS} ${QT_LIBRARIES})
