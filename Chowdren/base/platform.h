#ifndef CHOWDREN_PLATFORM_H
#define CHOWDREN_PLATFORM_H

#include <string>
#include <stdio.h>
#include "color.h"
#include "fileio.h"

void open_url(const std::string & name);
void platform_init();
void platform_exit();
void platform_poll_events();
bool platform_display_closed();
void platform_create_display(bool fullscreen);
void platform_get_size(int * width, int * height);
double platform_get_time();
void platform_sleep(double v);
size_t get_file_size(const char * filename);
void create_directories(const std::string & v);
int translate_key(int vk);
int translate_key(const std::string & name);
bool is_mouse_pressed(int button);
bool is_key_pressed(int button);
bool platform_has_focus();
void platform_set_focus(bool v);
void platform_show_mouse();
void platform_get_mouse_pos(int * x, int * y);
void platform_hide_mouse();
void platform_begin_draw();
void platform_swap_buffers();
bool platform_remove_file(const std::string & path);
const std::string & platform_get_appdata_dir();
const std::string & platform_get_language();

// debug
void platform_print_stats();

// joystick
bool is_joystick_attached(int n);
bool is_joystick_pressed(int n, int button);
bool any_joystick_pressed(int n);
bool is_joystick_released(int n, int button);
int get_joystick_direction(int n);
bool compare_joystick_direction(int n, int test_dir);
bool is_joystick_direction_changed(int n);
float get_joystick_x(int n);
float get_joystick_y(int n);

// file

class FSFile
{
public:
    void * handle;
    bool closed;

    FSFile();
    FSFile(const char * filename, const char * mode);
    ~FSFile();
    void open(const char * filename, const char * mode);
    bool seek(size_t v, int origin = SEEK_SET);
    size_t tell();
    bool is_open();
    void read_line(std::string & line);
    void read_delim(std::string & line, char delim);
    size_t read(void * data, size_t size);
    size_t write(void * data, size_t size);
    void close();
    bool at_end();
    int getc();
};

// path

std::string convert_path(const std::string & value);

// shaders

void init_shaders_platform();

// demo

#ifdef CHOWDREN_IS_DEMO
bool platform_show_build_info();
bool platform_should_reset();
#endif

// wiiu

#define CHOWDREN_TV_TARGET 0
#define CHOWDREN_REMOTE_TARGET 1
#define CHOWDREN_HYBRID_TARGET 2
#define CHOWDREN_REMOTE_ONLY 3
void platform_clone_buffers();
void platform_set_display_target(int value);
void platform_set_remote_setting(const std::string & v);
const std::string & platform_get_remote_setting();
int platform_get_remote_value();
unsigned int & platform_get_texture_pixel(unsigned int tex, int x, int y);
void platform_set_border(bool value);
bool platform_has_error();

#endif // CHOWDREN_PLATFORM_H
