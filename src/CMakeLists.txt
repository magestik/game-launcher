cmake_minimum_required(VERSION 3.1)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")

    add_executable(GameLauncher WIN32 main.c settings.c settings.h splashscreen_win32.c splashscreen.h splashscreen_common.c splashscreen_common.h)

else ()

    option(ENABLE_XCB "Enable XCB" ON)
    option(ENABLE_XLIB "Enable Xlib" OFF)

    if (ENABLE_XCB AND ENABLE_XLIB)
	message(FATAL_ERROR "Can't enable both xcb and Xlib")
    endif()

    if (ENABLE_XCB)
	add_executable(GameLauncher main.c settings.c settings.h splashscreen_xcb.c splashscreen.h splashscreen_common.c splashscreen_common.h)
	target_link_libraries(GameLauncher PRIVATE xcb)
    elseif(ENABLE_XLIB)
	add_executable(GameLauncher main.c settings.c settings.h splashscreen_xlib.c splashscreen.h splashscreen_common.c splashscreen_common.h)
	target_link_libraries(GameLauncher PRIVATE X11)
    else()
	message(FATAL_ERROR "Must enable xcb or Xlib")
    endif()

    target_link_libraries(GameLauncher PRIVATE dl)

    target_compile_options(GameLauncher PRIVATE -Wall -Wextra -Werror)

endif()

target_link_libraries(GameLauncher PRIVATE jansson)

target_link_libraries(GameLauncher PRIVATE rlottie)
target_include_directories(GameLauncher PRIVATE ../rlottie/inc)

target_link_libraries(GameLauncher PRIVATE cairo)
