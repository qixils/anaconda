#define CHOWDREN_BUILD_GLC
#include "chowconfig.h"
#include "../include_gl.h"
#include "../mathcommon.h"
#include "glslshader.h"
#include <vector>
#include <string.h>

struct Vec2
{
    float x, y;
};

struct Vec3
{
    float x, y, z;
};

struct Vec4
{
    float x, y, z, w;
};

typedef GLfloat Mat4x4[16];

struct MatContainer
{
    Mat4x4 m;
};

typedef Vec3 Quad[4];
typedef Vec2 TexQuad[4];
typedef std::vector<MatContainer> Stack;

void glc_copy_color_buffer_rect(unsigned int tex, int x1, int y1, int x2,
                                int y2)
{
    int width = x2 - x1;
    int height = y2 - y1;

    int y = WINDOW_HEIGHT - y2;

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                    0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x1, y, width, height);
}

void glc_init_shaders()
{
}

inline void mult_matrix(Mat4x4 & m, Mat4x4 & n, Mat4x4 & d)
{
    d[0] = m[0]*n[0] + m[4]*n[1] + m[8]*n[2] + m[12]*n[3];
    d[1] = m[1]*n[0] + m[5]*n[1] + m[9]*n[2] + m[13]*n[3];
    d[2] = m[2]*n[0] + m[6]*n[1] + m[10]*n[2] + m[14]*n[3];
    d[3] = m[3]*n[0] + m[7]*n[1] + m[11]*n[2] + m[15]*n[3];
    d[4] = m[0]*n[4] + m[4]*n[5] + m[8]*n[6] + m[12]*n[7];
    d[5] = m[1]*n[4] + m[5]*n[5] + m[9]*n[6] + m[13]*n[7];
    d[6] = m[2]*n[4] + m[6]*n[5] + m[10]*n[6] + m[14]*n[7];
    d[7] = m[3]*n[4] + m[7]*n[5] + m[11]*n[6] + m[15]*n[7];
    d[8] = m[0]*n[8] + m[4]*n[9] + m[8]*n[10] + m[12]*n[11];
    d[9] = m[1]*n[8] + m[5]*n[9] + m[9]*n[10] + m[13]*n[11];
    d[10] = m[2]*n[8] + m[6]*n[9] + m[10]*n[10] + m[14]*n[11];
    d[11] = m[3]*n[8] + m[7]*n[9] + m[11]*n[10] + m[15]*n[11];
    d[12] = m[0]*n[12] + m[4]*n[13] + m[8]*n[14] + m[12]*n[15];
    d[13] = m[1]*n[12] + m[5]*n[13] + m[9]*n[14] + m[13]*n[15];
    d[14] = m[2]*n[12] + m[6]*n[13] + m[10]*n[14] + m[14]*n[15];
    d[15] = m[3]*n[12] + m[7]*n[13] + m[11]*n[14] + m[15]*n[15];
}

inline void mult_matrix(Mat4x4 & m, Vec3 & v, Vec3 & d)
{
    d.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12];
    d.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13];
    d.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14];
}

class GLState
{
public:
#ifdef CHOWDREN_USE_GLES2
    bool tex_on;
    GLenum matrix_mode;
    Stack projection_stack;
    Stack modelview_stack;
    Mat4x4 projection;
    Mat4x4 modelview;

    inline Mat4x4 & get_mat()
    {
        switch (matrix_mode) {
            case GL_PROJECTION:
                return projection;
            case GL_MODELVIEW:
            default:
                return modelview;
        }
    }

    inline Stack & get_stack()
    {
        switch (matrix_mode) {
            case GL_PROJECTION:
                return projection_stack;
            case GL_MODELVIEW:
            default:
                return modelview_stack;
        }
    }

    inline void mult_matrix(Mat4x4 & n)
    {
        Mat4x4 & d = get_mat();
        Mat4x4 m;
        memcpy(m, d, sizeof(Mat4x4));
        ::mult_matrix(m, n, d);
    }
#endif

    Vec4 current_color;
    int vert_index;
    Quad vertices;
    int tc_index[2];
    TexQuad texcoords[2];

    GLState()
    : tex_on(false), vert_index(0)
    {
        tc_index[0] = tc_index[1] = 0;
    }
};

static GLState gl_state;

void glc_scissor_world(int x, int y, int w, int h)
{
#ifdef CHOWDREN_USE_GL
    GLfloat modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
#else
    Mat4x4 & modelview = gl_state.get_mat();
#endif

    int w_x = int(x + modelview[12]);
    int w_y2 = int(WINDOW_HEIGHT - y - modelview[13]);
    int w_x2 = w_x + w;
    int w_y = w_y2 - h;

    w_x = int_max(0, int_min(w_x, WINDOW_WIDTH));
    w_x2 = int_max(0, int_min(w_x2, WINDOW_WIDTH));
    w_y = int_max(0, int_min(w_y, WINDOW_HEIGHT));
    w_y2 = int_max(0, int_min(w_y2, WINDOW_HEIGHT));

    glScissor(w_x, w_y, w_x2 - w_x, w_y2 - w_y);
}

void glc_enable(GLenum cap)
{
    switch (cap) {
        case GL_TEXTURE_2D:
            gl_state.tex_on = true;
            return;
        default:
            break;
    }
    glEnable(cap);
}

void glc_disable(GLenum cap)
{
    switch (cap) {
        case GL_TEXTURE_2D:
            gl_state.tex_on = false;
            return;
        default:
            break;
    }
    glDisable(cap);
}

void glc_matrix_mode(GLenum mode)
{
    gl_state.matrix_mode = mode;
}

Mat4x4 mvp;

void glc_vertex_3f(GLfloat x, GLfloat y, GLfloat z)
{
    Vec3 v1 = {x, y, z};
    Vec3 v2;
    mult_matrix(mvp, v1, v2);
    gl_state.vertices[gl_state.vert_index++] = v2;
}

void glc_texcoord_2f(GLfloat s, GLfloat t)
{
    Vec2 tc = {s, t};
    gl_state.texcoords[0][gl_state.tc_index[0]++] = tc;
}

void glc_multi_texcoord_2f(GLenum target, GLfloat s, GLfloat t)
{
    Vec2 tc = {s, t};
    gl_state.texcoords[1][gl_state.tc_index[1]++] = tc;
}

void glc_begin(GLenum mode)
{
    gl_state.vert_index = 0;
    gl_state.tc_index[0] = gl_state.tc_index[1] = 0;
    mult_matrix(gl_state.projection, gl_state.modelview, mvp);
}

void glc_end()
{
    bool is_platform = false;
    GLSLShader * shader = GLSLShader::current;
    if (shader == NULL) {
        is_platform = true;
        if (gl_state.tex_on) {
            shader = (GLSLShader*)texture_shader;
        } else {
            shader = (GLSLShader*)basic_shader;
        }
        shader->begin(NULL, NULL); // we don't actually need these parameters
    }

    glEnableVertexAttribArray(POSITION_ATTRIB_IDX);
    glVertexAttribPointer(POSITION_ATTRIB_IDX, 3, GL_FLOAT, GL_FALSE, 0,
                          (void*)gl_state.vertices);

    if (shader != basic_shader) {
        glEnableVertexAttribArray(TEXCOORD1_ATTRIB_IDX);
        glVertexAttribPointer(TEXCOORD1_ATTRIB_IDX, 2, GL_FLOAT, GL_FALSE, 0,
                              (void*)&gl_state.texcoords[0]);
    }

    if (shader->has_background) {
        glEnableVertexAttribArray(TEXCOORD2_ATTRIB_IDX);
        glVertexAttribPointer(TEXCOORD1_ATTRIB_IDX, 2, GL_FLOAT, GL_FALSE, 0,
                              (void*)&gl_state.texcoords[1]);
    }

    if (shader->blend_color != -1)
        glUniform4fv((GLint)shader->blend_color, 1,
                     (const GLfloat*)&gl_state.current_color);


    // drawing
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(POSITION_ATTRIB_IDX);
    if (shader != basic_shader)
        glDisableVertexAttribArray(TEXCOORD1_ATTRIB_IDX);
    if (shader->has_background)
        glDisableVertexAttribArray(TEXCOORD2_ATTRIB_IDX);

    if (is_platform)
        shader->end(NULL);
}

void glc_load_identity()
{
    Mat4x4 & m = gl_state.get_mat();
    m[0] = 1.0f;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;

    m[4] = 0.0f;
    m[5] = 1.0f;
    m[6] = 0.0f;
    m[7] = 0.0f;

    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = 1.0f;
    m[11] = 0.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

void glc_ortho(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top,
               GLfloat near, GLfloat far)
{
    Mat4x4 m;
    GLfloat one_over_rml = 1.0 / (right - left);
    GLfloat one_over_fmn = 1.0 / (far - near);
    GLfloat one_over_tmb = 1.0 / (top - bottom);

    m[0] = 2.0 * one_over_rml;
    m[1] = 0.0;
    m[2] = 0.0;
    m[3] = 0.0;

    m[4] = 0.0;
    m[5] = 2.0 * one_over_tmb;
    m[6] = 0.0;
    m[7] = 0.0;

    m[8] = 0.0;
    m[9] = 0.0;
    m[10] = -2.0 * one_over_fmn;
    m[11] = 0.0;

    m[12] = -(right + left) *  one_over_rml;
    m[13] = -(top + bottom) *  one_over_tmb;
    m[14] = -(far + near) * one_over_fmn;
    m[15] = 1.0;

    gl_state.mult_matrix(m);
}

void glc_color_4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    Vec4 current_color = {red, green, blue, alpha};
    gl_state.current_color = current_color;
}

void glc_color_4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    Vec4 current_color = {red / 255.0f, green / 255.0f, blue / 255.0f, 
                          alpha / 255.0f};
    gl_state.current_color = current_color;
}

void glc_translate_3f(GLfloat x, GLfloat y, GLfloat z)
{
    Mat4x4 & m = gl_state.get_mat();
    m[12] = m[0] * x + m[4] * y + m[8] * z + m[12];
    m[13] = m[1] * x + m[5] * y + m[9] * z + m[13];
    m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];
    m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];
}

void glc_push_matrix()
{
    Stack & stack = gl_state.get_stack();
    size_t index = stack.size();
    stack.resize(index + 1);
    memcpy(&stack[index], gl_state.get_mat(), sizeof(Mat4x4));
}

void glc_pop_matrix()
{
    memcpy(gl_state.get_mat(), &gl_state.get_stack().back(), sizeof(Mat4x4));
    gl_state.get_stack().pop_back();
}

void glc_scale_f(GLfloat x, GLfloat y, GLfloat z)
{
    Mat4x4 & m = gl_state.get_mat();
    m[0] *= x;
    m[1] *= x;
    m[2] *= x;
    m[3] *= x;
    m[4] *= y;
    m[5] *= y;
    m[6] *= y;
    m[7] *= y;
    m[8] *= z;
    m[9] *= z;
    m[10] *= z;
    m[11] *= z;
}

void glc_rotate_f(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    // XXX only rotates by z right now
    Mat4x4 & m = gl_state.get_mat();
    float c = cos(rad(angle));
    float s = sin(rad(angle));
    float a00 = m[0];
    float a01 = m[1];
    float a02 = m[2];
    float a03 = m[3];
    float a10 = m[4];
    float a11 = m[5];
    float a12 = m[6];
    float a13 = m[7];

    m[0] = a00 * c + a10 * s;
    m[1] = a01 * c + a11 * s;
    m[2] = a02 * c + a12 * s;
    m[3] = a03 * c + a13 * s;
    m[4] = a00 * -s + a10 * c;
    m[5] = a01 * -s + a11 * c;
    m[6] = a02 * -s + a12 * c;
    m[7] = a03 * -s + a13 * c;
}
