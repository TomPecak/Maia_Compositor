cmake_minimum_required(VERSION 3.16)

project(Maia-Compositor-App LANGUAGES C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(WLROOTS REQUIRED IMPORTED_TARGET wlroots-0.19)
pkg_check_modules(WAYLAND_SERVER REQUIRED IMPORTED_TARGET wayland-server)
pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon)

message(STATUS "=== DETECTED WLROOTS VERSION: ${WLROOTS_VERSION} ===")

add_executable(Maia-Compositor-App main.c)

target_compile_definitions(Maia-Compositor-App PRIVATE WLR_USE_UNSTABLE)

target_link_libraries(Maia-Compositor-App PRIVATE
    PkgConfig::WLROOTS
    PkgConfig::WAYLAND_SERVER
    PkgConfig::XKBCOMMON
)

include(GNUInstallDirs)
install(TARGETS Maia-Compositor-App
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
