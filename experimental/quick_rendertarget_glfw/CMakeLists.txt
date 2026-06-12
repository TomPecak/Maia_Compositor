cmake_minimum_required(VERSION 3.16)

project(quick_rendertarget_glfw VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Gui OpenGL)

qt_standard_project_setup(REQUIRES 6.9)

qt_add_executable(appquick_rendertarget_glfw main.cpp)

qt_add_qml_module(appquick_rendertarget_glfw
    URI quick_rendertarget_glfw
    QML_FILES Main.qml
)

set_target_properties(appquick_rendertarget_glfw PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appquick_rendertarget_glfw
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Musisz mieć zainstalowane GLFW w systemie lub dodane przez FetchContent/add_subdirectory
target_link_libraries(appquick_rendertarget_glfw PRIVATE
    Qt6::Quick
    Qt6::Gui
    Qt6::OpenGL
    glfw
)

install(TARGETS appquick_rendertarget_glfw BUNDLE DESTINATION . LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
