cmake_minimum_required(VERSION 3.16)

project(qtwrapper VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/worker/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/mutexsafe/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/logger/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/imageprovider/*.cpp
)

file(GLOB PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/worker/*.h
     ${CMAKE_CURRENT_SOURCE_DIR}/mutexsafe/*.h ${CMAKE_CURRENT_SOURCE_DIR}/logger/*.h
     ${CMAKE_CURRENT_SOURCE_DIR}/imageprovider/*.h
)

option(QT5_BUILD "Build wrapper for qt5" ON)
option(QT6_BUILD "Build wrapper for qt6" ON)
set(QTWRAPPER_LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
set(QTWRAPPER_INC_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/qtwrapper)

if(QT5_BUILD)
    find_package(Qt5 COMPONENTS Core Quick)
    if(Qt5_FOUND)
        set(QT5_WRAPPER
            qt5-wrapper
            CACHE INTERNAL ""
        )
        add_library(${QT5_WRAPPER} SHARED ${SRC_FILES})
        target_link_libraries(${QT%_WRAPPER} PUBLIC Qt5::Core Qt5::Quick)
        target_include_directories(
            qt5-warpper
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/worker
                   ${CMAKE_CURRENT_SOURCE_DIR}/mutexsafe ${CMAKE_CURRENT_SOURCE_DIR}/logger
        )
    endif()
endif()

if(QT6_BUILD)
    find_package(Qt6 COMPONENTS Core Quick)
    if(Qt6_FOUND)
        set(QT6_WRAPPER
            qt6-wrapper
            CACHE INTERNAL ""
        )
        add_library(${QT6_WRAPPER} SHARED ${SRC_FILES})
        target_link_libraries(${QT6_WRAPPER} PUBLIC Qt6::Core Qt6::Quick)
        target_include_directories(
            ${QT6_WRAPPER}
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/worker
                   ${CMAKE_CURRENT_SOURCE_DIR}/mutexsafe ${CMAKE_CURRENT_SOURCE_DIR}/logger
        )

    endif()
endif()

set(VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

if(QT5_WRAPPER)
    set_target_properties(
        ${QT5_WRAPPER} PROPERTIES VERSION ${VERSION_STRING} SOVERSION ${PROJECT_VERSION_MAJOR}
    )
    install(
        TARGETS ${QT5_WRAPPER}
        RUNTIME DESTINATION ${QTWRAPPER_LIB_INSTALL_DIR}
        BUNDLE DESTINATION ${QTWRAPPER_LIB_INSTALL_DIR}
        LIBRARY DESTINATION ${QTWRAPPER_LIB_INSTALL_DIR}
    )
endif()

if(QT6_WRAPPER)
    set_target_properties(
        ${QT6_WRAPPER} PROPERTIES VERSION ${VERSION_STRING} SOVERSION ${PROJECT_VERSION_MAJOR}
    )
    install(
        TARGETS ${QT6_WRAPPER}
        RUNTIME DESTINATION ${QTWRAPPER_LIB_INSTALL_DIR}
        BUNDLE DESTINATION ${QTWRAPPER_LIB_INSTALL_DIR}
        LIBRARY DESTINATION ${QTWRAPPER_LIB_INSTALL_DIR}
    )
endif()
install(FILES ${PUBLIC_HEADER} DESTINATION ${QTWRAPPER_INC_INSTALL_DIR})
