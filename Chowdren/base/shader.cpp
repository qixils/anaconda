#include "shader.h"
#include "include_gl.h"
#include "platform.h"
#include "frameobject.h"
#include "assetfile.h"

#include "shaderparam.cpp"
#include "shaderparam.h"

void convert_vec4(int val, float & a, float & b, float & c, float & d)
{
    a = (val & 0xFF) / 255.0f;
    b = ((val >> 8) & 0xFF) / 255.0f;
    c = ((val >> 16) & 0xFF) / 255.0f;
    d = ((val >> 24) & 0xFF) / 255.0f;
}

class AdditiveShader : public Shader
{
public:
    void begin(FrameObject * instance, int width, int height)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }

    void end(FrameObject * instance)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
};

#include "glslshader.h"

void GLSLShader::set_image(FrameObject * instance, int src)
{
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, (GLuint)instance->get_shader_parameter(src));
    glActiveTexture(GL_TEXTURE0);
}

// shader implementations

class SubtractShader : public GLSLShader
{
public:
    SubtractShader()
    : GLSLShader(SHADER_SUBTRACT)
    {
    }

    void begin(FrameObject * instance, int width, int height)
    {
        // MMF2 default
        // glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
        glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
        glBlendFunc(GL_DST_COLOR, GL_ONE);
        GLSLShader::begin(instance, width, height);
    }

    void end(FrameObject * instance)
    {
        // MMF2 default
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        GLSLShader::end(instance);
    }
};

class MonochromeShader : public GLSLShader
{
public:
    MonochromeShader()
    : GLSLShader(SHADER_MONOCHROME)
    {

    }
};

class MixerShader : public GLSLShader
{
public:
    int r;
    int g;
    int b;

    MixerShader()
    : GLSLShader(SHADER_COLORMIXER)
    {

    }

    void initialize_parameters()
    {
        r = get_uniform("r");
        g = get_uniform("g");
        b = get_uniform("b");
    }

    void set_parameters(FrameObject * instance)
    {
        set_vec4(instance, SHADER_PARAM_R, r);
        set_vec4(instance, SHADER_PARAM_G, g);
        set_vec4(instance, SHADER_PARAM_B, b);
    }
};

class HueShader : public GLSLShader
{
public:
    int hue;

    HueShader()
    : GLSLShader(SHADER_HUE)
    {

    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_FHUE, hue);
    }

    void initialize_parameters()
    {
        hue = get_uniform("fHue");
    }
};

class OffsetShader : public GLSLShader
{
public:
    int width;
    int height;

    OffsetShader()
    : GLSLShader(SHADER_OFFSET, SHADER_HAS_BACK | SHADER_HAS_TEX_SIZE)
    {

    }

    void initialize_parameters()
    {
        width = get_uniform("width");
        height = get_uniform("height");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_WIDTH, width);
        set_float(instance, SHADER_PARAM_HEIGHT, height);
    }
};

class InvertShader : public GLSLShader
{
public:
    InvertShader()
    : GLSLShader(SHADER_INVERT)
    {

    }
};

class DodgeBlurShader : public GLSLShader
{
public:
    int vertical;
    int radius;

    DodgeBlurShader()
    : GLSLShader(SHADER_DODGEBLUR, SHADER_HAS_BACK | SHADER_HAS_TEX_SIZE)
    {

    }

    void initialize_parameters()
    {
        vertical = get_uniform("vertical");
        radius = get_uniform("radius");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_VERTICAL, vertical);
        set_float(instance, SHADER_PARAM_RADIUS, radius);
    }
};

class GrainShader : public GLSLShader
{
public:
    int strength, seed;
    int invert;
    int r, g, b, a;

    GrainShader()
    : GLSLShader(SHADER_GRAIN)
    {

    }

    void initialize_parameters()
    {
        strength = get_uniform("fStrength");
        seed = get_uniform("fSeed");
        invert = get_uniform("iInvert");
        r = get_uniform("iR");
        g = get_uniform("iG");
        b = get_uniform("iB");
        a = get_uniform("iA");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_FSTRENGTH, strength);
        set_float(instance, SHADER_PARAM_FSEED, seed);
        set_int(instance, SHADER_PARAM_IINVERT, invert);
        set_int(instance, SHADER_PARAM_IR, r);
        set_int(instance, SHADER_PARAM_IG, g);
        set_int(instance, SHADER_PARAM_IB, b);
        set_int(instance, SHADER_PARAM_IA, a);
    }
};

class MultiplyShader : public GLSLShader
{
public:
    MultiplyShader()
    : GLSLShader(SHADER_MULTIPLY, SHADER_HAS_BACK)
    {
    }
};

class HardLightShader : public GLSLShader
{
public:
    HardLightShader()
    : GLSLShader(SHADER_HARDLIGHT, SHADER_HAS_BACK)
    {
    }
};

class TintShader : public GLSLShader
{
public:
    int tint_color;
    int tint_power;
    int original_power;

    TintShader()
    : GLSLShader(SHADER_TINT)
    {

    }

    void initialize_parameters()
    {
        tint_color = get_uniform("fTintColor");
        tint_power = get_uniform("fTintPower");
        original_power = get_uniform("fOriginalPower");
    }

    void set_parameters(FrameObject * instance)
    {
        set_vec4(instance, SHADER_PARAM_FTINTCOLOR, tint_color);
        set_float(instance, SHADER_PARAM_FTINTPOWER, tint_power);
        set_float(instance, SHADER_PARAM_FORIGINALPOWER, original_power);
    }
};

class ChannelBlurShader : public GLSLShader
{
public:
    int coeff;
    int r, g, b, a;

    ChannelBlurShader()
    : GLSLShader(SHADER_CHANNELBLUR)
    {
    }

    void initialize_parameters()
    {
        coeff = get_uniform("fCoeff");
        r = get_uniform("iR");
        g = get_uniform("iG");
        b = get_uniform("iB");
        a = get_uniform("iA");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_FCOEFF, coeff);
        set_int(instance, SHADER_PARAM_IR, r);
        set_int(instance, SHADER_PARAM_IG, g);
        set_int(instance, SHADER_PARAM_IB, b);
        set_int(instance, SHADER_PARAM_IA, a);
    }
};

class BgBloomShader : public GLSLShader
{
public:
    int radius;
    int exponent;
    int coeff;

    BgBloomShader()
    : GLSLShader(SHADER_BGBLOOM, SHADER_HAS_BACK | SHADER_HAS_TEX_SIZE)
    {
    }

    void initialize_parameters()
    {
        coeff = get_uniform("coeff");
        radius = get_uniform("radius");
        exponent = get_uniform("exponent");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_COEFF, coeff);
        set_float(instance, SHADER_PARAM_EXPONENT, exponent);
        set_float(instance, SHADER_PARAM_RADIUS, radius);
    }
};

class UnderwaterShader : public GLSLShader
{
public:
    int blur;
    int amplitudeX;
    int periodsX;
    int freqX;
    int amplitudeY;
    int periodsY;
    int freqY;

    UnderwaterShader()
    : GLSLShader(SHADER_UNDERWATER)
    {
    }

    void initialize_parameters()
    {
        blur = get_uniform("fBlur");
        amplitudeX = get_uniform("fAmplitudeX");
        periodsX = get_uniform("fPeriodsX");
        freqX = get_uniform("fFreqX");
        amplitudeY = get_uniform("fAmplitudeY");
        periodsY = get_uniform("fPeriodsY");
        freqY = get_uniform("fFreqY");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_FBLUR, blur);
        set_float(instance, SHADER_PARAM_FAMPLITUDEX, amplitudeX);
        set_float(instance, SHADER_PARAM_FPERIODSX, periodsX);
        set_float(instance, SHADER_PARAM_FFREQX, freqX);
        set_float(instance, SHADER_PARAM_FAMPLITUDEY, amplitudeY);
        set_float(instance, SHADER_PARAM_FPERIODSY, periodsY);
        set_float(instance, SHADER_PARAM_FFREQY, freqY);
    }
};

class RotateSubShader : public GLSLShader
{
public:
    int angle, x, y, shift_x, shift_y;

    RotateSubShader()
    : GLSLShader(SHADER_ROTATESUB)
    {
    }

    void initialize_parameters()
    {
        angle = get_uniform("fA");
        x = get_uniform("fX");
        y = get_uniform("fY");
        shift_x = get_uniform("fSx");
        shift_y = get_uniform("fSy");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_FA, angle);
        set_float(instance, SHADER_PARAM_FX, x);
        set_float(instance, SHADER_PARAM_FY, y);
        set_float(instance, SHADER_PARAM_FSX, shift_x);
        set_float(instance, SHADER_PARAM_FSY, shift_y);
    }
};

class SimpleMaskShader : public GLSLShader
{
public:
    int fade, color;

    SimpleMaskShader()
    : GLSLShader(SHADER_SIMPLEMASK)
    {
    }

    void initialize_parameters()
    {
        color = get_uniform("fC");
        fade = get_uniform("fFade");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_FFADE, fade);
        set_vec4(instance, SHADER_PARAM_FC, color);
    }
};

class OffsetStationaryShader : public GLSLShader
{
public:

    int width, height, x_offset, y_offset;

    OffsetStationaryShader()
    : GLSLShader(SHADER_OFFSETSTATIONARY)
    {
    }

    void initialize_parameters()
    {
        width = get_uniform("width");
        height = get_uniform("height");
        x_offset = get_uniform("xoff");
        y_offset = get_uniform("yoff");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_WIDTH, width);
        set_float(instance, SHADER_PARAM_HEIGHT, height);
        set_float(instance, SHADER_PARAM_XOFF, x_offset);
        set_float(instance, SHADER_PARAM_YOFF, y_offset);
    }
};

class PatternOverlayShader : public GLSLShader
{
public:
    int x, y, width, height;
    int alpha;

    PatternOverlayShader()
    : GLSLShader(SHADER_PATTERNOVERLAY, SHADER_HAS_TEX_SIZE, "pattern")
    {
    }

    void initialize_parameters()
    {
        x = get_uniform("x");
        y = get_uniform("y");
        width = get_uniform("width");
        height = get_uniform("height");
        alpha = get_uniform("alpha");
    }

    void set_parameters(FrameObject * instance)
    {
        set_image(instance, SHADER_PARAM_PATTERN);
        set_float(instance, SHADER_PARAM_WIDTH, width);
        set_float(instance, SHADER_PARAM_HEIGHT, height);
        set_float(instance, SHADER_PARAM_X, x);
        set_float(instance, SHADER_PARAM_Y, y);
        set_float(instance, SHADER_PARAM_ALPHA, alpha);
    }
};

class SubPxShader : public GLSLShader
{
public:
    int x, y, limit;

    SubPxShader()
    : GLSLShader(SHADER_SUBPX, SHADER_HAS_TEX_SIZE)
    {
    }

    void initialize_parameters()
    {
        x = get_uniform("x");
        y = get_uniform("y");
        limit = get_uniform("limit");
    }

    void set_parameters(FrameObject * instance)
    {
        set_int(instance, SHADER_PARAM_LIMIT, limit);
        set_float(instance, SHADER_PARAM_X, x);
        set_float(instance, SHADER_PARAM_Y, y);
    }
};

class ZoomOffsetShader : public GLSLShader
{
public:
    int x, y, width, height;
    int zoom_x, zoom_y;

    ZoomOffsetShader()
    : GLSLShader(SHADER_ZOOMOFFSET)
    {
    }

    void initialize_parameters()
    {
        x = get_uniform("fX");
        y = get_uniform("fY");
        width = get_uniform("fWidth");
        height = get_uniform("fHeight");
        zoom_x = get_uniform("fZoomX");
        zoom_y = get_uniform("fZoomY");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_FX, x);
        set_float(instance, SHADER_PARAM_FY, y);
        set_float(instance, SHADER_PARAM_FZOOMX, zoom_x);
        set_float(instance, SHADER_PARAM_FZOOMY, zoom_y);
        set_float(instance, SHADER_PARAM_FWIDTH, width);
        set_float(instance, SHADER_PARAM_FHEIGHT, height);
    }
};

class GradientShader : public GLSLShader
{
public:
    int a_rgb, b_rgb;
    int a_a, b_a;
    int coeff, offset, fade;
    int t, f, r, mask;

    GradientShader()
    : GLSLShader(SHADER_GRADIENT)
    {
    }

    void initialize_parameters()
    {
        a_rgb = get_uniform("fArgb");
        a_a = get_uniform("fAa");
        b_rgb = get_uniform("fBrgb");
        b_a = get_uniform("fBa");
        coeff = get_uniform("fCoeff");
        offset = get_uniform("fOffset");
        fade = get_uniform("fFade");
        t = get_uniform("iT");
        f = get_uniform("iF");
        r = get_uniform("iR");
        mask = get_uniform("iMask");
    }

    void set_parameters(FrameObject * instance)
    {
        set_vec4(instance, SHADER_PARAM_FARGB, a_rgb);
        set_float(instance, SHADER_PARAM_FAA, a_a);
        set_vec4(instance, SHADER_PARAM_FBRGB, b_rgb);
        set_float(instance, SHADER_PARAM_FBA, b_a);
        set_float(instance, SHADER_PARAM_FCOEFF, coeff);
        set_float(instance, SHADER_PARAM_FOFFSET, offset);
        set_float(instance, SHADER_PARAM_FFADE, fade);
        set_int(instance, SHADER_PARAM_IT, t);
        set_int(instance, SHADER_PARAM_IF, f);
        set_int(instance, SHADER_PARAM_IR, r);
        set_int(instance, SHADER_PARAM_IMASK, mask);
    }
};

class OverlayAlphaShader : public GLSLShader
{
public:
    int alpha;

    OverlayAlphaShader()
    : GLSLShader(SHADER_OVERLAYALPHA, SHADER_HAS_BACK)
    {
    }

    void initialize_parameters()
    {
        alpha = get_uniform("bgA");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_BGA, alpha);
    }
};

class LensShader : public GLSLShader
{
public:
    int coeff, base;

    LensShader()
    : GLSLShader(SHADER_LENS, SHADER_HAS_BACK)
    {
    }

    void initialize_parameters()
    {
        coeff = get_uniform("fCoeff");
        base = get_uniform("fBase");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_FCOEFF, coeff);
        set_float(instance, SHADER_PARAM_FBASE, base);
    }
};

class ColDirBlurShader : public GLSLShader
{
public:
    int rr, rg, rb, gr, gg, gb, br, bg, bb;
    int angle, coeff;

    ColDirBlurShader()
    : GLSLShader(SHADER_COLDIRBLUR, SHADER_HAS_BACK)
    {
    }

    void initialize_parameters()
    {
        rr = get_uniform("rr");
        rg = get_uniform("rg");
        rb = get_uniform("rb");
        gr = get_uniform("gr");
        gg = get_uniform("gg");
        gb = get_uniform("gb");
        br = get_uniform("br");
        bg = get_uniform("bg");
        bb = get_uniform("bb");
        angle = get_uniform("fAngle");
        coeff = get_uniform("fCoeff");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_RR, rr);
        set_float(instance, SHADER_PARAM_RG, rg);
        set_float(instance, SHADER_PARAM_RB, rb);
        set_float(instance, SHADER_PARAM_GR, gr);
        set_float(instance, SHADER_PARAM_GG, gg);
        set_float(instance, SHADER_PARAM_GB, gb);
        set_float(instance, SHADER_PARAM_BR, br);
        set_float(instance, SHADER_PARAM_BG, bg);
        set_float(instance, SHADER_PARAM_BB, bb);
        set_float(instance, SHADER_PARAM_FANGLE, angle);
        set_float(instance, SHADER_PARAM_FCOEFF, coeff);
    }
};

class PerspectiveShader : public GLSLShader
{
public:
    int effect;
    int direction, perspective_dir;
    int zoom, offset;
    int sine_waves;

    PerspectiveShader()
    : GLSLShader(SHADER_PERSPECTIVE, SHADER_HAS_TEX_SIZE)
    {
    }

    void initialize_parameters()
    {
        effect = get_uniform("effect");
        direction = get_uniform("direction");
        // perspective_dir = get_uniform("perspective_dir");
        zoom = get_uniform("zoom");
        offset = get_uniform("offset");
        sine_waves = get_uniform("sine_waves");
    }

    void set_parameters(FrameObject * instance)
    {
        set_int(instance, SHADER_PARAM_EFFECT, effect);
        set_int(instance, SHADER_PARAM_DIRECTION, direction);
        // set_int(instance, SHADER_PARAM_PERSPECTIVE_DIR, perspective_dir);
        set_float(instance, SHADER_PARAM_ZOOM, zoom);
        set_float(instance, SHADER_PARAM_OFFSET, offset);
        set_int(instance, SHADER_PARAM_SINE_WAVES, sine_waves);
    }
};

class NinePatchShader : public GLSLShader
{
public:
    int x_scale, y_scale;
    int color_1, alpha_1, color_2, alpha_2;
    int coeff, offset, fade;

    NinePatchShader()
    : GLSLShader(SHADER_9G, SHADER_HAS_TEX_SIZE)
    {
    }

    void initialize_parameters()
    {
        x_scale = get_uniform("x_scale");
        y_scale = get_uniform("y_scale");
        color_1 = get_uniform("color_1");
        alpha_1 = get_uniform("alpha_1");
        color_2 = get_uniform("color_2");
        alpha_2 = get_uniform("alpha_2");
        offset = get_uniform("offset");
        // coeff = get_uniform("coeff");
        // fade = get_uniform("fade");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_XSCALE, x_scale);
        set_float(instance, SHADER_PARAM_YSCALE, y_scale);
        set_vec4(instance, SHADER_PARAM_FARGB, color_1);
        set_float(instance, SHADER_PARAM_FAA, alpha_1);
        set_vec4(instance, SHADER_PARAM_FBRGB, color_2);
        set_float(instance, SHADER_PARAM_FBA, alpha_2);
        set_float(instance, SHADER_PARAM_FOFFSET, offset);
        // set_float(instance, SHADER_PARAM_FCOEFF, coeff);
        // set_float(instance, SHADER_PARAM_FFADE, fade);
    }
};

class PixelOutlineShader : public GLSLShader
{
public:
    int color;

    PixelOutlineShader()
    : GLSLShader(SHADER_PIXELOUTLINE, SHADER_HAS_TEX_SIZE)
    {
    }

    void initialize_parameters()
    {
        color = get_uniform("color");
    }

    void set_parameters(FrameObject * instance)
    {
        set_vec4(instance, SHADER_PARAM_COLOR, color);
    }
};

class BrightSatBgShader : public GLSLShader
{
public:
    int brightness, saturation;

    BrightSatBgShader()
    : GLSLShader(SHADER_BRIGHTSATBG, SHADER_HAS_BACK)
    {
    }

    void initialize_parameters()
    {
        brightness = get_uniform("brightness");
        saturation = get_uniform("saturation");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_BRIGHTNESS, brightness);
        set_float(instance, SHADER_PARAM_SATURATION, saturation);
    }
};

class BgBlurShader : public GLSLShader
{
public:
    int x, y, alpha;

    BgBlurShader()
    : GLSLShader(SHADER_BGBLUR, SHADER_HAS_BACK | SHADER_HAS_TEX_SIZE)
    {
    }

    void initialize_parameters()
    {
        x = get_uniform("x");
        y = get_uniform("y");
        alpha = get_uniform("alpha");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, SHADER_PARAM_FX, x);
        set_float(instance, SHADER_PARAM_FY, y);
        set_float(instance, SHADER_PARAM_FA, alpha);
    }
};

#ifndef CHOWDREN_USE_GL
class BasicShader : public GLSLShader
{
public:
    BasicShader()
    : GLSLShader(SHADER_BASIC, false)
    {
    }
};

class TextureShader : public GLSLShader
{
public:
    TextureShader()
    : GLSLShader(SHADER_TEXTURE, false)
    {
    }
};
#endif

// XXX make these static variables
Shader * subtract_shader;
Shader * additive_shader;
Shader * monochrome_shader;
Shader * mixer_shader;
Shader * hue_shader;
Shader * dummy_shader;
Shader * blend_shader;
Shader * offset_shader;
Shader * dodgeblur_shader;
Shader * invert_shader;
Shader * grain_shader;
Shader * multiply_shader;
Shader * hardlight_shader;
Shader * tint_shader;
Shader * channelblur_shader;
Shader * bgbloom_shader;
Shader * underwater_shader;
Shader * rotatesub_shader;
Shader * simplemask_shader;
Shader * offsetstationary_shader;
Shader * patternoverlay_shader;
Shader * subpx_shader;
Shader * zoomoffset_shader;
Shader * gradient_shader;
Shader * overlayalpha_shader;
Shader * lens_shader;
Shader * coldirblur_shader;
Shader * ninepatch_shader;
Shader * pixeloutline_shader;
Shader * brightsatbg_shader;
Shader * bgblur_shader;

// extension shaders
Shader * perspective_shader;

// system shaders
#ifndef CHOWDREN_USE_GL
Shader * basic_shader;
Shader * texture_shader;
#endif

void init_shaders()
{
#ifndef CHOWDREN_USE_GL
    basic_shader = new BasicShader;
    texture_shader = new TextureShader;
#endif
    subtract_shader = new SubtractShader;
    additive_shader = new AdditiveShader;
    monochrome_shader = new MonochromeShader;
    mixer_shader = new MixerShader;
    hue_shader = new HueShader;
    dummy_shader = new Shader;
    blend_shader = new Shader;
    offset_shader = new OffsetShader;
    dodgeblur_shader = new DodgeBlurShader;
    invert_shader = new InvertShader;
    grain_shader = new GrainShader;
    multiply_shader = new MultiplyShader;
    hardlight_shader = new HardLightShader;
    tint_shader = new TintShader;
    channelblur_shader = new ChannelBlurShader;
    bgbloom_shader = new BgBloomShader;
    underwater_shader = new UnderwaterShader;
    rotatesub_shader = new RotateSubShader;
    simplemask_shader = new SimpleMaskShader;
    offsetstationary_shader = new OffsetStationaryShader;
    patternoverlay_shader = new PatternOverlayShader;
    subpx_shader = new SubPxShader;
    zoomoffset_shader = new ZoomOffsetShader;
    gradient_shader = new GradientShader;
    overlayalpha_shader = new OverlayAlphaShader;
    lens_shader = new LensShader;
    coldirblur_shader = new ColDirBlurShader;
    perspective_shader = new PerspectiveShader;
    ninepatch_shader = new NinePatchShader;
    pixeloutline_shader = new PixelOutlineShader;
    brightsatbg_shader = new BrightSatBgShader;
    bgblur_shader = new BgBlurShader;
}
