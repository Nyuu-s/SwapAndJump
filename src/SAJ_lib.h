#pragma once

#include <stdio.h>
//########################################################################
// DEFINES
//########################################################################
#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak()
#elif __linux__
#define DEBUG_BREAK() _builtin_debugtrap()
#elif __APPLE__
#define DEBUG_BREAK() _builtin_trap()
#endif
//########################################################################
// LOGGING
//########################################################################
enum TextColor {
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE,
    COLOR_B_BLACK,
    COLOR_B_RED,
    COLOR_B_GREEN,
    COLOR_B_YELLOW,
    COLOR_B_BLUE,
    COLOR_B_MAGENTA,
    COLOR_B_CYAN,
    COLOR_B_WHITE,


    COLOR_COUNT

};
template <typename ...Args>
void _log(char* prefix, char* msg, TextColor textColor, Args... args)
{
    static char* ColorTable[COLOR_COUNT] =
    {
        "\x1b[30m",
        "\x1b[31m",
        "\x1b[32m",
        "\x1b[33m",
        "\x1b[34m",
        "\x1b[35m",
        "\x1b[36m",
        "\x1b[37m",
        "\x1b[90m",
        "\x1b[91m",
        "\x1b[92m",
        "\x1b[93m",
        "\x1b[94m",
        "\x1b[95m",
        "\x1b[96m",
        "\x1b[97m",
    };

    char formatBuffer[8192]={};
    sprintf(formatBuffer, "%s %s %s \033[0m", ColorTable[textColor], prefix, msg);

    char textBuffer[8192]={};
    sprintf(textBuffer, formatBuffer, args...);

    puts(textBuffer);
}

#define SAJ_TRACE(msg, ...) _log("TRACE: ", msg, COLOR_GREEN, ##__VA_ARGS__);
#define SAJ_WARN(msg, ...) _log("WARN: ", msg, COLOR_YELLOW,##__VA_ARGS__);
#define SAJ_ERROR(msg, ...) _log("ERROR: ", msg, COLOR_RED, ##__VA_ARGS__);
#define SAJ_DEBUG(msg, ...) _log("DEBUG: ", msg, COLOR_B_CYAN, ##__VA_ARGS__);

#define SAJ_ASSERT(x, msg, ...)             \
{                                           \
    if(!(x))                                \
    {                                       \
        SAJ_ERROR(msg, ##__VA_ARGS__);      \
        DEBUG_BREAK();                      \
        SAJ_ERROR("ASSERTION HIT")          \
    }                                       \
}                                           