#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#else
#include <sys/time.h>
#endif

#include <SDL.h>
#include "chowconfig.h"
#include "platform.h"
#include "include_gl.h"
#include "manager.h"
#include "mathcommon.h"
#include "fbo.h"
#include <iostream>

static Framebuffer screen_fbo;
static SDL_Window * global_window = NULL;
static SDL_GLContext global_context = NULL;
static bool is_fullscreen = false;
static bool hide_cursor = false;
static bool has_closed = false;
static Uint64 start_time;

static int draw_x_size = 0;
static int draw_y_size = 0;
static int draw_x_off = 0;
static int draw_y_off = 0;

#ifdef CHOWDREN_USE_GL
// opengl function pointers
PFNGLBLENDEQUATIONSEPARATEEXTPROC glBlendEquationSeparateEXT;
PFNGLBLENDEQUATIONEXTPROC glBlendEquationEXT;
PFNGLBLENDFUNCSEPARATEEXTPROC glBlendFuncSeparateEXT;
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;

PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLDETACHSHADERPROC glDetachShader;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
#endif

static bool check_opengl_extension(const char * name)
{
    if (SDL_GL_ExtensionSupported(name) == SDL_TRUE)
        return true;
    std::string message;
    message += "OpenGL extension '";
    message += name;
    message += "' not supported.";
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "OpenGL error",
                             message.c_str(), NULL);
    return false;
}

const char * extensions[] = {
#ifdef CHOWDREN_USE_GL
    "GL_EXT_framebuffer_object",
    "GL_ARB_vertex_shader",
    "GL_ARB_fragment_shader",
    "GL_ARB_texture_non_power_of_two",
#endif
    NULL
};

static bool check_opengl_extensions()
{
    for (int i = 0; extensions[i] != NULL; i++)
        if (!check_opengl_extension(extensions[i]))
            return false;
    return true;
}

static void on_key(SDL_KeyboardEvent & e)
{
    if (e.repeat != 0)
        return;
    global_manager->on_key(e.keysym.sym, e.state == SDL_PRESSED);
}

static void on_mouse(SDL_MouseButtonEvent & e)
{
    global_manager->on_mouse(e.button, e.state == SDL_PRESSED);
}

void init_joystick();
void update_joystick();
void add_joystick(int device);
void remove_joystick(int instance);
void on_joystick_button(int instance, int button, bool state);

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <sys/param.h> // For MAXPATHLEN

static void set_resources_dir()
{
    char resourcesPath[MAXPATHLEN];

    CFBundleRef bundle = CFBundleGetMainBundle();
    if (!bundle)
        return;

    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(bundle);

    CFStringRef last = CFURLCopyLastPathComponent(resourcesURL);
    if (CFStringCompare(CFSTR("Resources"), last, 0) != kCFCompareEqualTo)
    {
        CFRelease(last);
        CFRelease(resourcesURL);
        return;
    }

    CFRelease(last);

    if (!CFURLGetFileSystemRepresentation(resourcesURL,
                                          true,
                                          (UInt8*) resourcesPath,
                                          MAXPATHLEN))
    {
        CFRelease(resourcesURL);
        return;
    }

    CFRelease(resourcesURL);

    chdir(resourcesPath);
}
#endif

void platform_init()
{
    unsigned int flags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
                         SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC |
                         SDL_INIT_NOPARACHUTE;
    if (SDL_Init(flags) < 0) {
        std::cout << "SDL could not be initialized: " << SDL_GetError()
            << std::endl;
        return;
    }
#ifdef __APPLE__
    set_resources_dir();
#endif
    start_time = SDL_GetPerformanceCounter();
    init_joystick();
}

void platform_exit()
{
    SDL_Quit();
}

void platform_poll_events()
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                on_key(e.key);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                on_mouse(e.button);
                break;
            case SDL_CONTROLLERDEVICEADDED:
                add_joystick(e.cdevice.which);
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                remove_joystick(e.cdevice.which);
                break;
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                on_joystick_button(e.cbutton.which, e.cbutton.button,
                                   e.cbutton.state == SDL_PRESSED);
                break;
            case SDL_QUIT:
                has_closed = true;
                break;
            default:
                break;
        }
    }
    update_joystick();
}

// time

double platform_get_time()
{
    Uint64 s = SDL_GetPerformanceCounter();
    s -= start_time;
    return s / double(SDL_GetPerformanceFrequency());
}

void platform_sleep(double t)
{
    if (t < 0.001)
        return;
    SDL_Delay(t * 1000.0);
}

bool platform_display_closed()
{
    if (global_window == NULL)
        return true;
    return has_closed;
}

void platform_get_mouse_pos(int * x, int * y)
{
    SDL_GetMouseState(x, y);
    *x = (*x - draw_x_off) * (float(WINDOW_WIDTH) / draw_x_size);
    *y = (*y - draw_y_off) * (float(WINDOW_HEIGHT) / draw_y_size);
}

void platform_create_display(bool fullscreen)
{
    is_fullscreen = fullscreen;

#ifdef CHOWDREN_USE_GL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#elif CHOWDREN_USE_GLES1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#elif CHOWDREN_USE_GLES2
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    global_window = SDL_CreateWindow(NAME,
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     WINDOW_WIDTH, WINDOW_HEIGHT,
                                     flags);
    if (global_window == NULL) {
        std::cout << "Could not open window: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
        return;
    }
    global_context = SDL_GL_CreateContext(global_window);
    if (global_context == NULL) {
        std::cout << "Could not create OpenGL context: " << SDL_GetError()
            << std::endl;
        exit(EXIT_FAILURE);
        return;
    }

    const GLubyte * renderer = glGetString(GL_RENDERER);
    const GLubyte * vendor = glGetString(GL_VENDOR);

    std::cout << "Renderer: " << renderer << " - " << vendor << " - "
        << std::endl;

#ifdef CHOWDREN_USE_GL
    // initialize OpenGL function pointers
    glBlendEquationSeparateEXT =
        (PFNGLBLENDEQUATIONSEPARATEEXTPROC)
        SDL_GL_GetProcAddress("glBlendEquationSeparateEXT");
    glBlendEquationEXT =
        (PFNGLBLENDEQUATIONEXTPROC)
        SDL_GL_GetProcAddress("glBlendEquationEXT");
    glBlendFuncSeparateEXT =
        (PFNGLBLENDFUNCSEPARATEEXTPROC)
        SDL_GL_GetProcAddress("glBlendFuncSeparateEXT");

    glActiveTextureARB =
        (PFNGLACTIVETEXTUREARBPROC)
        SDL_GL_GetProcAddress("glActiveTextureARB");
    glMultiTexCoord2fARB =
        (PFNGLMULTITEXCOORD2FARBPROC)
        SDL_GL_GetProcAddress("glMultiTexCoord2fARB");

    glGenFramebuffersEXT =
        (PFNGLGENFRAMEBUFFERSEXTPROC)
        SDL_GL_GetProcAddress("glGenFramebuffersEXT");
    glFramebufferTexture2DEXT =
        (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)
        SDL_GL_GetProcAddress("glFramebufferTexture2DEXT");
    glBindFramebufferEXT =
        (PFNGLBINDFRAMEBUFFEREXTPROC)
        SDL_GL_GetProcAddress("glBindFramebufferEXT");

    // shaders
    glUniform1i =
        (PFNGLUNIFORM1IPROC)
        SDL_GL_GetProcAddress("glUniform1i");
    glUseProgram =
        (PFNGLUSEPROGRAMPROC)
        SDL_GL_GetProcAddress("glUseProgram");
    glDetachShader =
        (PFNGLDETACHSHADERPROC)
        SDL_GL_GetProcAddress("glDetachShader");
    glGetProgramInfoLog =
        (PFNGLGETPROGRAMINFOLOGPROC)
        SDL_GL_GetProcAddress("glGetProgramInfoLog");
    glGetProgramiv =
        (PFNGLGETPROGRAMIVPROC)
        SDL_GL_GetProcAddress("glGetProgramiv");
    glLinkProgram =
        (PFNGLLINKPROGRAMPROC)
        SDL_GL_GetProcAddress("glLinkProgram");
    glCreateProgram =
        (PFNGLCREATEPROGRAMPROC)
        SDL_GL_GetProcAddress("glCreateProgram");
    glAttachShader =
        (PFNGLATTACHSHADERPROC)
        SDL_GL_GetProcAddress("glAttachShader");
    glGetShaderInfoLog =
        (PFNGLGETSHADERINFOLOGPROC)
        SDL_GL_GetProcAddress("glGetShaderInfoLog");
    glGetShaderiv =
        (PFNGLGETSHADERIVPROC)
        SDL_GL_GetProcAddress("glGetShaderiv");
    glCompileShader =
        (PFNGLCOMPILESHADERPROC)
        SDL_GL_GetProcAddress("glCompileShader");
    glShaderSource =
        (PFNGLSHADERSOURCEPROC)
        SDL_GL_GetProcAddress("glShaderSource");
    glCreateShader =
        (PFNGLCREATESHADERPROC)
        SDL_GL_GetProcAddress("glCreateShader");
    glUniform2f =
        (PFNGLUNIFORM2FPROC)
        SDL_GL_GetProcAddress("glUniform2f");
    glUniform1f =
        (PFNGLUNIFORM1FPROC)
        SDL_GL_GetProcAddress("glUniform1f");
    glUniform4f =
        (PFNGLUNIFORM4FPROC)
        SDL_GL_GetProcAddress("glUniform4f");
    glGetUniformLocation =
        (PFNGLGETUNIFORMLOCATIONPROC)
        SDL_GL_GetProcAddress("glGetUniformLocation");
#endif

    // check extensions
    if (!check_opengl_extensions()) {
        std::cout << "Not all OpenGL extensions supported. Quitting..."
            << std::endl;
        exit(EXIT_FAILURE);
        return;
    }

    // if the cursor was hidden before the window was created, hide it now
    if (hide_cursor)
        platform_hide_mouse();

    screen_fbo.init(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void platform_set_vsync(bool value)
{
    if (global_window == NULL)
        return;

    int vsync;
    if (value)
        vsync = 1;
    else
        vsync = 0;

    int ret = SDL_GL_SetSwapInterval(vsync);
    if (ret == 0)
        return;

    std::cout << "Set vsync failed: " << SDL_GetError() << std::endl;
}

void platform_set_fullscreen(bool value)
{
    int flags;
    if (value)
        flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
    else
        flags = 0;
    SDL_SetWindowFullscreen(global_window, flags);
}

void platform_begin_draw()
{
    screen_fbo.bind();
}

void platform_swap_buffers()
{
    int window_width, window_height;
    platform_get_size(&window_width, &window_height);
    bool resize = window_width != WINDOW_WIDTH ||
                  window_height != WINDOW_HEIGHT;

    if (resize) {
        // aspect-aware resize
        float aspect_width = window_width / float(WINDOW_WIDTH);
        float aspect_height = window_height / float(WINDOW_HEIGHT);

        float aspect = std::min(aspect_width, aspect_height);

#ifdef CHOWDREN_QUICK_SCALE
        aspect = std::max(std::min(1.0f, aspect), float(floor(aspect)));
#endif
        draw_x_size = aspect * WINDOW_WIDTH;
        draw_y_size = aspect * WINDOW_HEIGHT;

        draw_x_off = (window_width - draw_x_size) / 2;
        draw_y_off = (window_height - draw_y_size) / 2;
    } else {
        draw_x_off = draw_y_off = 0;
        draw_x_size = WINDOW_WIDTH;
        draw_y_size = WINDOW_HEIGHT;
    }

    screen_fbo.unbind();

    // resize the window contents if necessary (fullscreen mode)

    glViewport(0, 0, window_width, window_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, window_width, window_height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    int x2 = draw_x_off + draw_x_size;
    int y2 = draw_y_off + draw_y_size;

    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, screen_fbo.get_tex());
    glDisable(GL_BLEND);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0);
    glVertex2i(draw_x_off, draw_y_off);
    glTexCoord2f(1.0, 1.0);
    glVertex2i(x2, draw_y_off);
    glTexCoord2f(1.0, 0.0);
    glVertex2i(x2, y2);
    glTexCoord2f(0.0, 0.0);
    glVertex2i(draw_x_off, y2);
    glEnd();
    glEnable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    SDL_GL_SwapWindow(global_window);
}

void platform_get_size(int * width, int * height)
{
    SDL_GL_GetDrawableSize(global_window, width, height);
}

void platform_get_screen_size(int * width, int * height)
{
    int display_index;
    display_index = SDL_GetWindowDisplayIndex(global_window);
    SDL_Rect bounds;
    SDL_GetDisplayBounds(display_index, &bounds);
    *width = bounds.w;
    *height = bounds.h;
}

bool platform_has_focus()
{
    int f = SDL_GetWindowFlags(global_window);
    if ((f & SDL_WINDOW_SHOWN) == 0)
        return false;
    if ((f & SDL_WINDOW_INPUT_FOCUS) == 0)
        return false;
    return true;
}

void platform_set_focus(bool value)
{
    if (value)
        SDL_RestoreWindow(global_window);
    else
        SDL_MinimizeWindow(global_window);
}

void platform_show_mouse()
{
    hide_cursor = false;
    SDL_ShowCursor(1);
}

void platform_hide_mouse()
{
    hide_cursor = true;
    SDL_ShowCursor(0);
}

const std::string & platform_get_language()
{
    static std::string language("English");
    return language;
}

// filesystem stuff

#include <sys/stat.h>
#include <boost/filesystem.hpp>

size_t platform_get_file_size(const char * filename)
{
    boost::system::error_code err;
    uintmax_t ret = boost::filesystem::file_size(filename, err);
    if (ret == uintmax_t(-1))
        return 0;
    return ret;
}

bool platform_path_exists(const std::string & value)
{
    boost::system::error_code err;
    return boost::filesystem::exists(value, err);
}

bool platform_is_directory(const std::string & value)
{
    boost::system::error_code err;
    return boost::filesystem::is_directory(value, err);
}

bool platform_is_file(const std::string & value)
{
    boost::system::error_code err;
    return boost::filesystem::is_regular_file(value, err);
}

void platform_create_directories(const std::string & value)
{
    boost::filesystem::path path(value);
    if (path.has_filename())
        path.remove_filename();

    boost::system::error_code err;
    boost::filesystem::create_directories(path, err);
}

#ifdef _WIN32
#include "windows.h"
#include "shlobj.h"
#elif __APPLE__
#include <CoreServices/CoreServices.h>
#include <limits.h>
#elif __linux
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

const std::string & platform_get_appdata_dir()
{
    static bool initialized = false;
    static std::string dir;
    if (!initialized) {
#ifdef _WIN32
        TCHAR path[MAX_PATH];
        SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path);
        dir = path;
#elif __APPLE__
        FSRef ref;
        OSType folderType = kApplicationSupportFolderType;
        char path[PATH_MAX];
        FSFindFolder(kUserDomain, folderType, kCreateFolder, &ref);
        FSRefMakePath(&ref, (UInt8*)&path, PATH_MAX);
        dir = path;
#elif __linux
        struct passwd *pw = getpwuid(getuid());
        dir = pw->pw_dir;
#endif
    }
    return dir;
}

// joystick

class JoystickData;
static vector<JoystickData> joysticks;
static SDL_HapticEffect rumble_effect;

class JoystickData
{
public:
    SDL_Joystick * joy;
    SDL_GameController * controller;
    int device;
    int instance;
    SDL_Haptic * haptics;
    bool has_effect, has_rumble;
    const unsigned char * buttons;
    int axis_count;
    const float * axes;
    int last_press;

    JoystickData()
    : has_effect(false), has_rumble(false), last_press(0), controller(NULL),
      haptics(NULL)
    {
    }

    void init(int device, SDL_GameController * c)
    {
        controller = c;
        this->device = device;
        joy = SDL_GameControllerGetJoystick(c);
        instance = SDL_JoystickInstanceID(joy);
        init_rumble();
    }

    ~JoystickData()
    {
        if (controller != NULL)
            SDL_GameControllerClose(controller);
        if (haptics != NULL)
            SDL_HapticClose(haptics);
    }

    void init_rumble()
    {
        if (SDL_JoystickIsHaptic(joy) != 1)
            return;
        haptics = SDL_HapticOpenFromJoystick(joy);
        if (haptics == NULL)
            return;
        int efx = SDL_HapticEffectSupported(haptics, &rumble_effect);
        if (efx == 1) {
            rumble_effect.leftright.large_magnitude = 0;
            rumble_effect.leftright.small_magnitude = 0;
            SDL_HapticNewEffect(haptics, &rumble_effect);
            has_effect = true;
        } else if (SDL_HapticRumbleSupported(haptics) == 1) {
            SDL_HapticRumbleInit(haptics);
            has_rumble = true;
        }
    }

    float get_axis(SDL_GameControllerAxis n)
    {
        return float(SDL_GameControllerGetAxis(controller, n)) / float(0x7FFF);
    }

    float get_axis(int n)
    {
        return get_axis((SDL_GameControllerAxis)n);
    }

    bool get_button(int b)
    {
        return get_button((SDL_GameControllerButton)b);
    }

    bool get_button(SDL_GameControllerButton b)
    {
        return SDL_GameControllerGetButton(controller, b) == 1;
    }

    void vibrate(float left, float right, int ms)
    {
        if (!has_rumble && !has_effect)
            return;

        if (has_effect) {
            int lm = (int)(65535.0f * left);
            int sm = (int)(65535.0f * right);
            rumble_effect.leftright.large_magnitude = lm;
            rumble_effect.leftright.small_magnitude = sm;
            rumble_effect.leftright.length = ms;
            SDL_HapticUpdateEffect(haptics, 0, &rumble_effect);
            SDL_HapticRunEffect(haptics, 0, 1);
        } else {
            float strength;
            if (left >= right) {
                strength = left;
            } else {
                strength = right;
            }
            SDL_HapticRumblePlay(haptics, strength, ms);
        }
    }
};

JoystickData & get_joy(int n)
{
    return joysticks[n-1];
}

JoystickData * get_joy_instance(int instance)
{
    vector<JoystickData>::iterator it;
    for (it = joysticks.begin(); it != joysticks.end(); it++) {
        JoystickData & j = *it;
        if (j.instance != instance)
            continue;
        return &j;
    }
    return NULL;
}

void add_joystick(int device)
{
    if (!SDL_IsGameController(device))
        return;
    vector<JoystickData>::iterator it;
    for (it = joysticks.begin(); it != joysticks.end(); it++) {
        JoystickData & j = *it;
        if (j.device == device)
            return;
    }
    SDL_GameController * c = SDL_GameControllerOpen(device);
    if (c == NULL)
        return;
    int index = joysticks.size();
    joysticks.resize(index+1);
    joysticks[index].init(device, c);
}

void remove_joystick(int instance)
{
    vector<JoystickData>::iterator it;
    for (it = joysticks.begin(); it != joysticks.end(); it++) {
        JoystickData & j = *it;
        if (j.instance != instance)
            continue;
        joysticks.erase(it);
        break;
    }
}

void init_joystick()
{
    rumble_effect.type = SDL_HAPTIC_LEFTRIGHT;
    rumble_effect.leftright.length = 0;

    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        add_joystick(i);
    }
}

void update_joystick()
{
}

void on_joystick_button(int instance, int button, bool state)
{
    if (!state)
        return;
    get_joy_instance(instance)->last_press = button;
}

int get_joystick_last_press(int n)
{
    if (!is_joystick_attached(n))
        return CHOWDREN_BUTTON_INVALID;
    return remap_button(get_joy(n).last_press+1);
}

bool is_joystick_attached(int n)
{
    n--;
    return n >= 0 && n < int(joysticks.size());
}

bool is_joystick_pressed(int n, int button)
{
    if (!is_joystick_attached(n))
        return false;
    button = remap_button(button);
    button--;
    return get_joy(n).get_button(button);
}

bool any_joystick_pressed(int n)
{
    if (!is_joystick_attached(n))
        return false;
    JoystickData & joy = get_joy(n);
    for (unsigned int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
        if (joy.get_button(i))
            return true;
    }
    return false;
}

bool is_joystick_released(int n, int button)
{
    if (!is_joystick_attached(n))
        return true;
    button = remap_button(button);
    button--;
    return !get_joy(n).get_button(button);
}

void joystick_vibrate(int n, int l, int r, int ms)
{
    if (!is_joystick_attached(n))
        return;
    get_joy(n).vibrate(l / 100.0f, r / 100.0f, ms);
}

float get_joystick_axis(int n, int axis)
{
    if (!is_joystick_attached(n))
        return 0.0f;
    axis--;
    return get_joy(n).get_axis(axis);
}

// url open

#ifdef _WIN32

#include <windows.h>
#include <shellapi.h>

void open_url(const std::string & name)
{
    ShellExecute(NULL, "open", name.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

#elif __APPLE__

void open_url(const std::string & name)
{
    CFStringRef str = CFStringCreateWithCString(0, name.c_str(), 0);
    CFURLRef ref = CFURLCreateWithString(0, str, 0);
    LSOpenCFURLRef(ref, 0);
    CFRelease(ref);
    CFRelease(str);
}

#elif __linux

#include <sys/types.h>
#include <dirent.h>

void open_url(const std::string & name)
{
    std::string cmd("xdg-open '");
    cmd += name;
    cmd += "'";
    system(cmd.c_str());
}

#endif

// file

#ifdef CHOWDREN_ENABLE_STEAM
#include "sdk/public/steam/steam_api.h"
#include "sdk/public/steam/steamtypes.h"
#include <sstream>
#include "../path.h"
#endif

bool platform_remove_file(const std::string & file)
{
#ifdef CHOWDREN_ENABLE_STEAM
    std::string base = get_path_filename(file);
    const char * base_c = base.c_str();
    if (SteamRemoteStorage()->FileExists(base_c)) {
        return SteamRemoteStorage()->FileDelete(base_c);
    }
#endif
    return remove(convert_path(file).c_str()) == 0;
}

#include "../filecommon.cpp"
#include "stdiofile.cpp"

// path

std::string convert_path(const std::string & v)
{
    std::string value = v;
    if (value.compare(0, 3, "./\\") == 0)
        value = std::string("./", 2) + value.substr(3);
#ifndef _WIN32
    std::replace(value.begin(), value.end(), '\\', '/');
#else
    std::replace(value.begin(), value.end(), '/', '\\');
#endif
    return value;
}

// debug

void platform_print_stats()
{

}

// wiiu dummies

void platform_set_remote_setting(const std::string & v)
{

}

void platform_set_remote_value(int v)
{

}

int platform_get_remote_value()
{
    return CHOWDREN_TV_TARGET;
}

void platform_set_border(bool v)
{

}

static std::string remote_string("TV");

const std::string & platform_get_remote_setting()
{
    return remote_string;
}

bool platform_has_error()
{
    return false;
}
