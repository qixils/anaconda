#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

#include "../config.h"
#include "../platform.h"
#include "../include_gl.h"
#include "../manager.h"
#include "../mathcommon.h"

#include <tinythread/tinythread.h>

#include <iostream>

GLuint screen_texture;
GLuint screen_fbo;

GLFWwindow * global_window = NULL;

inline bool check_opengl_extension(const char * name)
{
    if (glewGetExtension(name) == GL_TRUE)
        return true;
    std::cout << "OpenGL extension '" << name << "' not supported." << std::endl;
    return false;
}

inline bool check_opengl_extensions()
{
    char * extensions[] = {
        "GL_EXT_framebuffer_object",
        "GL_ARB_vertex_shader",
        "GL_ARB_fragment_shader",
        "GL_ARB_texture_non_power_of_two",
        NULL
    };
    for (int i = 0; extensions[i] != NULL; i++)
        if (!check_opengl_extension(extensions[i]))
            return false;
    return true;
}

void _on_key(GLFWwindow * window, int key, int scancode, int action,
             int mods)
{
    if (action == GLFW_REPEAT)
        return;
    global_manager->on_key(key, action == GLFW_PRESS);
}

void _on_mouse(GLFWwindow * window, int button, int action, int mods) 
{
    global_manager->on_mouse(button, action == GLFW_PRESS);
}

void init_platform()
{
    glfwInit();
}

void platform_poll_events()
{
    glfwPollEvents();
}

#ifdef _WIN32

void platform_sleep(double t)
{
    DWORD tt;

    if(t == 0.0) {
        tt = 0;
    } else if (t < 0.001) {
        tt = 1;
    } else if (t > 2147483647.0) {
        tt = 2147483647;
    } else {
        tt = (DWORD)(t*1000.0 + 0.5);
    }
    Sleep(tt);
}

#else

void platform_sleep(double t)
{
    if(t == 0.0) {
        sched_yield();
        return;
    }

    struct timeval  currenttime;
    struct timespec wait;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    long dt_sec, dt_usec;

    // Not all pthread implementations have a pthread_sleep() function. We
    // do it the portable way, using a timed wait for a condition that we
    // will never signal. NOTE: The unistd functions sleep/usleep suspends
    // the entire PROCESS, not a signle thread, which is why we can not
    // use them to implement glfwSleep.

    // Set timeout time, relatvie to current time
    gettimeofday( &currenttime, NULL );
    dt_sec  = (long) t;
    dt_usec = (long) ((t - (double)dt_sec) * 1000000.0);
    wait.tv_nsec = (currenttime.tv_usec + dt_usec) * 1000L;
    if( wait.tv_nsec > 1000000000L )
    {
        wait.tv_nsec -= 1000000000L;
        dt_sec ++;
    }
    wait.tv_sec  = currenttime.tv_sec + dt_sec;

    // Initialize condition and mutex objects
    pthread_mutex_init( &mutex, NULL );
    pthread_cond_init( &cond, NULL );

    // Do a timed wait
    pthread_mutex_lock( &mutex );
    pthread_cond_timedwait( &cond, &mutex, &wait );
    pthread_mutex_unlock( &mutex );

    // Destroy condition and mutex objects
    pthread_mutex_destroy( &mutex );
    pthread_cond_destroy( &cond );
}

#endif

bool platform_display_closed()
{
    return glfwWindowShouldClose(global_window) == GL_TRUE;
}

void platform_get_mouse_pos(int * x, int * y)
{
    if (global_window == NULL) {
        *x = *y = 0;
        return;
    }
    double xx, yy;
    glfwGetCursorPos(global_window, &xx, &yy);
    *x = int(xx);
    *y = int(yy);
}

void platform_create_display(bool fullscreen)
{
    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    int width, height;
    GLFWmonitor * monitor = NULL;
    if (fullscreen) {
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode * desktop_mode = glfwGetVideoMode(monitor);
        width = desktop_mode->width;
        height = desktop_mode->height;
    } else {
        width = WINDOW_WIDTH;
        height = WINDOW_HEIGHT;
    }

    global_window = glfwCreateWindow(width, height, NAME, monitor, NULL);
    glfwMakeContextCurrent(global_window);

    glfwSwapInterval(0);
    glfwSetKeyCallback(global_window, _on_key);
    glfwSetMouseButtonCallback(global_window, _on_mouse);

    // initialize OpenGL extensions
    glewInit();

    // check extensions
    if (!check_opengl_extensions()) {
        std::cout << "Not all OpenGL extensions supported. Quitting..." 
            << std::endl;
        exit(EXIT_FAILURE);
        return;
    }
}

void platform_begin_draw()
{
    
}

void platform_swap_buffers()
{
    glfwSwapBuffers(global_window);
}

void platform_get_size(int * width, int * height)
{
    glfwGetFramebufferSize(global_window, width, height);
}

bool platform_has_focus()
{
    return glfwGetWindowAttrib(global_window, GLFW_FOCUSED) == GL_TRUE;
}

void platform_set_focus(bool value)
{
    if (value)
        glfwRestoreWindow(global_window);
    else
        glfwIconifyWindow(global_window);
}

void platform_show_mouse()
{
    glfwSetInputMode(global_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void platform_hide_mouse()
{
    glfwSetInputMode(global_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

// time

#ifdef _WIN32
// apparently, QueryPerformanceCounter sucks on Windows. use timeGetTime!

double platform_get_time()
{
    return timeGetTime() / 1000.0;
}

#else

double platform_get_time()
{
    return glfwGetTime();
}

#endif

// input

bool is_mouse_pressed(int button)
{
    if (button < 0)
        return false;
    return glfwGetMouseButton(global_window, button) == GLFW_PRESS;
}

bool is_key_pressed(int button)
{
    if (button < 0)
        return false;
    return glfwGetKey(global_window, button) == GLFW_PRESS;
}

// filesystem stuff

#include <sys/stat.h>

size_t get_file_size(const char * filename)
{
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

// for some reason, "unix" is not defined on OS X
#ifdef __APPLE__
#define unix
#endif
#include <platformstl/platformstl.hpp>
#include <platformstl/filesystem/path.hpp>
#include <platformstl/filesystem/directory_functions.hpp>

void create_directories(const std::string & value)
{
    platformstl::path path(value);
    path.pop();
    platformstl::create_directory_recurse(path);
}

// joystick

inline int joy_num(int n)
{
    return GLFW_JOYSTICK_1 - 1 + n;
}

bool is_joystick_attached(int n)
{
    return glfwJoystickPresent(joy_num(n)) == GL_TRUE;
}

bool is_joystick_pressed(int n, int button)
{
    int count;
    const unsigned char * buttons = glfwGetJoystickButtons(joy_num(n), &count);
    return buttons[button-1] == GLFW_PRESS;
}

bool any_joystick_pressed(int n)
{
    int count;
    const unsigned char * buttons = glfwGetJoystickButtons(joy_num(n), &count);
    for (int i = 0; i < count; i++)
        if (buttons[i] == GLFW_PRESS)
            return true;
    return false;
}

bool is_joystick_released(int n, int button)
{
    int count;
    const unsigned char * buttons = glfwGetJoystickButtons(joy_num(n), &count);
    return buttons[button-1] == GLFW_RELEASE;
}

int get_joystick_direction(int n)
{
    int count;
    const float * axes = glfwGetJoystickAxes(joy_num(n), &count);
    const static float threshold = 0.4f;
    float x = axes[0];
    float y = axes[1];
    if (get_length(x, y) < 0.4f)
        return 8; // center
    return int_round(atan2d(y, x) / (90.0f / 2.0f));
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

bool platform_remove_file(const std::string & file)
{
    return remove(convert_path(path).c_str()) == 0;
}

#include "../filecommon.cpp"

void FSFile::open(const char * filename, const char * mode)
{
    const char * real_mode;
    switch (*mode) {
        case 'r':
            real_mode = "rb";
            break;
        case 'w':
            real_mode = "wb";
            break;
    }
    FILE * fp = fopen(filename, real_mode);
    if (fp == NULL) {
        closed = true;
        return;
    }
    handle = (void*)fp;
    closed = false;
}

bool FSFile::seek(size_t v, int origin)
{
    return fseek((FILE*)handle, v, origin) == 0;
}

size_t FSFile::tell()
{
    return ftell((FILE*)handle);
}

int FSFile::getc()
{
    return fgetc((FILE*)handle);
}

size_t FSFile::read(void * data, size_t size)
{
    return fread(data, 1, size, (FILE*)handle);
}

size_t FSFile::write(void * data, size_t size)
{
    return fwrite(data, 1, size, (FILE*)handle);
}

void FSFile::close()
{
    fclose((FILE*)handle);
    closed = true;
}

bool FSFile::at_end()
{
    int c = getc();
    ungetc(c, (FILE*)handle);
    return c == EOF;
}

// path

std::string convert_path(const std::string & v)
{
#ifndef _WIN32
    std::string value = v;
    std::replace(value.begin(), value.end(), '\\', '/');
    return value;
#else
    return v;
#endif
}

// shaders

void init_shaders_platform()
{
}

// debug

void platform_print_stats()
{

}
