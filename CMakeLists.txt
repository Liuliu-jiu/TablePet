cmake_minimum_required(VERSION 3.16)

project(TablePet2 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp



)
# 设置图标路径（指向ICO文件）
set(APP_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cat_Idle.ico")

# Windows图标设置
if(WIN32)
    # 创建RC文件
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc"
        "IDI_ICON1 ICON DISCARDABLE \"${APP_ICON_PATH}\""
    )
    # 将RC文件添加到源文件列表
    list(APPEND PROJECT_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc")
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TablePet2
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}





        res.qrc
        logsystem.h logsystem.cpp
        mainwidget.h mainwidget.cpp mainwidget.ui
        setwindow.h setwindow.cpp setwindow.ui
        statemanagement.h statemanagement.cpp

        preload.h preload.cpp
        showfontwindow.h showfontwindow.cpp showfontwindow.ui

        performancewindow.h performancewindow.cpp performancewindow.ui


        configfile.h configfile.cpp
        custommodelwindow.h custommodelwindow.cpp custommodelwindow.ui

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET TablePet2 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(TablePet2 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(TablePet2
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(TablePet2 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TablePet2)
endif()
set_target_properties(TablePet2 PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS TablePet2
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TablePet2)
endif()
