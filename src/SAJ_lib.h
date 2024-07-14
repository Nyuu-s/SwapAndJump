#pragma once

#include <stdio.h>

// malloc
#include <stdlib.h>

//memset
#include <string.h>

//Timestamp
#include <sys/stat.h>
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

#define SHIFT_IN_BIT(x) 1 << (x)
#define KB(x) ((unsigned long long)1024 * x)
#define MB(x) ((unsigned long long)1024 * KB(x))
#define GB(x) ((unsigned long long)1024 * MB(x))

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
    }                                       \
}                                           


//########################################################################
// BUMP ALLOCATOR
//########################################################################
struct BumpAllocator
{
    size_t capacity;
    size_t used;
    char * memory;
};

BumpAllocator make_bump_allocator(size_t size)
{
    BumpAllocator bump_allocator = {};
    //void pointer can be casted to any types
    bump_allocator.memory = (char*) malloc(size);
    if (bump_allocator.memory)
    {
        bump_allocator. capacity = size;
        memset(bump_allocator.memory, 0, size);
    }
    else
    {
        SAJ_ASSERT(false, "Memory allocation failed !");
    }
    return bump_allocator;
}

char* bump_alloc(BumpAllocator* BumpAllocator, size_t size)
{
    char* result = nullptr;
    //round to nearest "high" multiple of M-1 (8 here)
    size_t allignedSize = (size + 7) & ~ 7;
    if(BumpAllocator->used + allignedSize <= BumpAllocator->capacity)
    {
        result = BumpAllocator->memory + BumpAllocator->used;
        BumpAllocator->used += allignedSize;
    }
    else
    {
        SAJ_ASSERT(false, "BumpAllocator is Full")
    }

    return result;
}

//########################################################################
// FILE IO
//########################################################################

long long get_timestamp(char* file)
{
    struct stat file_stat = {};
    stat(file, &file_stat);
    return file_stat.st_mtime;
}

bool do_file_exists(char* filePath)
{
    FILE* file = fopen(filePath, "rb");
    if(!file)
    {
        return false;
    }
    fclose(file);
    return true;
}

int get_file_size(char* filePath)
{
    SAJ_ASSERT(filePath, "No FilePath supllied!");
    long fileSize = 0;
    auto file = fopen(filePath, "rb");
    if(!file)
    {
        SAJ_ERROR("Failed to open file ! %s", filePath)
        return false;
    }

    //seek the end
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    //seek back to begin
    fseek(file, 0, SEEK_SET);
    fclose(file);

    return fileSize;
}

char* read_file(char* filePath, int* fileSize, char* buffer)
{
    SAJ_ASSERT(filePath, "No filePath provided")
    SAJ_ASSERT(fileSize, "No fileSize provided")
    
    *fileSize = 0;
    auto file = fopen(filePath, "rb");
    if(!file)
    {
        SAJ_ERROR("Failed to open file ! %s", filePath)
        return nullptr;
    }

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    //seek back to begin
    fseek(file, 0, SEEK_SET);

    //zero out memory
    memset(buffer, 0, *fileSize +1);
    //read file
    fread(buffer, sizeof(char), *fileSize, file);

    fclose(file);

    return buffer;

}

char* read_file(char* filePath, int* fileSize, BumpAllocator* bumpAllocator)
{
    char* file = nullptr;
    long computedSize = get_file_size(filePath);
    if(computedSize)
    {
        char* buffer = bump_alloc(bumpAllocator, computedSize + 1);
        file = read_file(filePath,fileSize, buffer);
    }
    return file;
}

void write_file(char* filePath, char* buffer, int size)
{
    SAJ_ASSERT(filePath, "No filePath provided");
    SAJ_ASSERT(buffer, "No buffer provided");
    SAJ_ASSERT(size, "No size provided");

    auto file = fopen(filePath, "wb");
    if(!file)
    {
        SAJ_ERROR("Failed to open File %s", filePath)
    }
    fwrite(buffer, sizeof(char), size, file);
    fclose(file);
}

bool copy_file(char* fileName, char* outputName, char* buffer)
{
    SAJ_ASSERT(fileName, "No fileName provided")
    SAJ_ASSERT(outputName, "No outputName provided")
    
    int fileSize = 0;
    char* data = read_file(fileName, &fileSize, buffer);
    
    auto file = fopen(outputName, "rb");
    if(!file)
    {
        SAJ_ERROR("Failed to open file ! %s", fileName)
        return false;
    }

    int result = fwrite(data, sizeof(char), fileSize, file);
    if(!result)
    {
        SAJ_ERROR("Failed openoing output file : %s", outputName);
        return false;
    }

    fclose(file);

    return true;


}

bool copy_file(char* fileName, char* outputName, BumpAllocator* bumpAllocator)
{
    char* file = 0;
    long fileSize = get_file_size(fileName);
    if(fileSize)
    {
        char* buffer = bump_alloc(bumpAllocator, fileSize +1);
        copy_file(fileName, outputName, buffer);
    }

    return true;
}