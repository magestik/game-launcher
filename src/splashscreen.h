#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <stdbool.h>

#ifdef WIN32
#include <Windows.h>
bool show_splashscreen_win32(HINSTANCE hInstance, const char * title, const char * background, const char * animation, const char * icon);
#endif

#ifdef __linux__
bool show_splashscreen(const char * title, const char * background, const char * animation, const char * icon);
#endif

bool hide_splashscreen(void);

#endif // SPLASHSCREEN_H
