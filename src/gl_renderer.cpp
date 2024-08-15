#include "gl_renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "gl_render_interface.h"



//########################################################################
// OpenGL constants
//########################################################################
const char* TEXTURE_PATH = "src/assets/textures/T_SPRITESET.png";
char* SHADER_VERT_PATH = "src/assets/shaders/quad.vert";
char* SHADER_FRAG_PATH = "src/assets/shaders/quad.frag";

//########################################################################
// OpenGL Structs
//########################################################################
struct GLContext
{
    GLuint programID;
    GLuint textureID;
    GLuint sTransformSBOID;
    GLuint screenSizeID;
    GLuint orthoProjectionID;

    long long textureTimeStamp;
    long long shaderTimeStamp;
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

GLuint gl_create_shader(int shaderType, char* shaderPath, BumpAllocator* transientStorage)
{
    int fileSize = 0;
    char* shader = read_file(shaderPath, &fileSize, transientStorage);
    if(!shader)
    {
        SAJ_ASSERT(false, "failed to load shader: %s", shaderPath)
        return 0;
    }
    GLuint shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, &shader, 0);
    glCompileShader(shaderID);
    {
        int sucess;
        char shaderLog[2048] = {};

        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &sucess);
        if(!sucess)
        {
            glGetShaderInfoLog(shaderID, 2048, 0, shaderLog);
            SAJ_ASSERT(false, "Compilation of vertex shader failed. \n %s", shaderLog)

            return 0;
        }
    }
    return shaderID;
}

bool gl_init(BumpAllocator* transientStorage)
{
    gl_load_functions();

    glDebugMessageCallback(&gl_debug_callback, nullptr);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glEnable(GL_DEBUG_OUTPUT);

    GLuint vertShaderID =  gl_create_shader(GL_VERTEX_SHADER, SHADER_VERT_PATH, transientStorage); 
    GLuint fragShaderID = gl_create_shader(GL_FRAGMENT_SHADER, SHADER_FRAG_PATH, transientStorage);

    if(!vertShaderID || !fragShaderID)
    {
        SAJ_ASSERT(false, "Failed to create shaders")
        return false;
    }

    long long timestampVert = get_timestamp(SHADER_VERT_PATH);
    long long timestampFrag = get_timestamp(SHADER_FRAG_PATH);
    glContext.shaderTimeStamp = max_LL(timestampVert, timestampFrag);

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

        glContext.textureTimeStamp = get_timestamp(TEXTURE_PATH);
        stbi_image_free(data);
    }

    {
            
        glGenBuffers(1, &glContext.sTransformSBOID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glContext.sTransformSBOID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SpriteTransform) * renderData->transforms.maxCount,
                    renderData->transforms.elements, GL_DYNAMIC_DRAW);

    }
    //Uniforms
    {
        glContext.screenSizeID = glGetUniformLocation(glContext.programID, "screenSize");
        glContext.orthoProjectionID = glGetUniformLocation(glContext.programID, "orthoProjection");
    }

    glEnable(GL_FRAMEBUFFER_SRGB);
    glDisable(0x809D);

    //Depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);

    glUseProgram(glContext.programID);
    return true;
}


void gl_render(BumpAllocator* transientStorage)
{
    //Textures hot reloading
    {
        long long currentTimeStamp = get_timestamp(TEXTURE_PATH);

        if(currentTimeStamp > glContext.textureTimeStamp)
        {
            glActiveTexture(GL_TEXTURE0);
            int width, height, nChannels;
            char* data = (char*)stbi_load(TEXTURE_PATH, &width,&height, &nChannels, 4);
            if(data)
            {
                glContext.textureTimeStamp = currentTimeStamp;
                glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
        }
    }
    // Shader hot realoding 
    {
        long long timestampVert = get_timestamp(SHADER_VERT_PATH);
        long long timestampFrag = get_timestamp(SHADER_FRAG_PATH);

        if(timestampVert > glContext.shaderTimeStamp || timestampFrag > glContext.shaderTimeStamp)
        {
            GLuint vertShaderID = gl_create_shader(GL_VERTEX_SHADER, SHADER_VERT_PATH, transientStorage);
            GLuint fragShaderID = gl_create_shader(GL_FRAGMENT_SHADER, SHADER_FRAG_PATH, transientStorage);

            if(!vertShaderID || !fragShaderID)
            {
                SAJ_ASSERT(false, "Failed to create shaders")
                return;
            }

            glAttachShader(glContext.programID, vertShaderID);
            glAttachShader(glContext.programID, fragShaderID);

            glLinkProgram(glContext.programID);

            glDetachShader(glContext.programID, vertShaderID);
            glDetachShader(glContext.programID, fragShaderID);

            glDeleteShader(vertShaderID);
            glDeleteShader(fragShaderID);

            glContext.shaderTimeStamp = max_LL(timestampFrag, timestampVert);
        }

    }
    glClearColor(119.0f / 255.0f, 33.0f / 255.0f, 111.0f / 255.0f, 1.0f);
    glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,input->ScreenSize.x, input->ScreenSize.y);

    //copy screensize to gpu
    Vec2 screenSize = {(float)input->ScreenSize.x, (float)input->ScreenSize.y};
    glUniform2fv(glContext.screenSizeID, 1, &screenSize.x);

    //orthographic projection 
    OrthoCamera2D camera = renderData->gameCamera;
    Mat4 orthoProjection = orthographic_projection(camera.position.x - camera.dimensions.x / 2.0f, 
                                                    camera.position.x + camera.dimensions.x / 2.0f, 
                                                    camera.position.y - camera.dimensions.y / 2.0f, 
                                                    camera.position.y + camera.dimensions.y / 2.0f);
  
    glUniformMatrix4fv(glContext.orthoProjectionID, 1, GL_FALSE, &orthoProjection.ax);

    {
        //Copy transform to the GPU
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(SpriteTransform) * renderData->transforms.count,renderData->transforms.elements);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, renderData->transforms.count);
        

        //reset for next frame 
        renderData->transforms.clear();

    }
}