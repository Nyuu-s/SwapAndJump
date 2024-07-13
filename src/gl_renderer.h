#pragma once

#include "glcorearb.h"
#include "SAJ_lib.h"

//create function pointer, definied in dynamic lib from opengl32.dll
static PFNGLCREATEPROGRAMPROC glCreateProgram_ptr;

void gl_load_functions()
{
    

    glCreateProgram_ptr = (PFNGLCREATEPROGRAMPROC) platform_load_gl_function("glCreateProgram");

}

GLAPI GLuint APIENTRY glCreateProgram(void)
{
    return glCreateProgram_ptr();
}