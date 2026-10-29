cmake_minimum_required(VERSION 3.16)

project(VehicleMeterGuiVer2 LANGUAGES CXX)

include(qt.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)

find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
        Sql
)

qt_standard_project_setup()


# ==========================================
# 車両ロジック
# GUIとGoogleTestの両方から使用する
# ==========================================

add_library(VehicleCore STATIC
    VehicleState.h
    VehicleState.cpp

    VehicleController.h
    VehicleController.cpp

    FuelEconomyCalculator.h
    FuelEconomyCalculator.cpp

    Odometer.h
    Odometer.cpp

    InputTypes.h
    Config.h
)

target_include_directories(VehicleCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)


# ==========================================
# GUIアプリ
# ==========================================

set(PROJECT_SOURCES
    main.cpp

    MainWindow.ui
    MainWindow.h
    MainWindow.cpp

    SpeedometerWidget.h
    SpeedometerWidget.cpp

    DriveSummaryDialog.ui
    DriveSummaryDialog.h
    DriveSummaryDialog.cpp

    DriveRecord.h

    DatabaseManager.h
    DatabaseManager.cpp

    DriveHistoryDialog.ui
    DriveHistoryDialog.h
    DriveHistoryDialog.cpp

    DriveStatistics.h
)

qt_add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        VehicleCore
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Sql
)


# ==========================================
# GoogleTest
# ==========================================

include(FetchContent)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(VehicleMeterCoreTests
    tests/VehicleCoreTests.cpp
 )

target_link_libraries(VehicleMeterCoreTests
    PRIVATE
        VehicleCore
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(VehicleMeterCoreTests)

add_executable(DatabaseManagerTests
    tests/DatabaseManagerTestMain.cpp
    tests/DatabaseManagerTests.cpp
    DatabaseManager.cpp
)

target_include_directories(DatabaseManagerTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(DatabaseManagerTests
    PRIVATE
        Qt::Core
        Qt::Sql
        GTest::gtest
)

add_test(
    NAME DatabaseManagerTests
    COMMAND DatabaseManagerTests
)

set_tests_properties(
    DatabaseManagerTests
    PROPERTIES
        ENVIRONMENT_MODIFICATION
        "PATH=path_list_prepend:$ENV{QTDIR}/bin;QT_PLUGIN_PATH=set:$ENV{QTDIR}/plugins"
)