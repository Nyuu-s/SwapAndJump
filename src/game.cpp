
#include "assets.h"
#include "gl_render_interface.h"
#include "SAJ_lib.h"


void update_game()
{
    for (size_t i = 0; i < 10; i++)
    {
        for (size_t j = 0; j < 10; j++)
        {
            draw_sprite(SPRITE_DICE, {i*100.0f, j*100.0f}, {100.0f, 100.0f});
        }
        
    }
    
}