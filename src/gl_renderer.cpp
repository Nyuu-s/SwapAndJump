#include "gl_renderer.h"


//########################################################################
// OpenGL Structs
//########################################################################
struct GLContext
{
    GLuint programID;

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
    if(severity == GL_DEBUG_SEVERITY_LOW || GL_DEBUG_SEVERITY_MEDIUM || GL_DEBUG_SEVERITY_HIGH)
    {
        SAJ_ASSERT(false, "OpenGL Error: %s", message)
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
        
        glUseProgram(glContext.programID);
        
        //Depth testing
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GREATER);
    return true;
}


void gl_render()
{
    glClearColor(119.0f / 255.0f, 33.0f / 255.0f, 111.0f / 255.0f, 1.0f);
    glClearDepth(0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glViewport(0,0,input.screenSizeX, input.screenSizeY);
    glDrawArrays(GL_TRIANGLES, 0,6);
}