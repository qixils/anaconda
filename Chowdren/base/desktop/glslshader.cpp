#include "../shader.h"
#include "../frameobject.h"
#include "../manager.h"
#include "../config.h"
#include "../filecommon.h"
#include "../image.h"
#include "glslshader.h"

static bool background_initialized = false;
static GLuint background_fbo;
static GLuint background_texture;

void initialize_background()
{
    background_initialized = true;
    glGenTextures(1, &background_texture);
    glBindTexture(GL_TEXTURE_2D, background_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenFramebuffersEXT(1, &background_fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, background_fbo);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
        GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, background_texture, 0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, global_manager->screen_fbo);
}


GLSLShader::GLSLShader(const std::string & name, bool has_back) 
: initialized(false), name(name), has_background(has_back)
{
}

void GLSLShader::initialize()
{
    program = glCreateProgram();
    GLuint vert_shader = attach_source("vert", GL_VERTEX_SHADER);
    GLuint frag_shader = attach_source("frag", GL_FRAGMENT_SHADER);
    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint info_len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_len);
        GLchar * info_log = new GLchar[info_len + 1];
        glGetProgramInfoLog(program, info_len, NULL, info_log);
        std::cout << "Linker failure: " << info_log << std::endl;
        delete[] info_log;
    }
    
    glDetachShader(program, vert_shader);
    glDetachShader(program, frag_shader);

    if (has_background && !background_initialized)
        initialize_background();

    glUseProgram(program);
    glUniform1i((GLint)get_uniform("texture"), 0);

    if (has_background) {
        glUniform1i((GLint)get_uniform("background_texture"), 1);
        size_uniform = (GLint)get_uniform("texture_size");
    }

    glUseProgram(0);

    initialize_parameters();

    initialized = true;
}

void GLSLShader::initialize_parameters() 
{
}

GLuint GLSLShader::get_background_texture()
{
    if (!has_background)
        return 0;
    return background_texture;
}

GLuint GLSLShader::attach_source(const std::string & ext, GLenum type)
{
    GLuint shader = glCreateShader(type);

    std::string path = shader_path + "/" + name + "." + ext;

    GLchar * data;
    size_t length;

    read_file(path.c_str(), &data, &length);

    GLint len = length;

    glShaderSource(shader, 1, (const GLchar**)&data, &len);
    delete[] data;
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint info_len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
        GLchar * info_log = new GLchar[info_len + 1];
        glGetShaderInfoLog(shader, info_len, NULL, info_log);            
        std::cout << "Compile error in " << type << ":" << std::endl <<
            info_log << std::endl;
        delete[] info_log;
    } else {
        glAttachShader(program, shader);
    }
    return shader;
}

void GLSLShader::begin(FrameObject * instance, Image * image)
{
    if (!initialized)
        initialize();

    if (has_background) {
        int box[4];
        instance->get_box(box);
        int width = box[2] - box[0];
        int height = box[3] - box[1];

        glBindTexture(GL_TEXTURE_2D, background_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
            0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glPushAttrib(GL_VIEWPORT_BIT | GL_TRANSFORM_BIT);
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, background_fbo);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
            GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, background_texture, 0);

#if 0
        GLenum result = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        if (result != GL_FRAMEBUFFER_COMPLETE_EXT)
            std::cout << "framebuffer incomplete: " << result << std::endl;
#endif

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, global_manager->screen_texture);
        glDisable(GL_BLEND);
        glBegin(GL_QUADS);
        glTexCoord2f(box[0] / float(WINDOW_WIDTH), 
                     1.0 - box[3] / float(WINDOW_HEIGHT));
        glVertex2i(0, 0);
        glTexCoord2f(box[2] / float(WINDOW_WIDTH), 
                     1.0 - box[3] / float(WINDOW_HEIGHT));
        glVertex2i(width, 0);
        glTexCoord2f(box[2] / float(WINDOW_WIDTH),
                     1.0 - box[1] / float(WINDOW_HEIGHT));
        glVertex2i(width, height);
        glTexCoord2f(box[0] / float(WINDOW_WIDTH),
                     1.0 - box[1] / float(WINDOW_HEIGHT));
        glVertex2i(0, height);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glPopAttrib(); // restores MODELVIEW too
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 
                             global_manager->screen_fbo);
    }

    glUseProgram(program);

    if (has_background)
        glUniform2f(size_uniform, 1.0f / image->width, 
                                  1.0f / image->height);

    set_parameters(instance);
}

void GLSLShader::set_parameters(FrameObject * instance) 
{

}

void GLSLShader::end(FrameObject * instance)
{
    glUseProgram(0);
}

void GLSLShader::set_float(FrameObject * instance, const std::string & name,
                           void * uniform)
{
    glUniform1f((GLint)uniform, (*instance->shader_parameters)[name]);
}

void GLSLShader::set_vec4(FrameObject * instance, const std::string & name,
                          void * uniform)
{
    int val = (int)(*instance->shader_parameters)[name];
    float a, b, c, d;
    convert_vec4(val, a, b, c, d);
    glUniform4f((GLint)uniform, a, b, c, d);
}

void * GLSLShader::get_uniform(const char * value)
{
    return (void*)glGetUniformLocation(program, value);
}