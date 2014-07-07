#include "objects/surface.h"
#include "include_gl.h"
#include "collision.h"
#include <iostream>
#include "shader.h"
#include "mathcommon.h"

// SurfaceObject

SurfaceObject::SurfaceObject(int x, int y, int type_id)
: FrameObject(x, y, type_id), selected_index(-1), displayed_index(-1),
  load_failed(false), dest_width(0), dest_height(0), dest_x(0), dest_y(0),
  stretch_mode(0), effect(0)
{
    collision = new InstanceBox(this);
}

SurfaceObject::~SurfaceObject()
{
    delete collision;
}

void SurfaceObject::draw()
{
    // need to get light system working first
    if (shader == coldirblur_shader)
        return;

    if (display_selected)
        displayed_image = selected_image;

    if (displayed_image == NULL)
        return;
    blend_color.apply();
    displayed_image->draw(this, x, y);
}

void SurfaceObject::update(float dt)
{
    load_failed = false;
}

void SurfaceObject::resize(int w, int h)
{
    if (selected_image == NULL)
        return;
    selected_image->width = w;
    selected_image->height = h;

    if (displayed_image != selected_image)
        return;
    width = selected_image->width;
    height = selected_image->height;
    collision->update_aabb();
}

void SurfaceObject::resize_canvas(int x1, int y1, int x2, int y2)
{
    if (selected_image == NULL)
        return;

    if (x2 - x1 == 0 || y2 - y1 == 0) {
        selected_image->reset();
        return;
    }
    // x1 and y1 are always zero, yay
    //selected_image->scroll_x += x1;
    //selected_image->scroll_y += y1;
    selected_image->canvas_width = std::min(selected_image->canvas_width,
                                            x2);
    selected_image->canvas_height = std::min(selected_image->canvas_height,
                                             y2);
}

void SurfaceObject::load(const std::string & filename,
                         const std::string & ignore_ext)
{
    if (selected_image == NULL)
        return;
    Color old_trans = selected_image->transparent;
    selected_image->reset();
    selected_image->transparent = Color(255, 0, 255); // old_trans;

    std::string path = convert_path(filename);

    Color * trans = NULL;
    if (selected_image->has_transparent)
        trans = &selected_image->transparent;

    Image * image = get_image_cache(path, 0, 0, 0, 0, trans);
    selected_image->set_image(image);

    if (image == NULL)
        load_failed = true;

    set_edit_image(selected_index);
}

void SurfaceObject::set_stretch_mode(int v)
{
    stretch_mode = v;
}

void SurfaceObject::set_dest_pos(int x, int y)
{
    dest_x = x;
    dest_y = y;
}

void SurfaceObject::set_dest_size(int w, int h)
{
    dest_width = w;
    dest_height = h;
}

void SurfaceObject::scroll(int x, int y, int wrap)
{
    SurfaceImage * image = selected_image;
    if (image == NULL || image->handle == NULL)
        return;
    image->scroll_x = (image->scroll_x + x) % image->handle->width;
    image->scroll_y = (image->scroll_y + y) % image->handle->height;
    image->wrap = wrap != 0;
}

void SurfaceObject::blit(Active * obj)
{
    // std::cout << "Blit onto surface" << std::endl;
}

void SurfaceObject::set_effect(int index)
{
    effect = index;
}

void SurfaceObject::set_display_image(int index)
{
    if (display_selected)
        return;

    if (index < 0 || index >= int(images.size())) {
        displayed_index = -1;
        displayed_image = NULL;
    } else {
        displayed_index = index;
        displayed_image = &images[index];
    }
}

void SurfaceObject::set_edit_image(int index, bool display)
{
    if (index < 0 || index >= int(images.size())) {
		selected_index = -1;
        selected_image = NULL;
	} else {
		selected_index = index;
        selected_image = &images[index];
	}

    if (display_selected || display)
        set_display_image(index);

    if (displayed_image != selected_image)
        return;
    width = selected_image->get_display_width();
    height = selected_image->get_display_height();
}

void SurfaceObject::create_alpha(int index)
{
    // std::cout << "Create Surface alpha: " << index << std::endl;
}

void SurfaceObject::clear(const Color & color)
{
    // std::cout << "Clear Surface: "
    //     << color.r << " " << color.g << " " << color.b << std::endl;
}

void SurfaceObject::clear(int value)
{
    clear(Color(value));
}

void SurfaceObject::blit_background()
{
    std::cout << "Blit Surface background" << std::endl;
}

void SurfaceObject::blit_alpha(int image)
{
    std::cout << "Blit onto image alpha channel: " << image << std::endl;
}

void SurfaceObject::apply_matrix(double div, double offset, double iterations,
                                 double x1y1, double x1y2, double x1y3,
                                 double x2y1, double x2y2, double x2y3,
                                 double x3y1, double x3y2, double x3y3)
{
    std::cout << "Apply matrix not implemented" << std::endl;
}

void SurfaceObject::save(const std::string & filename,
                         const std::string & ext)
{
    std::cout << "Surface save not implemented: " << filename << std::endl;
}

void SurfaceObject::reverse_x()
{
    if (selected_image == NULL)
        return;
    selected_image->has_reverse_x = !selected_image->has_reverse_x;
}

void SurfaceObject::add_image(int w, int h)
{
    SurfaceImage new_image;
    new_image.width = w;
    new_image.height = h;
    images.push_back(new_image);
}

void SurfaceObject::set_transparent_color(const Color & color, bool replace)
{
    if (selected_image == NULL)
        return;
    // selected_image->has_transparent = true;
    // selected_image->transparent = color;
    // printf("newest color %x\n", color.get_int());
}

int SurfaceObject::get_edit_width()
{
    if (selected_image == NULL)
        return 0;
    return selected_image->width;
}

int SurfaceObject::get_image_width(int index)
{
    if (selected_image == NULL)
        return 0;
    return selected_image->width;
}

// SurfaceImage

void SurfaceImage::reset(int w, int h)
{
    handle = NULL;
    has_transparent = true; //?
    transparent = Color(0, 0, 0);
    width = w;
    height = h;
    canvas_width = w;
    canvas_height = h;
    scroll_x = 0;
    scroll_y = 0;
    wrap = false;
    has_reverse_x = false;
}

void SurfaceImage::set_image(Image * image)
{
	if (image == NULL) {
		reset();
		handle = NULL;
	} else {
		reset(image->width, image->height);
		handle = image;
	}
}

void SurfaceImage::draw(FrameObject * instance, int x, int y)
{
    if (handle == NULL)
        return;

    double scale_x = width / double(handle->width);
    double scale_y = height / double(handle->height);

    glEnable(GL_SCISSOR_TEST);
    glc_scissor_world(x, y, get_display_width(), get_display_height());

    if ((scroll_x == 0 && scroll_y == 0) || !wrap) {
        instance->draw_image(handle, x + scroll_x, y + scroll_y, 0.0,
                             scale_x, scale_y, has_reverse_x);
    } else {
        int start_x = x - (handle->width - scroll_x);
        int start_y = y - (handle->height - scroll_y);
        for (int xx = start_x; xx < x + canvas_width; xx += handle->width)
        for (int yy = start_y; yy < y + canvas_height; yy += handle->height) {
            instance->draw_image(handle, xx, yy, 0.0, 1.0, 1.0, has_reverse_x);
        }
    }

    glDisable(GL_SCISSOR_TEST);
}
