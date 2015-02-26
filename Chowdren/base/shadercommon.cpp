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

class AdditiveShader : public BaseShader
{
public:
    AdditiveShader()
    : BaseShader()

    void begin(FrameObject * instance, int width, int height)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }

    void end(FrameObject * instance)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
};

void BaseShader::set_image(FrameObject * instance, int src)
{
}

// shader implementations

class SubtractShader : public BaseShader
{
public:
    SubtractShader()
    : BaseShader(SHADER_SUBTRACT)
    {
    }

    void begin(FrameObject * instance, int width, int height)
    {
        // MMF2 default
        // glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
        glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
        glBlendFunc(GL_DST_COLOR, GL_ONE);
        BaseShader::begin(instance, width, height);
    }

    void end(FrameObject * instance)
    {
        // MMF2 default
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        BaseShader::end(instance);
    }
};

class MonochromeShader : public BaseShader
{
public:
    MonochromeShader()
    : BaseShader(SHADER_MONOCHROME)
    {
    }
};

class MixerShader : public BaseShader
{
public:
    int r;
    int g;
    int b;

    MixerShader()
    : BaseShader(SHADER_COLORMIXER)
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

class HueShader : public BaseShader
{
public:
    int hue;

    HueShader()
    : BaseShader(SHADER_HUE)
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

class OffsetShader : public BaseShader
{
public:
    int width;
    int height;

    OffsetShader()
    : BaseShader(SHADER_OFFSET, SHADER_HAS_BACK | SHADER_HAS_TEX_SIZE)
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

class InvertShader : public BaseShader
{
public:
    InvertShader()
    : BaseShader(SHADER_INVERT)
    {

    }
};

class DodgeBlurShader : public BaseShader
{
public:
    int vertical;
    int radius;

    DodgeBlurShader()
    : BaseShader(SHADER_DODGEBLUR, SHADER_HAS_BACK | SHADER_HAS_TEX_SIZE)
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

class GrainShader : public BaseShader
{
public:
    int strength, seed;
    int invert;
    int r, g, b, a;

    GrainShader()
    : BaseShader(SHADER_GRAIN)
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

class MultiplyShader : public BaseShader
{
public:
    MultiplyShader()
    : BaseShader(SHADER_MULTIPLY, SHADER_HAS_BACK)
    {
    }
};

class HardLightShader : public BaseShader
{
public:
    HardLightShader()
    : BaseShader(SHADER_HARDLIGHT, SHADER_HAS_BACK)
    {
    }
};

class TintShader : public BaseShader
{
public:
    int tint_color;
    int tint_power;
    int original_power;

    TintShader()
    : BaseShader(SHADER_TINT)
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

class ChannelBlurShader : public BaseShader
{
public:
    int coeff;
    int r, g, b, a;

    ChannelBlurShader()
    : BaseShader(SHADER_CHANNELBLUR)
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

class BgBloomShader : public BaseShader
{
public:
    int radius;
    int exponent;
    int coeff;

    BgBloomShader()
    : BaseShader(SHADER_BGBLOOM, SHADER_HAS_BACK | SHADER_HAS_TEX_SIZE)
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

class UnderwaterShader : public BaseShader
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
    : BaseShader(SHADER_UNDERWATER)
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

class RotateSubShader : public BaseShader
{
public:
    int angle, x, y, shift_x, shift_y;

    RotateSubShader()
    : BaseShader(SHADER_ROTATESUB)
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

class SimpleMaskShader : public BaseShader
{
public:
    int fade, color;

    SimpleMaskShader()
    : BaseShader(SHADER_SIMPLEMASK)
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

class OffsetStationaryShader : public BaseShader
{
public:

    int width, height, x_offset, y_offset;

    OffsetStationaryShader()
    : BaseShader(SHADER_OFFSETSTATIONARY)
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

class PatternOverlayShader : public BaseShader
{
public:
    int x, y, width, height;
    int alpha;

    PatternOverlayShader()
    : BaseShader(SHADER_PATTERNOVERLAY, SHADER_HAS_TEX_SIZE, "pattern")
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

class SubPxShader : public BaseShader
{
public:
    int x, y, limit;

    SubPxShader()
    : BaseShader(SHADER_SUBPX, SHADER_HAS_TEX_SIZE)
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

class ZoomOffsetShader : public BaseShader
{
public:
    int x, y, width, height;
    int zoom_x, zoom_y;

    ZoomOffsetShader()
    : BaseShader(SHADER_ZOOMOFFSET)
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

class GradientShader : public BaseShader
{
public:
    int a_rgb, b_rgb;
    int a_a, b_a;
    int coeff, offset, fade;
    int t, f, r, mask;

    GradientShader()
    : BaseShader(SHADER_GRADIENT)
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

class OverlayAlphaShader : public BaseShader
{
public:
    int alpha;

    OverlayAlphaShader()
    : BaseShader(SHADER_OVERLAYALPHA, SHADER_HAS_BACK)
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

class LensShader : public BaseShader
{
public:
    int coeff, base;

    LensShader()
    : BaseShader(SHADER_LENS, SHADER_HAS_BACK)
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

class ColDirBlurShader : public BaseShader
{
public:
    int rr, rg, rb, gr, gg, gb, br, bg, bb;
    int angle, coeff;

    ColDirBlurShader()
    : BaseShader(SHADER_COLDIRBLUR, SHADER_HAS_BACK)
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

class PerspectiveShader : public BaseShader
{
public:
    int effect;
    int direction, perspective_dir;
    int zoom, offset;
    int sine_waves;

    PerspectiveShader()
    : BaseShader(SHADER_PERSPECTIVE, SHADER_HAS_TEX_SIZE)
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

class NinePatchShader : public BaseShader
{
public:
    int x_scale, y_scale;
    int color_1, alpha_1, color_2, alpha_2;
    int coeff, offset, fade;

    NinePatchShader()
    : BaseShader(SHADER_9G, SHADER_HAS_TEX_SIZE)
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

class PixelOutlineShader : public BaseShader
{
public:
    int color;

    PixelOutlineShader()
    : BaseShader(SHADER_PIXELOUTLINE, SHADER_HAS_TEX_SIZE)
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

class BrightSatBgShader : public BaseShader
{
public:
    int brightness, saturation;

    BrightSatBgShader()
    : BaseShader(SHADER_BRIGHTSATBG, SHADER_HAS_BACK)
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

class BgBlurShader : public BaseShader
{
public:
    int x, y, alpha;

    BgBlurShader()
    : BaseShader(SHADER_BGBLUR, SHADER_HAS_BACK | SHADER_HAS_TEX_SIZE)
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
class BasicShader : public BaseShader
{
public:
    BasicShader()
    : BaseShader(SHADER_BASIC, false)
    {
    }
};

class TextureShader : public BaseShader
{
public:
    TextureShader()
    : BaseShader(SHADER_TEXTURE, false)
    {
    }
};
#endif

SubtractShader subtract_shader;
AdditiveShader additive_shader;
MonochromeShader monochrome_shader;
MixerShader mixer_shader;
HueShader hue_shader;
OffsetShader offset_shader;
DodgeBlurShader dodgeblur_shader;
InvertShader invert_shader;
GrainShader grain_shader;
MultiplyShader multiply_shader;
HardLightShader hardlight_shader;
TintShader tint_shader;
ChannelBlurShader channelblur_shader;
BgBloomShader bgbloom_shader;
UnderwaterShader underwater_shader;
RotateSubShader rotatesub_shader;
SimpleMaskShader simplemask_shader;
OffsetStationaryShader offsetstationary_shader;
PatternOverlayShader patternoverlay_shader;
SubPxShader subpx_shader;
ZoomOffsetShader zoomoffset_shader;
GradientShader gradient_shader;
OverlayAlphaShader overlayalpha_shader;
LensShader lens_shader;
ColDirBlurShader coldirblur_shader;
NinePatchShader ninepatch_shader;
PixelOutlineShader pixeloutline_shader;
BrightSatBgShader brightsatbg_shader;
BgBlurShader bgblur_shader;

// extension shaders
PerspectiveShader perspective_shader;

// system shaders
BasicShader basic_shader;
TextureShader texture_shader;

void init_shaders()
{
}
