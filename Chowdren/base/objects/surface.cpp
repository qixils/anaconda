#include "objects/surface.h"
#include "objects/active.h"
#include "include_gl.h"
#include "collision.h"
#include <iostream>
#include "shader.h"
#include "mathcommon.h"
#include "common.h"

// SurfaceObject

#define SURFACE_FBO_WIDTH WINDOW_WIDTH
#define SURFACE_FBO_HEIGHT WINDOW_HEIGHT

static Framebuffer surface_fbo;
static bool has_fbo = false;

SurfaceObject::SurfaceObject(int x, int y, int type_id)
: FrameObject(x, y, type_id), selected_index(-1), displayed_index(-1),
  load_failed(false), dest_width(0), dest_height(0), dest_x(0), dest_y(0),
  stretch_mode(0), effect(0), selected_image(NULL), displayed_image(NULL),
  use_fbo_blit(false), use_image_blit(false), vert_index(0)
{
    if (!has_fbo) {
        has_fbo = true;
        surface_fbo.init(SURFACE_FBO_WIDTH, SURFACE_FBO_HEIGHT);
    }
    collision = new InstanceBox(this);
}

SurfaceObject::~SurfaceObject()
{
    delete collision;
}

void SurfaceObject::draw()
{
    if (!quads.empty()) {
        begin_draw();

        vector<SurfaceQuad>::const_iterator it;
        for (it = quads.begin(); it != quads.end(); it++) {
            const SurfaceQuad & quad = *it;
            int * p = (int*)&quad.points[0];
            Render::draw_quad(p, quad.color);
        }

        end_draw();
        return;
    }

    if (!lines.empty()) {
        lines.clear();
    }

    if (display_selected)
        displayed_image = selected_image;

    if (displayed_image == NULL)
        return;

    if (use_fbo_blit) {
        surface_fbo.bind();

        int old_offset[2] = Render::offset;
        Render::set_view(0, 0, SURFACE_FBO_WIDTH, SURFACE_FBO_HEIGHT);
        Render::set_offset(0, 0);
        Render::clear(clear_color);

        vector<SurfaceBlit>::const_iterator it;
        for (it = blit_images.begin(); it != blit_images.end(); it++) {
            const SurfaceBlit & img = *it;
            if (img.effect == 11)
                Render::set_effect(Render::Effect::SURFACESUBTRACT);
            int draw_x = img.x + img.image->hotspot_x * img.scale_x;
            int draw_y = img.y + img.image->hotspot_y * img.scale_y;
            img.image->draw(draw_x, draw_y, 0.0, img.scale_x, img.scale_y);
            if (img.effect == 11)
                Render::disable_effect();
        }
        blit_images.clear();

        Render::set_view(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        Render::set_offset(old_offset[0], old_offset[1]);
        surface_fbo.unbind();

        blend_color.apply();
        begin_draw(SURFACE_FBO_WIDTH, SURFACE_FBO_HEIGHT);

        int x1 = x;
        int y1 = y;
        int x2 = x1 + SURFACE_FBO_WIDTH;
        int y2 = y1 + SURFACE_FBO_HEIGHT;

        Render::draw_tex(x1, y1, x2, y2, blend_color,
                         surface_fbo.get_tex());

        end_draw();

        return;
    }

    if (use_image_blit) {
        vector<SurfaceBlit>::const_iterator it;
        int display_width = displayed_image->width;
        for (it = blit_images.begin(); it != blit_images.end(); it++) {
            const SurfaceBlit & img = *it;
            Image * h = img.image;

            // XXX this is a hack, generalize it
            if (shader == subpx_shader)
                h->set_filter(true);
            h->upload_texture();
            Texture t = h->tex;

            begin_draw(h->width, h->height);
            int off_x = x + h->hotspot_x * img.scale_x;
            int off_y = y + h->hotspot_x * img.scale_x;
            int draw_x = img.x + img.scroll_x;
            int draw_y = img.y + img.scroll_y;

            int w = h->width * img.scale_x;
            int h = h->height * img.scale_y;

            Render::draw_tex(draw_x + off_x, draw_y + off_y, 
                             draw_x + off_x + w, draw_y + off_y + h,
                             blend_color, t);

            for (int i = -1; i <= 1; i += 2) {
                int x1 = draw_x + display_width * i;
                int y1 = draw_y;
                int x2 = x1 + h->width;
                if (x1 >= 0 && x2 <= display_width)
                    continue;
                if (x1 >= display_width || x2 <= 0)
                    continue;
                int xx1 = x1 + off_x;
                int yy1 = y1 + off_y;
                Render::draw_tex(xx1, yy1, xx1 + w, yy1 + h, blend_color, t);
            }
            end_draw();
        }
    } else {
        if (displayed_image->handle == NULL)
            return;

        SurfaceImage & m = *displayed_image;
        Image * h = m.handle;

        int w = h->width;
        int h = h->height;
        float scale_x = m.width / float(w);
        float scale_y = m.height / float(h);

        Render::enable_scissor(x, y,
                               get_display_width(), get_display_height());

        if ((scroll_x == 0 && scroll_y == 0) || !wrap) {
            draw_image(h, x + scroll_x, y + scroll_y, blend_color,
                       0.0, scale_x, scale_y, has_reverse_x);
        } else {
            int start_x = x - (h->width - scroll_x);
            int start_y = y - (h->height - scroll_y);
            for (int xx = start_x; xx < x + m.canvas_width; xx += w)
            for (int yy = start_y; yy < y + m.canvas_height; yy += h) {
                draw_image(h, xx, yy, blend_color,
                           0.0, 1.0, 1.0, has_reverse_x);
            }
        }

        Render::disable_scissor();
    }
}

void SurfaceObject::update()
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

void SurfaceObject::resize_source(int w, int h)
{
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
    Image * image = get_image_cache(path, 0, 0, 0, 0,
                                    selected_image->transparent);
    selected_image->set_image(image);

    if (image == NULL)
        load_failed = true;

    set_edit_image(selected_index);
}

void SurfaceObject::set_stretch_mode(int v)
{
    stretch_mode = v;
}

void SurfaceObject::set_alpha_mode(int mode)
{
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

inline void wrap_pos(int & scroll, int pos, int size1, int size2)
{
    pos += scroll;
    if (pos >= size2)
        scroll -= size2;
    else if (pos + size1 <= 0)
        scroll += size2;
}

void SurfaceObject::scroll(int x, int y, int wrap)
{
    SurfaceImage * image = selected_image;
    image->wrap = wrap != 0;
    if (use_image_blit) {
        vector<SurfaceBlit>::iterator it;
        for (it = blit_images.begin(); it != blit_images.end(); it++) {
            SurfaceBlit & img = *it;
            img.scroll_x += x;
            img.scroll_y += y;
            wrap_pos(img.scroll_x, img.x, img.image->width, image->width);
            wrap_pos(img.scroll_y, img.y, img.image->height, image->height);
        }
        return;
    } else if (image == NULL || image->handle == NULL)
        return;
    image->scroll_x = (image->scroll_x + x) % image->handle->width;
    image->scroll_y = (image->scroll_y + y) % image->handle->height;
}

void SurfaceObject::blit(Active * obj)
{
    use_fbo_blit = true;
    if (!collides(0, 0, selected_image->width, selected_image->height,
                  dest_x, dest_y, dest_x+dest_width, dest_y+dest_height))
        return;

    Image * img = obj->image;
    int scale_x = SURFACE_FBO_WIDTH / selected_image->width;
    int scale_y = SURFACE_FBO_HEIGHT / selected_image->height;
    int index = blit_images.size();
    blit_images.resize(index+1);
    blit_images[index].x = dest_x * scale_x;
    blit_images[index].y = dest_y * scale_y;
    dest_width *= scale_x;
    dest_height *= scale_y;
    blit_images[index].scale_x = dest_width / double(img->width);
    blit_images[index].scale_y = dest_height / double(img->height);
    blit_images[index].scroll_x = 0;
    blit_images[index].scroll_y = 0;

    if (effect != 1 && effect != 11) {
        std::cout << "Unsupported blit effect: " << effect << std::endl;
        blit_images[index].effect = 1;
    } else
        blit_images[index].effect = effect;

    blit_images[index].image = img;
}

void SurfaceObject::blit(SurfaceObject * obj, int image)
{
    quads = obj->quads;
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
    quads.clear();
    clear_color = color;
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

void SurfaceObject::blit_image(int image)
{
    use_image_blit = true;
    Image * img = images[image].handle;
    int index = blit_images.size();
    blit_images.resize(index+1);
    blit_images[index].x = dest_x;
    blit_images[index].y = dest_y;
    blit_images[index].scale_x = dest_width / double(img->width);
    blit_images[index].scale_y = dest_height / double(img->height);
    blit_images[index].image = img;
    blit_images[index].scroll_x = 0;
    blit_images[index].scroll_y = 0;
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
    int index = images.size();
    images.resize(index + 1);
    images[index].reset();
    images[index].width = w;
    images[index].height = h;
    if (selected_index != -1)
        selected_image = &images[selected_index];
    if (displayed_index != -1)
        displayed_image = &images[displayed_index];
}

void SurfaceObject::set_transparent_color(const Color & color, bool replace)
{
    if (selected_image == NULL)
        return;
    selected_image->transparent = color;
}

int SurfaceObject::get_edit_height()
{
    if (selected_image == NULL)
        return 0;
    return selected_image->height;
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

void SurfaceObject::clear_alpha(int index)
{
}

void SurfaceObject::draw_rect(int x, int y, int w, int h, Color color,
                              int outline_size, Color outline)
{
    SurfaceQuad quad;
    quad.points[0].x = x;
    quad.points[0].y = y;
    quad.points[1].x = x+w;
    quad.points[1].y = y;
    quad.points[2].x = x+w;
    quad.points[2].y = y+h;
    quad.points[3].x = x;
    quad.points[3].y = y+h;
    quad.color = color;
    quads.push_back(quad);
}

void SurfaceObject::draw_line(int x1, int y1, int x2, int y2, Color color,
                              int width)
{
    SurfaceLine line;
    line.points[0].x = x1;
    line.points[0].y = y1;
    line.points[1].x = x2;
    line.points[1].y = y2;
    line.color = color;
    line.size = width;
    lines.push_back(line);
}

void SurfaceObject::draw_polygon(int x, int y, Color color,
                                 int outline_size, Color outline)
{
    vert_index = 0;
    color.set_alpha(255);
    quad.color = color;
    for (int i = 0; i < 4; i++) {
        quad.points[i].x += x;
        quad.points[i].y += y;
    }
    quads.push_back(quad);
}

void SurfaceObject::insert_point(int index, int x, int y)
{
    if (index == -1) {
        index = vert_index;
        vert_index++;
    }

    if (use_abs_coords) {
        x -= this->x + layer->off_x;
        y -= this->y + layer->off_y;
    }

    quad.points[index].x = x;
    quad.points[index].y = y;
}

// SurfaceImage

void SurfaceImage::reset(int w, int h)
{
    handle = NULL;
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

