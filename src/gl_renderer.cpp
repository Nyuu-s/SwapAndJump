#include "gl_renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "gl_render_interface.h"

//########################################################################
// OpenGL constants
//########################################################################
const char* TEXTURE_PATH = "assets/textures/TEXTURE_SKY.png";


//########################################################################
// OpenGL Structs
//########################################################################
struct GLContext
{
    GLuint programID;
    GLuint textureID;
    GLuint sTransformSBOID;
    GLuint screenSizeID;
};

//########################################################################
// OpenGL GLOBALS
//########################################################################
static GLContext glContext;

//########################################################################
// OpenGL FUNCTIONS
//########################################################################
static void APIENTRY gl_debug_callback( GLenum source, GLenum type,  GLuint id, GLenum severity, 
                                        GLsizei length, const GLchar* message, const void* user)
{
    if( severity == GL_DEBUG_SEVERITY_HIGH)
    {
        SAJ_ASSERT(false, "OpenGL Error: %s", message)
    }
    else if (severity ==  GL_DEBUG_SEVERITY_MEDIUM || GL_DEBUG_SEVERITY_LOW)
    {
        SAJ_WARN("OpenGL low severity: %s", message)
    }
    else
    {
        SAJ_TRACE((char*) message);
    }
}


bool gl_init(BumpAllocator* transientStorage)
{
    gl_load_functions();

    glDebugMessageCallback(&gl_debug_callback, nullptr);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glEnable(GL_DEBUG_OUTPUT);

    GLuint vertShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    int fileSize = 0;
    char* vertShader = read_file("assets/shaders/quad.vert", &fileSize, transientStorage);
    char* fragShader = read_file("assets/shaders/quad.frag", &fileSize, transientStorage);
    
    if(!vertShader || !fragShader)
    {
        SAJ_ASSERT(false, "Fail to load shaders")
        return false;
    }

    glShaderSource(vertShaderID, 1, &vertShader, 0);
    glShaderSource(fragShaderID, 1, &fragShader, 0);
    glCompileShader(vertShaderID);
    glCompileShader(fragShaderID);
    //test if shaders compiled successfully
    {
        int sucess;
        char shaderLog[2048] = {};

        glGetShaderiv(vertShaderID, GL_COMPILE_STATUS, &sucess);
        if(!sucess)
        {
            glGetShaderInfoLog(vertShaderID, 2048, 0, shaderLog);
            SAJ_ASSERT(false, "Compilation of vertex shader failed. \n %s", shaderLog)
            return false;
        }

        glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &sucess);
        if(!sucess)
        {  
            glGetShaderInfoLog(fragShaderID, 2048, 0, shaderLog);
            SAJ_ASSERT(false, "Compilation of vertex shader failed. \n %s", shaderLog)
            return false;
        }
    }

        glContext.programID = glCreateProgram();
        glAttachShader(glContext.programID, vertShaderID);
        glAttachShader(glContext.programID, fragShaderID);
        glLinkProgram(glContext.programID);

        glDetachShader(glContext.programID, vertShaderID);
        glDetachShader(glContext.programID, fragShaderID);


        // This has to be done even if vertices are definied in shader directly
        // otherwise nothing show up
        GLuint VAO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        
        //Texture loading using STBI
        {
            int width, height, channels;
            char* data = (char*) stbi_load(TEXTURE_PATH, &width, &height, &channels, 4);
            if(!data)
            {
                SAJ_ASSERT(false, "Failed to load data")
                return false;
            }
            glGenTextures(1, &glContext.textureID);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, glContext.textureID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S , GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
            stbi_image_free(data);
        }

        {
              
            glGenBuffers(1, &glContext.sTransformSBOID);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glContext.sTransformSBOID);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SpriteTransform) * MAX_SPRITE_TRANSFORMS,
                        renderData.transforms, GL_DYNAMIC_DRAW);
  
        }
        //Uniforms
        {
            glContext.screenSizeID = glGetUniformLocation(glContext.programID, "screenSize");
        }

        glEnable(GL_FRAMEBUFFER_SRGB);
        glDisable(0x809D);

        //Depth testing
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GREATER);

        glUseProgram(glContext.programID);
    return true;
}


void gl_render()
{
    glClearColor(119.0f / 255.0f, 33.0f / 255.0f, 111.0f / 255.0f, 1.0f);
    glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,input.screenSizeX, input.screenSizeY);
    Vec2 screenSize = {(float)input.screenSizeX, (float)input.screenSizeY};
    glUniform2fv(glContext.screenSizeID, 1, &screenSize.x);

    {
        //Copy transform to the GPU
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(SpriteTransform) * renderData.sTransformCount,renderData.transforms);
        
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, renderData.sTransformCount);

        //reset for next frame 
        renderData.sTransformCount = 0;

    }
}