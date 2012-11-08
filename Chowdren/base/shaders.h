#include "include_gl.h"
#include "frameobject.h"
#include "filecommon.h"

static GLuint background_texture = 0;

void initialize_background_texture()
{
    glGenTextures(1, &background_texture);
    glBindTexture(GL_TEXTURE_2D, background_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

class Shader
{
public:
    virtual void begin(FrameObject * instance) {}
    virtual void end(FrameObject * instance) {}
    virtual GLuint get_background_texture()
    {
        return 0;
    }
};

class GLSLShader : public Shader
{
public:
    GLuint program;
    std::string name;
    bool initialized;
    bool has_background;

    GLSLShader(const std::string & name, bool has_back = false) 
    : initialized(false), name(name), has_background(has_back)
    {
    }

    void initialize()
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


        if (has_background && background_texture == 0)
            initialize_background_texture();

        initialized = true;
    }

    GLuint get_background_texture()
    {
        if (!has_background)
            return 0;
        return background_texture;
    }

    GLuint attach_source(const std::string & ext, GLenum type)
    {
        GLuint shader = glCreateShader(type);

        std::string path = "./shaders/" + name + "." + ext;

        GLchar * data;
        size_t length;

        read_file(path.c_str(), &data, &length);

        GLint len = length;

        glShaderSource(shader, 1, (const GLchar**)&data, &len);
        delete data;
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

    void begin(FrameObject * instance)
    {
        if (!initialized)
            initialize();
        glUseProgram(program);

        int box[4];
        instance->get_box(box);
        int width = box[2] - box[0];
        int height = box[3] - box[1];
        if (has_background) {
            glUniform1i(get_uniform("texture"), 0);
            glUniform1i(get_uniform("background_texture"), 1);
            glBindTexture(GL_TEXTURE_2D, background_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                GL_UNSIGNED_BYTE, 0);
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
                box[0], WINDOW_HEIGHT - box[3], width, height);
        }
        glUniform2f(get_uniform("texture_size"), width, height);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
            std::cout << "OpenGL error: " << error << std::endl;
        set_parameters(instance);
    }

    virtual void set_parameters(FrameObject * instance) {}

    void end(FrameObject * instance)
    {
        glUseProgram(0);
    }

    void set_float(FrameObject * instance, const std::string & name)
    {
        glUniform1f(get_uniform(name), (*instance->shader_parameters)[name]);
    }

    void set_vec4(FrameObject * instance, const std::string & name)
    {
        int val = (int)(*instance->shader_parameters)[name];
        float a, b, c, d;
        a = (val & 0xFF) / 255.0f;
        b = ((val >> 8) & 0xFF) / 255.0f;
        c = ((val >> 16) & 0xFF) / 255.0f;
        d = ((val >> 24) & 0xFF) / 255.0f;
        glUniform4f(get_uniform(name), a, b, c, d);
    }

    GLint get_uniform(const std::string & value)
    {
        return glGetUniformLocation(program, value.c_str());
    }

    GLint get_uniform(const char * value)
    {
        return glGetUniformLocation(program, value);
    }
};

// implementations

class SubtractShader : public Shader
{
public:
    void begin(FrameObject * instance)
    {
        glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
        glBlendFunc(GL_DST_COLOR, GL_ONE);
    }

    void end(FrameObject * instance)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
    }
};

class AdditiveShader : public Shader
{
public:
    void begin(FrameObject * instance)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }

    void end(FrameObject * instance)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
    }
};

class MonochromeShader : public GLSLShader
{
public:
    MonochromeShader() : GLSLShader("monochrome") {}
};

class MixerShader : public GLSLShader
{
public:
    MixerShader() : GLSLShader("colormixer") {}

    void set_parameters(FrameObject * instance) 
    {
        set_vec4(instance, "r");
        set_vec4(instance, "g");
        set_vec4(instance, "b");
    }
};

class HueShader : public GLSLShader
{
public:
    HueShader() : GLSLShader("hue") {}

    void set_parameters(FrameObject * instance) 
    {
        set_float(instance, "fHue");
    }
};

class OffsetShader : public GLSLShader
{
public:
    OffsetShader() : GLSLShader("offset", true) {}

    void set_parameters(FrameObject * instance) 
    {
        set_float(instance, "width");
        set_float(instance, "height");
    }
};

static SubtractShader subtract_shader;
static AdditiveShader additive_shader;
static MonochromeShader monochrome_shader;
static MixerShader mixer_shader;
static HueShader hue_shader;
static Shader dummy_shader;
static Shader blend_shader;
static OffsetShader offset_shader;
static Shader dodgeblur_shader;