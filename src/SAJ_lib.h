#pragma once

#include <stdio.h>

// malloc
#include <stdlib.h>

//memset
#include <string.h>

//Timestamp
#include <sys/stat.h>
#include <sys/types.h>


#include <stdio.h>
#include <limits.h> // For PATH_MAX
#include <direct.h>
#include <math.h>
//########################################################################
// DEFINES
//########################################################################
#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak()
#define EXPORT_FN __declspec(dllexport)
#elif __linux__
#define DEBUG_BREAK() _builtin_debugtrap()
#define EXPORT_FN
#elif __APPLE__
#define DEBUG_BREAK() _builtin_trap()
#endif

#define b8 char
#define SHIFT_IN_BIT(x) 1 << (x)
#define KB(x) ((unsigned long long)1024 * x)
#define MB(x) ((unsigned long long)1024 * KB(x))
#define GB(x) ((unsigned long long)1024 * MB(x))
#define ArrayDef(type, size) struct Array##type \
{\
    int count = 0;\
    int maxCount = 1000;\
    type elements[size];\
    int add(type element)\
    {\
        elements[count] = element;\
        return count++;\
    }\
    bool is_full()\
    {\
        return count == maxCount;\
    }\
    void clear()\
    {\
        count = 0;\
    }\
}

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

long long get_timestamp(const char* file)
{
    struct stat file_stat;

    SAJ_ASSERT(stat(file, &file_stat) != -1, "Failed to get timestamp of file %s" , file);
         
    return (long long)file_stat.st_mtime;
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
    
    auto file = fopen(outputName, "wb");
    if(!file)
    {
        SAJ_ERROR("Failed to open file ! %s", outputName)
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
        return copy_file(fileName, outputName, buffer);
    }

    return true;
}
//########################################################################
// MATHS
//########################################################################

struct Vec2
{
    float x;
    float y;
    Vec2 operator-(Vec2 other)
    {
        return {x - other.x, y - other.y};
    }
    Vec2 operator/(float other)
    {
        return {x / other, y / other};
    }
};

struct IVec2
{
    int x;
    int y;
    IVec2 operator-(IVec2 other)
    {
        return {x - other.x, y - other.y};
    }

    IVec2 operator/(float other)
    {
        return {(int) (x / other),(int) (y / other)};
    }

    IVec2& operator-=(int other)
    {
        x -= other;
        y -= other;
        return *this;
    }

    IVec2& operator+=(int other)
    {
        x += other;
        y += other;
        return *this;
    }

};
Vec2 vec_2(IVec2 v)
{
    return Vec2{(float)v.x, (float)v.y};
}
struct Vec4
{
    union
    {

        float values[4];
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
        struct
        {
            float r;
            float g;
            float b;
            float a;
        };
    };
    float& operator[](int i)
    {
        return values[i]; 
    };
};
struct Vec3
{
    union
    {

        float values[3];
        struct
        {
            float x;
            float y;
            float z;
        };
        struct
        {
            float r;
            float g;
            float b;
        };
    };
    float& operator[](int i)
    {
        return values[i]; 
    };
};

struct Mat4
{
     union
     {
        Vec4 values[4];
        struct
        {
            float ax;
            float bx;
            float cx;
            float dx;
            
            float ay;
            float by;
            float cy;
            float dy;
            
            float az;
            float bz;
            float cz;
            float dz;
            
            float aw;
            float bw;
            float cw;
            float dw;
        };
        
     };

    Vec4& operator[](int i)
    {
        return values[i]; 
    };
     
};

float lerp(float a, float b, float dt)
{
    // dt = 0 : a + (b-a) * 0 = a
    // dt = 1 : a + (b-a) * 1 = 
    //          a + b - a = b

    return a + (b-a) * dt;
}

Vec2 lerp(Vec2 prev, Vec2 curr, float dt)
{
    Vec2 res;
    res.x = lerp(prev.x, curr.x, dt);
    res.y = lerp(prev.y, curr.y, dt);
    return res;
}


IVec2 lerp(IVec2 prev, IVec2 curr, float dt)
{
    IVec2 res;
    res.x = (int)floorf(lerp((float)prev.x, (float) curr.x, dt));
    res.y = (int)floorf(lerp((float)prev.y, (float) curr.y, dt));
    return res;
}

long long max_LL(long long a, long long b)
{
    return (a > b) ? a : b;
}

float max_f(float a, float b)
{
    return (a > b) ? a : b;
}

float min_f(float a, float b)
{
    return (a < b) ? a : b;
}

float approach(float current, float target, float increase)
{
    if(current < target)
    {
        return min_f(current + increase, target);
    }
    return max_f(current - increase, target);
}
int sign(int x)
{
    return (x >= 0) ? 1 : -1;
}

float sign(float x)
{
    return (x >= 0.0f) ? 1.0f : -1.0f;
}

Mat4 orthographic_projection(float left, float right , float top , float bottom)
{
    // Mat4 result = {};
    // // Scale factors for X and Y axes
    // result[0][0] = 2.0f / (right - left); // Scale X
    // result[1][1] = 2.0f / (top - bottom); // Scale Y
    // // Translation factors to center the view
    // result.aw = -(right + left) / (right - left); // Center X
    // result.bw = (top + bottom) / (top - bottom); // Center Y

    // // Depth settings
    // result[2][2] = -1.0f / (1.0f - 0.0f); // Depth scale factor, note the negation for right-handed coordinate system
    // result[3][3] = 1.0f; // Homogeneous coordinate
    
    // return result;
    Mat4 result = {};
    result.aw = -(right + left) / (right - left);
    result.bw = (top + bottom) / (top - bottom);
    result.cw = 0.0f; // Near Plane
    result[0][0] = 2.0f / (right - left);
    result[1][1] = 2.0f / (top - bottom); 
    result[2][2] = 1.0f / (1.0f - 0.0f); // Far and Near
    result[3][3] = 1.0f;

    return result;

}

struct IRect {
    IVec2 pos;
    IVec2 size;
};
struct Rect {
    Vec2 pos;
    Vec2 size;
};

bool point_in_rect(Vec2 point, Rect rect)
{
    return( point.x >= rect.pos.x && 
            point.x <= rect.pos.x + rect.size.x &&
            point.y >= rect.pos.y && 
            point.y <= rect.pos.y + rect.size.y);
}


bool point_in_rect(IVec2 point, IRect rect)
{
    return( point.x >= rect.pos.x && 
            point.x <= rect.pos.x + rect.size.x &&
            point.y >= rect.pos.y && 
            point.y <= rect.pos.y + rect.size.y);
}

bool rect_collision(IRect a, IRect b)
{
    return(
        a.pos.x < b.pos.x + b.size.x &&  // left A with right B
        a.pos.x + a.size.x > b.pos.x &&  // Right A with left B
        a.pos.y < b.pos.y + b.size.y &&  // Bottom A with up B
        a.pos.y + a.size.y > b.pos.y     // Up A with bottom B 
    );
}

 