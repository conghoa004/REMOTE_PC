cmake_minimum_required(VERSION 3.16)

project(LR_02 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tìm các gói liên quan
find_package(Qt6 REQUIRED COMPONENTS Gui Qml Quick QuickControls2 Network QuickEffects Multimedia)

qt_standard_project_setup(REQUIRES 6.10)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appLR_02
    main.cpp
)

# Khai báo đường dẫn tĩnh
target_include_directories(appLR_02
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/backend
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/core
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/network
)

qt_add_qml_module(appLR_02
    URI LR_02

    # Các file giao diện QML
    QML_FILES
        qml/Home.qml
        qml/components/Toast.qml
        qml/Control.qml

    # Các file tĩnh
    RESOURCES
        fonts/as7.otf

    # Các file code backend
    SOURCES
        backend/core/NetworkManager.h
        backend/core/NetworkManager.cpp
        backend/core/ConnectionManager.h
        backend/core/ConnectionManager.cpp
        backend/core/ClipboardManager.h
        backend/core/ClipboardManager.cpp
        backend/core/ScreenRenderer.h
        backend/core/ScreenRenderer.cpp
        backend/core/AudioLoopbackCapture.h
        backend/core/AudioLoopbackCapture.cpp
        backend/network/RemoteClient.h
        backend/network/RemoteClient.cpp
        backend/network/RemoteServer.h
        backend/network/RemoteServer.cpp
        backend/network/Protocol.h
        backend/network/Packet.h
        backend/network/Packet.cpp
        backend/network/PacketStream.h
        backend/network/PacketStream.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appLR_02 PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appLR_02
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Khai báo các thư viện
target_link_libraries(appLR_02
    PRIVATE Qt6::Quick
    Qt6::Gui
    Qt6::QuickControls2
    Qt6::Network
    Qt6::QuickEffects
    Qt6::Multimedia
)

if (UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(appLR_02 PRIVATE ${X11_LIBRARIES} ${X11_Xtst_LIB})
    target_include_directories(appLR_02 PRIVATE ${X11_INCLUDE_DIR})
endif()

install(TARGETS appLR_02
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
