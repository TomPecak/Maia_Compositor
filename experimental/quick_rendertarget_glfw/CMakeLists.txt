cmake_minimum_required(VERSION 3.16)

project(quick_rendertarget_glfw VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(appquick_rendertarget_glfw
    main.cpp
)

qt_add_qml_module(appquick_rendertarget_glfw
    URI quick_rendertarget_glfw
    QML_FILES
        Main.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appquick_rendertarget_glfw PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appquick_rendertarget_glfw
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appquick_rendertarget_glfw
    PRIVATE Qt6::Quick
    glfw
)

install(TARGETS appquick_rendertarget_glfw
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
