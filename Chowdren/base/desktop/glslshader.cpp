#include "shader.h"
#include "frameobject.h"
#include "manager.h"
#include "chowconfig.h"
#include "fileio.h"
#include "image.h"
#include "glslshader.h"
#include "collision.h"

static bool background_initialized = false;
static GLuint background_texture;

static void initialize_background()
{
    background_initialized = true;
    glGenTextures(1, &background_texture);
    glBindTexture(GL_TEXTURE_2D, background_texture);
#ifdef CHOWDREN_QUICK_SCALE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

GLSLShader * GLSLShader::current = NULL;

GLSLShader::GLSLShader(const std::string & name, int flags,
                       const char * texture_parameter)
: initialized(false), name(name), flags(flags),
  texture_parameter(texture_parameter)
{
}

void GLSLShader::initialize()
{
    program = glCreateProgram();
    GLuint vert_shader = attach_source("vert", GL_VERTEX_SHADER);
    GLuint frag_shader = attach_source("frag", GL_FRAGMENT_SHADER);

#ifndef CHOWDREN_USE_GL
    glBindAttribLocation(program, POSITION_ATTRIB_IDX, POSITION_ATTRIB_NAME);
    glBindAttribLocation(program, TEXCOORD1_ATTRIB_IDX, TEXCOORD1_ATTRIB_NAME);
    glBindAttribLocation(program, TEXCOORD2_ATTRIB_IDX, TEXCOORD2_ATTRIB_NAME);
    glBindAttribLocation(program, COLOR_ATTRIB_IDX, COLOR_ATTRIB_NAME);
#endif

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

    if ((flags & SHADER_HAS_BACK) && !background_initialized)
        initialize_background();

    glUseProgram(program);

    // setup uniforms
    glUniform1i((GLint)get_uniform(TEXTURE_SAMPLER_NAME), 0);

    if (flags & SHADER_HAS_BACK)
        glUniform1i((GLint)get_uniform(BACKTEX_SAMPLER_NAME), 1);
    if (flags & SHADER_HAS_TEX_SIZE)
        size_uniform = (GLint)get_uniform(SIZE_UNIFORM_NAME);

    if (texture_parameter != NULL) {
        glUniform1i((GLint)get_uniform(texture_parameter), 2);
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
    if (!(flags & SHADER_HAS_BACK))
        return 0;
    return background_texture;
}

bool GLSLShader::has_texture_param()
{
    return texture_parameter != NULL;
}

GLuint GLSLShader::attach_source(const std::string & ext, GLenum type)
{
    GLuint shader = glCreateShader(type);

    std::string path = shader_path + "/" + name + "." + ext;
    std::cout << "Compiling " << path << std::endl;

    GLchar * data;
    size_t length;

    read_file(path.c_str(), &data, &length, false);

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

void GLSLShader::begin(FrameObject * instance, int width, int height)
{
    if (!initialized)
        initialize();

    if (flags & SHADER_HAS_BACK) {
        int box[4];
        instance->get_screen_aabb(box);
        glc_copy_color_buffer_rect(background_texture, box[0], box[1],
                                   box[2], box[3]);
    }

    glUseProgram(program);

    if (flags & SHADER_HAS_TEX_SIZE)
        glUniform2f(size_uniform, 1.0f / width, 1.0f / height);

    set_parameters(instance);

    current = this;
}

void GLSLShader::set_parameters(FrameObject * instance)
{

}

void GLSLShader::end(FrameObject * instance)
{
    current = NULL;

    glUseProgram(0);
}

void GLSLShader::set_int(FrameObject * instance, const std::string & name,
                           int uniform)
{
    int val = (int)(*instance->shader_parameters)[name];
    glUniform1i((GLint)uniform, val);
}

void GLSLShader::set_float(FrameObject * instance, const std::string & name,
                           int uniform)
{
    glUniform1f((GLint)uniform, (*instance->shader_parameters)[name]);
}

void GLSLShader::set_vec4(FrameObject * instance, const std::string & name,
                          int uniform)
{
    int val = (int)(*instance->shader_parameters)[name];
    float a, b, c, d;
    convert_vec4(val, a, b, c, d);
    glUniform4f((GLint)uniform, a, b, c, d);
}

int GLSLShader::get_uniform(const char * value)
{
    return glGetUniformLocation(program, value);
}
