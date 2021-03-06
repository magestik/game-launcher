cmake_minimum_required(VERSION 3.1)

include(CheckIncludeFile)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")

	find_package(unofficial-cairo CONFIG REQUIRED)
	find_package(jansson CONFIG REQUIRED)

    add_executable(GameLauncher WIN32 main.c settings.c settings.h splashscreen_win32.c splashscreen.h splashscreen_common.c splashscreen_common.h)
    target_link_libraries(GameLauncher PRIVATE unofficial::cairo::cairo unofficial::cairo::cairo-gobject)
    target_link_libraries(GameLauncher PRIVATE jansson::jansson)

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

    check_include_file("gamemode_client.h" HAVE_GAMEMODE_CLIENT_H)
	
	target_link_libraries(GameLauncher PRIVATE cairo)
	target_link_libraries(GameLauncher PRIVATE jansson)

    target_link_libraries(GameLauncher PRIVATE dl)

    target_compile_options(GameLauncher PRIVATE -Wall -Wextra -Werror)

    if (HAVE_GAMEMODE_CLIENT_H)
	target_compile_definitions(GameLauncher PRIVATE -DHAVE_GAMEMODE_CLIENT_H)
    endif()

endif()

target_link_libraries(GameLauncher PRIVATE rlottie)
