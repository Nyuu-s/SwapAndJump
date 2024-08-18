//avoid re6importing windows and defining mean max
#define APIENTRY
//to include other function definition like glEnable
#define GL_GLEXT_PROTOTYPES

#include "SAJ_lib.h"

#include "input.h"

#include "game.h"

#include "platform.h"

static KeyCodeBinding KeyCodeLookupTable[KEYCODE_COUNT];

//opengl core
#include "glcorearb.h"

#ifdef _WIN32
#include "win32_platform.cpp"
#endif
#include "gl_render_interface.h"
#include "gl_renderer.cpp"


//TODO training replace by own header
#include <chrono>
double get_delta_time();

//########################################################################
// Game DLL
//########################################################################*
typedef decltype(update_game) update_game_type;
static update_game_type* update_game_ptr;
//########################################################################
// Cross Platform functions
//########################################################################
void reload_game_dll(BumpAllocator* transientStorage);


int main()
{
    //init delta time static var
    get_delta_time();

    BumpAllocator transientStorage = make_bump_allocator(MB(50));
    BumpAllocator persistentStorage = make_bump_allocator(MB(15));

    input=(Input*)bump_alloc(&persistentStorage, sizeof(Input));
    renderData = (RenderData*) bump_alloc(&persistentStorage, sizeof(RenderData));
    gameState = (GameState*) bump_alloc(&persistentStorage, sizeof(GameState));

    if(!input)
    {
        SAJ_ERROR("Failed to allocate input");
        return -1;
    }
    if(!renderData)
    {
        SAJ_ERROR("Failed to allocate RenderData");
        return -1;
    }
    //somehow not initialize correctly in macro
    renderData->transforms.maxCount = 1000;
    if(!gameState)
    {
        SAJ_ERROR("Failed to allocate gameState");
        return -1;
    }
    //somehow not initialize correctly in macro
    gameState->keyMappings->keys.maxCount = 3;
    platform_fill_keycode_lookup_table();
    platform_create_window(1280, 720, "Swap & Jump");
    platform_set_vsync(true);

    gl_init(&transientStorage);
    while (running)
    {
        double dt = get_delta_time();

        reload_game_dll(&transientStorage);

        update_platform_window();
        update_game(gameState, renderData, input, dt);
        gl_render(&transientStorage);
        platform_swap_buffers();

        transientStorage.used = 0;
    }
    return 0;
}

void update_game(GameState* gameStateIn, RenderData* data, Input* inputIn, double deltatime)
{
    update_game_ptr(gameStateIn, data, inputIn, deltatime);
}

double get_delta_time()
{
    //Static (here): Only executed once, life duration is program duration
    static auto lastTime = std::chrono::steady_clock::now();

    auto currentTime = std::chrono::steady_clock::now();

    //in seconds
    double delta = std::chrono::duration<double>(currentTime - lastTime).count();
    lastTime = currentTime;

    return delta;
}
void reload_game_dll(BumpAllocator* transientStorage)
{
    static void* gameDLL;
    static long long last_edit_timestamp_gamedll;

    long long current_timestamp_gamedll = get_timestamp("game.dll");
    if(current_timestamp_gamedll > last_edit_timestamp_gamedll)
    {
        if(gameDLL)
        {
            bool freeResult = platform_free_dynamic_lib(gameDLL);
            SAJ_ASSERT(freeResult, "Failed to free game.dll");
            gameDLL = nullptr;
            SAJ_TRACE("game.dll has been set free")
        }

        while(!copy_file("game.dll", "game_load.dll", transientStorage))
        {
            Sleep(10);
        }
        SAJ_TRACE("Copied game.dll into game_load.dll");

        gameDLL = platform_load_dynamic_lib("game_load.dll");
        SAJ_ASSERT(gameDLL, "Failed to load dynamic lib");

        update_game_ptr = (update_game_type*)platform_load_dynamic_function(gameDLL, "update_game");
        SAJ_ASSERT(update_game_ptr, "Failed to load dynamic function");
        last_edit_timestamp_gamedll = current_timestamp_gamedll;
    }
}

