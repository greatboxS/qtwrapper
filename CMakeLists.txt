cmake_minimum_required(VERSION 3.16)

project(qtwrapper VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/worker/*.cpp
          ${CMAKE_SOURCE_DIR}/mutexsafe/*.cpp)

option(QT5_BUILD "Build wrapper for qt5" ON)
option(QT6_BUILD "Build wrapper for qt6" ON)

if (QT5_BUILD)
    find_package(Qt5 COMPONENTS Core)
    if(Qt5_FOUND)
        add_library(qt5-wrapper SHARED ${SRC_FILES})
        target_link_libraries(qt5-wrapper PRIVATE Qt5::Core)
        target_include_directories(qt5-warpper PRIVATE ${CMAKE_SOURCE_DIR}
                                                      ${CMAKE_SOURCE_DIR}/worker
                                                      ${CMAKE_SOURCE_DIR}/mutexsafe)
    endif()
endif()

if(QT6_BUILD)
    find_package(Qt6 COMPONENTS Core)
    if(Qt6_FOUND)
        add_library(qt6-wrapper SHARED ${SRC_FILES})
        target_link_libraries(qt6-wrapper PRIVATE Qt6::Core)
        target_include_directories(qt6-wrapper PRIVATE ${CMAKE_SOURCE_DIR}
                                                      ${CMAKE_SOURCE_DIR}/worker
                                                      ${CMAKE_SOURCE_DIR}/mutexsafe)

    endif()
endif()

