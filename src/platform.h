#pragma once

//########################################################################
// PLATFORM GLOBALS
//########################################################################
bool static running = true;


//########################################################################
// PLATFORM FUNCTIONS
//########################################################################
bool platform_create_window(int width, int height, char* title);
void update_platform_window();
void* platform_load_gl_function(char* funcName);
void platform_swap_buffers();
void platform_set_vsync(bool isVsync);

bool platform_free_dynamic_lib(void*);
void* platform_load_dynamic_lib(char*);
void* platform_load_dynamic_function(void*, char*);
void platform_fill_keycode_lookup_table();