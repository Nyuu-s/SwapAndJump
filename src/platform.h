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