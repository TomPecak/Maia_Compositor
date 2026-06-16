cmake_minimum_required(VERSION 3.16)

project(Maia-Compositor-App LANGUAGES C CXX)

find_package(PkgConfig REQUIRED)



pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)

pkg_check_modules(WAYLAND_SCANNER REQUIRED wayland-scanner)
pkg_get_variable(WAYLAND_SCANNER_EXEC wayland-scanner wayland_scanner)

pkg_check_modules(TINYWL_DEPS REQUIRED IMPORTED_TARGET
    wlroots-0.19
    wayland-server
    xkbcommon
)

message(STATUS "=== DETECTED WLROOTS VERSION: ${WLROOTS_VERSION} ===")

# Zdefiniowanie ścieżek dla generowanego kodu
set(XDG_SHELL_XML "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml")
set(XDG_SHELL_HEADER "${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-protocol.h")
#set(XDG_SHELL_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/xdg-shell-protocol.h")

add_custom_command(
    OUTPUT ${XDG_SHELL_HEADER}
    COMMAND ${WAYLAND_SCANNER_EXEC} server-header ${XDG_SHELL_XML} ${XDG_SHELL_HEADER}
    DEPENDS ${XDG_SHELL_XML}
    COMMENT "Generowanie xdg-shell-protocol.h za pomoca wayland-scanner"
    VERBATIM
)

add_executable(Maia-Compositor-App main.c
    ${XDG_SHELL_HEADER}
)

target_compile_options(Maia-Compositor-App PRIVATE -Werror)

target_compile_definitions(Maia-Compositor-App PRIVATE WLR_USE_UNSTABLE)

target_link_libraries(Maia-Compositor-App PRIVATE
    PkgConfig::TINYWL_DEPS
)

target_include_directories(Maia-Compositor-App PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

include(GNUInstallDirs)
install(TARGETS Maia-Compositor-App
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
