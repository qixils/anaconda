#include "common.h"
#include "filecommon.h"
#include <string>
#include <boost/unordered_map.hpp>
#include "chowconfig.h"
#include "font.h"

std::string newline_character("\r\n");
std::string empty_string("");

// Font

Font::Font(const char * face, int size, bool bold, bool italic, bool underline)
: face(face),  size(size),  bold(bold), italic(italic), underline(underline)
{

}

// Background

Background::Background()
{
}

void clear_back_vec(BackgroundItems & items)
{
    BackgroundItems::iterator it;
    for (it = items.begin(); it != items.end(); it++) {
        BackgroundItem * item = *it;
        delete item;
    }
    items.clear();
}

Background::~Background()
{
    clear_back_vec(col_items);
    clear_back_vec(items);
}

void Background::reset(bool clear_items)
{
    if (clear_items) {
        clear_back_vec(col_items);
        clear_back_vec(items);
#ifdef CHOWDREN_USE_COLTREE
        tree.clear();
#endif
    }
}

void Background::destroy_at(int x, int y)
{
    BackgroundItems::iterator it = items.begin();
    while (it != items.end()) {
        BackgroundItem * item = *it;
        if (collides(item->dest_x, item->dest_y,
                     item->dest_x + item->src_width,
                     item->dest_y + item->src_height,
                     x, y, x, y)) {
            // tree.remove(item.tree_item);
            delete item;
            it = items.erase(it);
        } else {
            ++it;
        }
    }
    it = col_items.begin();
    while (it != col_items.end()) {
        BackgroundItem * item = *it;
        if (collides(item->dest_x, item->dest_y,
                     item->dest_x + item->src_width,
                     item->dest_y + item->src_height,
                     x, y, x, y)) {
#ifdef CHOWDREN_USE_COLTREE
            tree.remove(item->col);
#endif
            delete item;
            it = col_items.erase(it);
        } else
            it++;
    }
}

void Background::paste(Image * img, int dest_x, int dest_y,
                       int src_x, int src_y, int src_width, int src_height,
                       int collision_type)
{
    src_width = std::min(img->width, src_x + src_width) - src_x;
    src_height = std::min(img->height, src_y + src_height) - src_y;

    if (src_width <= 0 || src_height <= 0)
        return;

    BackgroundItem * item = new BackgroundItem(img, dest_x, dest_y,
                                               src_x, src_y,
                                               src_width, src_height,
                                               collision_type);
    if (collision_type == 1) {
        col_items.push_back(item);
#ifdef CHOWDREN_USE_COLTREE
        int x2 = dest_x + src_width;
        int y2 = dest_y + src_height;
        int v[4] = {dest_x, dest_y, x2, y2};
        item->col = tree.add(&item, v);
#endif
    } else {
        items.push_back(item);
    }
}

void Background::draw()
{
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    BackgroundItems::const_iterator it;
    for (it = items.begin(); it != items.end(); it++) {
        BackgroundItem * item = *it;
        item->draw();
    }
}

struct BackgroundItemCallback
{
    CollisionBase * col;

    BackgroundItemCallback(CollisionBase * col)
    : col(col)
    {
    }

    bool on_callback(void * data)
    {
        BackgroundItem * item = (BackgroundItem*)data;
        if (collide(col, item))
            return false;
        return true;
    }
};

bool Background::collide(CollisionBase * a)
{
#ifdef CHOWDREN_USE_COLTREE
    TreeItems tree_items;
    int box[4];
    a->get_box(box);
    BackgroundItemCallback callback(a);
    if (!tree.query(box, callback))
        return true;
#else
    BackgroundItems::iterator it;
    for (it = col_items.begin(); it != col_items.end(); it++) {
        BackgroundItem * item = *it;
        if (::collide(a, item))
            return true;
    }
#endif
    return false;
}

// Layer

Layer::Layer(double scroll_x, double scroll_y, bool visible, int index)
: visible(visible), scroll_x(scroll_x), scroll_y(scroll_y), back(NULL),
  index(index), x1(0), y1(0), x2(0), y2(0), x(0), y(0), off_x(0), off_y(0)
{
#if defined(CHOWDREN_IS_WIIU) || defined(CHOWDREN_EMULATE_WIIU)
    remote = CHOWDREN_TV_TARGET;
#endif
    scroll_active = scroll_x != 1.0 || scroll_y != 1.0;
}

Layer::~Layer()
{
    delete back;

    // layers are in charge of deleting background instances
    for (FlatObjectList::const_iterator it = background_instances.begin();
         it != background_instances.end(); it++) {
        delete (*it);
    }
}

void Layer::scroll(int off_x, int off_y, int dx, int dy)
{
    this->off_x = off_x;
    this->off_y = off_y;

    FlatObjectList::const_iterator it;
    for (it = instances.begin(); it != instances.end(); it++) {
        FrameObject * object = *it;
        if (object->scroll)
            continue;
        object->set_position(object->x + dx, object->y + dy);
    }
}

void Layer::set_position(int x, int y)
{
    int dx = x - this->x;
    int dy = y - this->y;
    this->x = x;
    this->y = y;
    FlatObjectList::const_iterator it;
    for (it = instances.begin(); it != instances.end(); it++) {
        FrameObject * object = *it;
        if (!object->scroll)
            continue;
        object->set_position(object->x + dx, object->y + dy);
    }
    // XXX should we change background instance positions (coltree)
    for (it = background_instances.begin(); it != background_instances.end();
         it++) {
        FrameObject * item = *it;
        item->set_position(item->x + dx, item->y + dy);
    }
}

void Layer::add_background_object(FrameObject * instance)
{
    CollisionBase * col = instance->collision;
    int b[4];
    if (col != NULL) {
        col->get_box(b);
        if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) {
            x1 = b[0];
            y1 = b[1];
            x2 = b[2];
            y2 = b[3];
        } else {
            rect_union(x1, y1, x2, y2,
                       b[0], b[1], b[2], b[3],
                       x1, y1, x2, y2);
        }
#ifdef CHOWDREN_USE_COLTREE
        tree.add(col, b);
#endif
    }

#ifdef CHOWDREN_USE_COLTREE
    if (col == NULL) {
        b[0] = instance->x;
        b[1] = instance->y;
        b[2] = b[0] + instance->width;
        b[3] = b[1] + instance->height;
    }

    display_tree.add(instance, b);
#endif
    instance->depth = background_instances.size();
    background_instances.push_back(instance);
}

void Layer::add_object(FrameObject * instance)
{
    instances.push_back(instance);
}

void Layer::insert_object(FrameObject * instance, int index)
{
    instances.insert(instances.begin() + index, instance);
}

void Layer::remove_object(FrameObject * instance)
{
    remove_list(instances, instance);
}

void Layer::set_level(FrameObject * instance, int index)
{
    remove_object(instance);
    if (index == -1)
        add_object(instance);
    else
        insert_object(instance, index);
}

int Layer::get_level(FrameObject * instance)
{
    return std::find(instances.begin(), instances.end(),
                     instance) - instances.begin();
}

void Layer::create_background()
{
    if (back == NULL)
        back = new Background;
}

void Layer::destroy_backgrounds()
{
    if (back == NULL)
        return;
    back->reset();
}

void Layer::destroy_backgrounds(int x, int y, bool fine)
{
    if (fine)
        std::cout << "Destroy backgrounds at " << x << ", " << y <<
            " (" << fine << ") not implemented" << std::endl;
    back->destroy_at(x, y);
}

struct BackgroundCallback
{
    CollisionBase * col;

    BackgroundCallback(CollisionBase * col)
    : col(col)
    {
    }

    bool on_callback(void * data)
    {
        CollisionBase * item = (CollisionBase*)data;
        if (collide(col, item))
            return false;
        return true;
    }
};

bool Layer::test_background_collision(CollisionBase * a)
{
    if (a == NULL)
        return false;
    int b[4];
    a->get_box(b);
    if (!collides(b[0], b[1], b[2], b[3], x1, y1, x2, y2))
        return false;
    if (back != NULL && back->collide(a))
        return true;
#ifdef CHOWDREN_USE_COLTREE
    BackgroundCallback callback(a);
    if (!tree.query(b, callback))
        return true;
#else
    FlatObjectList::const_iterator it;
    for (it = background_instances.begin(); it != background_instances.end();
         it++) {
        CollisionBase * col = (*it)->collision;
        if (col == NULL)
            continue;
        if (collide(a, col))
            return true;
    }
#endif
    return false;
}

bool Layer::test_background_collision(int x, int y)
{
    PointCollision col(x, y);
    return test_background_collision(&col);
}

void Layer::paste(Image * img, int dest_x, int dest_y,
                  int src_x, int src_y, int src_width, int src_height,
                  int collision_type)
{
    create_background();
    if (collision_type == 1) {
        int xx = dest_x + src_width;
        int yy = dest_x + src_height;
        if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) {
            x1 = dest_x;
            y1 = dest_y;
            x2 = xx;
            y2 = yy;
        } else {
            rect_union(x1, y1, x2, y2,
                       dest_x, dest_y, xx, yy,
                       x1, y1, x2, y2);
        }
    }
    back->paste(img, dest_x, dest_y, src_x, src_y,
        src_width, src_height, collision_type);
}

struct BackgroundDrawCallback
{
    FlatObjectList & list;

    BackgroundDrawCallback(FlatObjectList & list)
    : list(list)
    {
    }

    bool on_callback(void * data)
    {
        FrameObject * item = (FrameObject*)data;
        list.push_back(item);
        return true;
    }
};

inline bool sort_depth_comp(FrameObject * obj1, FrameObject * obj2)
{
    return obj1->depth < obj2->depth;
}

inline void sort_depth(FlatObjectList & list)
{
    std::sort(list.begin(), list.end(), sort_depth_comp);
}

inline void draw_sorted_list(FlatObjectList & list)
{
    sort_depth(list);
    FlatObjectList::const_iterator it;
    for (it = list.begin(); it != list.end(); it++) {
        (*it)->draw();
    }
}

void Layer::draw()
{
    if (!visible)
        return;

    // draw backgrounds
    int x1 = global_manager->frame->off_x * scroll_x;
    int y1 = global_manager->frame->off_y * scroll_y;
    int x2 = x1+WINDOW_WIDTH;
    int y2 = y1+WINDOW_HEIGHT;

    PROFILE_BEGIN(Layer_draw_background_instances);

    FlatObjectList::const_iterator it;

#ifdef CHOWDREN_USE_COLTREE
    static FlatObjectList draw_list;
    draw_list.clear();
    BackgroundDrawCallback callback(draw_list);
    int v[4] = {x1-x, y1-y, x2-x, y2-y};
    display_tree.query(v, callback);
    draw_sorted_list(draw_list);
#else
    for (it = background_instances.begin(); it != background_instances.end();
         it++) {
        FrameObject * item = *it;
        if (!collide_box(item, x1, y1, x2, y2))
            continue;
        item->draw();
    }
#endif

    PROFILE_END();

    PROFILE_BEGIN(Layer_draw_pasted);

    // draw pasted items
    if (back != NULL)
        back->draw();

    PROFILE_END();

    PROFILE_BEGIN(Layer_draw_instances);

    // draw active instances
    for (it = instances.begin(); it != instances.end(); it++) {
        FrameObject * item = *it;
        if (!item->visible)
            continue;
        if (!collide_box(item, x1, y1, x2, y2))
            continue;
        item->draw();
    }

    PROFILE_END();
}

#if defined(CHOWDREN_IS_WIIU) || defined(CHOWDREN_EMULATE_WIIU)
void Layer::set_remote(int value)
{
    remote = value;
}
#endif

// Frame

Frame::Frame(const std::string & name, int width, int height,
             Color background_color, int index, GameManager * manager)
: name(name), width(width), height(height), index(index),
  background_color(background_color), manager(manager),
  off_x(0), off_y(0), new_off_x(0), new_off_y(0), has_quit(false),
  last_key(-1), next_frame(-1), loop_count(0), frame_time(0.0),
  frame_iteration(0)
{
}

void Frame::event_callback(int id)
{
}

void Frame::on_start()
{
}

void Frame::on_end()
{
    ObjectList::iterator it;
    for (unsigned int i = 0; i < MAX_OBJECT_ID; i++) {
        ObjectList & list = GameManager::instances.items[i];
        for (it = list.begin(); it != list.end(); it++) {
            delete it->obj;
        }
    }
    std::vector<Layer*>::const_iterator layer_it;
    for (layer_it = layers.begin(); layer_it != layers.end(); layer_it++) {
        delete *layer_it;
    }
    layers.clear();
    GameManager::instances.clear();
    destroyed_instances.clear();
    next_frame = -1;
    loop_count = 0;
    off_x = 0;
    off_y = 0;
    frame_time = 0.0;
    frame_iteration++;
}

void Frame::handle_events() {}

void Frame::clean_instances()
{
    FlatObjectList::const_iterator it;
    for (it = destroyed_instances.begin(); it != destroyed_instances.end();
         it++) {
        FrameObject * instance = *it;
        GameManager::instances.items[instance->id].remove(instance);
        instance->layer->remove_object(instance);
        delete instance;
    }
    destroyed_instances.clear();
}

bool Frame::update(float dt)
{
    frame_time += dt;

    if (timer_base == 0) {
        timer_mul = 1.0f;
    } else {
        timer_mul = dt * timer_base;
    }

    PROFILE_BEGIN(frame_update_objects);

    ObjectList::iterator it;
    for (unsigned int i = 0; i < MAX_OBJECT_ID; i++) {
        ObjectList & list = GameManager::instances.items[i];
        for (it = list.begin(); it != list.end(); it++) {
            FrameObject * instance = it->obj;
            if (instance->destroying)
                continue;
            instance->update(dt);
            if (instance->movement)
                instance->movement->update(dt);
        }
    }

    PROFILE_END();

    PROFILE_BEGIN(clean_instances);
    clean_instances();
    PROFILE_END();

    PROFILE_BEGIN(handle_events);
    handle_events();
    update_display_center();
    PROFILE_END();

    last_key = -1;

    loop_count++;

    return !has_quit;
}

void Frame::pause()
{

}

void Frame::restart()
{
    next_frame = -2;
}

void Frame::draw(int remote)
{
    PROFILE_BEGIN(frame_draw_start);
    // first, draw the actual window contents
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
#if defined(CHOWDREN_IS_WIIU) || defined(CHOWDREN_EMULATE_WIIU)
    if (remote == CHOWDREN_REMOTE_TARGET)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    else
#endif
    {
        glClearColor(background_color.r / 255.0f, background_color.g / 255.0f,
                     background_color.b / 255.0f, 1.0f);
    }
#if defined(CHOWDREN_IS_WIIU) || defined(CHOWDREN_EMULATE_WIIU)
    if (remote != CHOWDREN_REMOTE_ONLY)
#endif
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    PROFILE_END();

    std::vector<Layer*>::const_iterator it;
    for (it = layers.begin(); it != layers.end(); it++) {
        Layer * layer = *it;
#if defined(CHOWDREN_IS_WIIU) || defined(CHOWDREN_EMULATE_WIIU)
        if (remote == CHOWDREN_HYBRID_TARGET) {
            if (layer->remote == CHOWDREN_REMOTE_TARGET)
                continue;
        } else if (remote == CHOWDREN_REMOTE_TARGET) {
            if (layer->remote == CHOWDREN_TV_TARGET)
                continue;
        } else if (remote == CHOWDREN_TV_TARGET) {
            if (layer->remote != CHOWDREN_TV_TARGET)
                continue;
        } else if (remote == CHOWDREN_REMOTE_ONLY) {
            if (layer->remote != CHOWDREN_REMOTE_TARGET)
                continue;
        }
#endif
        glLoadIdentity();
        glTranslatef(-floor(off_x * layer->scroll_x),
                     -floor(off_y * layer->scroll_y), 0.0);
        layer->draw();
    }

// #ifdef CHOWDREN_USE_BOX2D
//     Box2D::draw_debug();
// #endif
}

class DefaultActive : public Active
{
public:
    DefaultActive()
    : Active(0, 0, 0)
    {
        create_alterables();
    }
};

DefaultActive default_active;
FrameObject * default_active_instance = &default_active;

void Frame::add_layer(double scroll_x, double scroll_y, bool visible)
{
    layers.push_back(new Layer(scroll_x, scroll_y, visible, layers.size()));
}

FrameObject * Frame::add_object(FrameObject * instance, Layer * layer)
{
    instance->frame = this;
    instance->layer = layer;
    GameManager::instances.items[instance->id].add(instance);
    layer->add_object(instance);
    return instance;
}

FrameObject * Frame::add_object(FrameObject * instance, int layer_index)
{
    layer_index = int_max(0, int_min(layer_index, layers.size() - 1));
    Layer * layer = layers[layer_index];
    return add_object(instance, layer);
}

void Frame::add_background_object(FrameObject * instance, int layer_index)
{
    instance->frame = this;
    Layer * layer = layers[layer_index];
    instance->layer = layer;
    layer->add_background_object(instance);
}

void Frame::destroy_object(FrameObject * instance)
{
    if (instance->destroying)
        return;
    instance->destroying = true;
    destroyed_instances.push_back(instance);
}

void Frame::set_object_layer(FrameObject * instance, int new_layer)
{
    instance->layer->remove_object(instance);
    Layer * layer = layers[new_layer];
    layer->add_object(instance);
    instance->layer = layer;
}

int Frame::get_loop_index(const std::string & name)
{
    return *(loops[name].index);
}

void Frame::set_timer(double value)
{
    frame_time = value;
}

void Frame::set_lives(int value)
{
    manager->lives = value;
}

void Frame::set_display_center(int x, int y)
{
    if (x != -1) {
        new_off_x = int_max(0, x - WINDOW_WIDTH / 2);
        new_off_x = int_min(new_off_x, width - WINDOW_WIDTH);
    }
    if (y != -1) {
        new_off_y = int_max(0, y - WINDOW_HEIGHT / 2);
        new_off_y = int_min(new_off_y, height - WINDOW_HEIGHT);
    }
}

void Frame::update_display_center()
{
    if (off_x == new_off_x && off_y == new_off_y)
        return;

    std::vector<Layer*>::const_iterator it;
    for (it = layers.begin(); it != layers.end(); it++) {
        Layer * layer = *it;
        int x1 = off_x * layer->scroll_x;
        int y1 = off_y * layer->scroll_y;
        int x2 = new_off_x * layer->scroll_x;
        int y2 = new_off_y * layer->scroll_y;
        int layer_off_x = new_off_x - x2;
        int layer_off_y = new_off_y - y2;
        layer->scroll(layer_off_x, layer_off_y, x2 - x1, y2 - y1);
    }

    off_x = new_off_x;
    off_y = new_off_y;
}

int Frame::frame_left()
{
    return new_off_x;
}

int Frame::frame_right()
{
    return new_off_x + WINDOW_WIDTH;
}

int Frame::frame_top()
{
    return new_off_y;
}

int Frame::frame_bottom()
{
    return new_off_y + WINDOW_HEIGHT;
}

void Frame::set_background_color(int color)
{
    background_color = Color(color);
}

void Frame::get_mouse_pos(int * x, int * y)
{
    *x = manager->mouse_x + off_x;
    *y = manager->mouse_y + off_y;
}

int Frame::get_mouse_x()
{
    int x, y;
    get_mouse_pos(&x, &y);
    return x;
}

int Frame::get_mouse_y()
{
    int x, y;
    get_mouse_pos(&x, &y);
    return y;
}

bool Frame::test_background_collision(int x, int y)
{
    if (x < 0 || y < 0 || x > width || y > width)
        return false;
    std::vector<Layer*>::const_iterator it;
    for (it = layers.begin(); it != layers.end(); it++) {
        if ((*it)->test_background_collision(x, y))
            return true;
    }
    return false;
}

bool Frame::compare_joystick_direction(int n, int test_dir)
{
    if (!is_joystick_attached(n))
        return false;
    return get_joystick_direction(n) == test_dir;
}

bool Frame::is_joystick_direction_changed(int n)
{
    if (!is_joystick_attached(n))
        return false;
    // hack for now
    static int last_dir = get_joystick_direction(n);
    int new_dir = get_joystick_direction(n);
    bool ret = last_dir != new_dir;
    last_dir = new_dir;
    return ret;
}

void Frame::set_vsync(bool value)
{
    std::cout << "Set vsync: " << value << std::endl;
}

// FrameObject

FrameObject::FrameObject(int x, int y, int type_id)
: x(x), y(y), id(type_id), visible(true), shader(NULL),
  values(NULL), strings(NULL), shader_parameters(NULL), direction(0),
  destroying(false), scroll(true), movement(NULL), movements(NULL),
  movement_count(0), collision(NULL)
{
#ifdef CHOWDREN_USE_BOX2D
    body = -1;
#endif
}

FrameObject::~FrameObject()
{
    delete movement;
    if (movements != NULL) {
        for (int i = 0; i < movement_count; i++) {
            if (movement == movements[i])
                continue;
            delete movements[i];
        }
        delete[] movements;
    }
    delete shader_parameters;
}

void FrameObject::draw_image(Image * img, double x, double y, double angle,
                             double x_scale, double y_scale, bool flip_x,
                             bool flip_y)
{
    GLuint back_tex = 0;
    if (shader != NULL) {
        shader->begin(this, img);
        back_tex = shader->get_background_texture();
    }
    img->draw(x, y, angle, x_scale, y_scale, flip_x, flip_y, back_tex);
    if (shader != NULL)
        shader->end(this);
}

void FrameObject::set_position(int x, int y)
{
    this->x = x;
    this->y = y;
#ifdef CHOWDREN_USE_DYNTREE
    if (collision == NULL)
        return;
    collision->update_aabb();
#endif
}

void FrameObject::set_global_position(int x, int y)
{
    set_position(x - layer->off_x, y - layer->off_y);
}

void FrameObject::set_x(int x)
{
    this->x = x - layer->off_x;
#ifdef CHOWDREN_USE_DYNTREE
    if (collision == NULL)
        return;
    collision->update_aabb();
#endif
}

int FrameObject::get_x()
{
    return x + layer->off_x;
}

void FrameObject::set_y(int y)
{
    this->y = y - layer->off_y;
#ifdef CHOWDREN_USE_DYNTREE
    if (collision == NULL)
        return;
    collision->update_aabb();
#endif
}

int FrameObject::get_y()
{
    return y + layer->off_y;
}

void FrameObject::create_alterables()
{
    values = new AlterableValues;
    strings = new AlterableStrings;
}

void FrameObject::set_visible(bool value)
{
    flash(0);
    visible = value;
}

void FrameObject::set_blend_color(int color)
{
    int a = blend_color.a;
    blend_color = Color(color);
    blend_color.a = a;
}

void FrameObject::set_layer(int index)
{
    frame->set_object_layer(this, index);
}

void FrameObject::flash(float value) {}
void FrameObject::draw() {}
void FrameObject::update(float dt) {}
void FrameObject::set_direction(int value, bool set_movement)
{
    direction = value & 31;
    if (set_movement && movement != NULL)
        movement->set_direction(value);
}

int FrameObject::get_direction()
{
    return direction;
}

bool FrameObject::mouse_over()
{
    if (destroying)
        return false;
    int x, y;
    frame->get_mouse_pos(&x, &y);
    PointCollision col1(x, y);
    return collide(&col1, collision);
}

bool FrameObject::overlaps(FrameObject * other)
{
    if (destroying || other->destroying)
        return false;
    if (other->layer != layer)
        return false;
    return collide(other->collision, collision);
}

bool FrameObject::overlaps_background()
{
    if (destroying)
        return false;
    return layer->test_background_collision(collision);
}

bool FrameObject::overlaps_background_save()
{
    if (destroying)
        return false;
    bool ret = layer->test_background_collision(collision);
    if (movement != NULL) {
        movement->set_background_collision();
    }
    return ret;
}

bool FrameObject::outside_playfield()
{
    int box[4];
    collision->get_box(box);
    return !collides(box[0], box[1], box[2], box[3],
                     frame->off_x, frame->off_y,
                     frame->width+frame->off_x, frame->height+frame->off_y);
}

int FrameObject::get_box_index(int index)
{
    int box[4];
    collision->get_box(box);
    int ret = box[index];
    if (index == 0 || index == 2)
        ret += layer->off_x;
    else
        ret += layer->off_y;
    return ret;
}

void FrameObject::get_box(int box[4])
{
    collision->get_box(box);
}

void FrameObject::set_shader(Shader * value)
{
    if (shader_parameters == NULL)
        shader_parameters = new ShaderParameters;
    shader = value;
}

void FrameObject::set_shader_parameter(const std::string & name, double value)
{
    if (shader_parameters == NULL)
        shader_parameters = new ShaderParameters;
    (*shader_parameters)[name] = value;
}

void FrameObject::set_shader_parameter(const std::string & name,
                                       const Color & color)
{
    set_shader_parameter(name, (double)color.get_int());
}

double FrameObject::get_shader_parameter(const std::string & name)
{
    if (shader_parameters == NULL)
        return 0.0;
    return (*shader_parameters)[name];
}

void FrameObject::destroy()
{
    frame->destroy_object(this);
}

void FrameObject::set_level(int index)
{
    layer->set_level(this, index);
}

int FrameObject::get_level()
{
    return layer->get_level(this);
}

void FrameObject::move_back(FrameObject * other)
{
    if (other->layer != layer)
        return;
    int level = get_level();
    int level2 = other->get_level();
    if (level < level2)
        return;
    set_level(level2);
}

void FrameObject::move_back()
{
    set_level(0);
}

void FrameObject::move_front()
{
    set_level(-1);
}

void FrameObject::move_front(FrameObject * other)
{
    if (!other)
        return;
    if (other->layer != layer)
        return;
    int level = get_level();
    int level2 = other->get_level();
    if (level > level2)
        return;
    set_level(level2);
}

FixedValue FrameObject::get_fixed()
{
    return FixedValue(this);
}

void FrameObject::clear_movements()
{
    if (movements != NULL) {
        for (int i = 0; i < movement_count; i++) {
            if (movement == movements[i])
                continue;
            delete movements[i];
        }
        movements = NULL;
        movement_count = 0;
    }
    delete movement;
    movement = NULL;
}

void FrameObject::set_movement(int i)
{
    if (movement != NULL && (i < 0 || i >= movement_count))
        return;
    movement = movements[i];
    movement->start();
}

Movement * FrameObject::get_movement()
{
    if (movement == NULL)
        movement = new StaticMovement(this);
    return movement;
}

int FrameObject::get_action_x()
{
    return x;
}

int FrameObject::get_action_y()
{
    return y;
}

void FrameObject::shoot(FrameObject * other, int speed, int direction)
{
    if (direction == -1)
        direction = this->direction;
    other->set_position(get_action_x(), get_action_y());
    other->set_direction(direction);
    delete other->movement;
    other->movement = new ShootMovement(other);
    other->movement->set_max_speed(speed);
    other->movement->start();
}

double FrameObject::get_angle()
{
    return 0.0;
}

void FrameObject::set_angle(double angle, int quality)
{
}

const std::string & FrameObject::get_name()
{
#ifdef NDEBUG
    static const std::string v("Unspecified");
    return v;
#else
    return name;
#endif
}

void FrameObject::look_at(int x, int y)
{
    set_direction(get_direction_int(this->x, this->y, x, y));
}

void FrameObject::update_flash(float dt, float interval, float & t)
{
    if (interval != 0.0f) {
        t += dt;
        if (t >= interval) {
            t = 0.0f;
            visible = !visible;
        }
    }
}

void FrameObject::set_animation(int value)
{
}

// FixedValue

FixedValue::FixedValue(FrameObject * object)
: object(object)
{

}

FixedValue::operator double() const
{
    int64_t v = int64_t(intptr_t(object));
    double v2;
    memcpy(&v2, &v, sizeof(double));
    return v2;
}

FixedValue::operator std::string() const
{
    intptr_t val = intptr_t(object);
    return number_to_string(val);
}

FixedValue::operator FrameObject*() const
{
    return object;
}

// Direction

Direction::Direction(int index, int min_speed, int max_speed, int loop_count,
                     int back_to)
: index(index), min_speed(min_speed), max_speed(max_speed),
  loop_count(loop_count), back_to(back_to)
{

}

Direction::Direction()
{

}

// Animation

Animation::Animation()
: dirs()
{
}

// Animations

Animations::Animations(int count)
: count(count)
{
    items = new Animation*[count]();
}

// Active

inline int direction_diff(int dir1, int dir2)
{
    return (((dir1 - dir2 + 540) % 360) - 180);
}

inline Direction * find_nearest_direction(int dir, Direction ** dirs)
{
    int i = 0;
    Direction * check_dir;
    while (true) {
        i++;
        check_dir = dirs[(dir + i) & 31];
        if (check_dir != NULL)
            return check_dir;
        check_dir = dirs[(dir - i) & 31];
        if (check_dir != NULL)
            return check_dir;
    }
}

Active::Active(int x, int y, int type_id)
: FrameObject(x, y, type_id), forced_animation(-1),
  animation_frame(0), counter(0), angle(0.0), forced_frame(-1),
  forced_speed(-1), forced_direction(-1), x_scale(1.0), y_scale(1.0),
  animation_direction(0), stopped(false), flash_interval(0.0f),
  animation_finished(-1), transparent(false), flags(0)
{
    active_col.instance = this;
    collision = &active_col;
}

void Active::initialize_active()
{
    active_col.is_box = collision_box;
    update_direction();
}

Active::~Active()
{
}

void Active::initialize_animations()
{
    for (int i = 0; i < animations->count; i++) {
        Animation * anim = animations->items[i];
        if (anim == NULL)
            continue;
        Animation check_anim = *anim;
        for (int i = 0; i < 32; i++) {
            if (anim->dirs[i] == NULL)
                anim->dirs[i] = find_nearest_direction(i, check_anim.dirs);
        }
    }
}

void Active::set_animation(int value)
{
    if (value == animation)
        return;
    value = get_animation(value);
    if (value == animation)
        return;
    animation = value;
    if (forced_animation != -1)
        return;
    if (forced_frame == -1)
        animation_frame = 0;
    update_direction();
}

void Active::force_animation(int value)
{
    if (value == forced_animation)
        return;
    value = get_animation(value);
    if (value == forced_animation)
        return;
    int old_anim = get_animation();
    forced_animation = value;
    if (old_anim == forced_animation)
        return;
    if (forced_frame == -1)
        animation_frame = 0;
    update_direction();
}

void Active::force_frame(int value)
{
    if (forced_animation == DISAPPEARING)
        return;
    forced_frame = value;
    update_frame();
}

void Active::force_speed(int value)
{
    if (forced_animation == DISAPPEARING)
        return;
    forced_speed = value;
}

void Active::force_direction(int value)
{
    value &= 31;
    if (forced_direction == value)
        return;
    forced_direction = value;
    update_direction();
}

void Active::restore_animation()
{
    forced_animation = -1;
    if (forced_frame == -1)
        animation_frame = 0;
    update_direction();
}

void Active::restore_frame()
{
    if (forced_animation == DISAPPEARING)
        return;
    animation_frame = forced_frame;
    forced_frame = -1;
    update_frame();
}

void Active::restore_speed()
{
    forced_speed = -1;
}

void Active::add_direction(int animation, int direction,
                   int min_speed, int max_speed, int loop_count,
                   int back_to)
{
    Animation * anim = animations->items[animation];
    if (anim == NULL) {
        anim = new Animation;
        animations->items[animation] = anim;
    }
    anim->dirs[direction] = new Direction(direction, min_speed, max_speed,
                                          loop_count, back_to);
}

void Active::add_image(int animation, int direction, Image * image)
{
    Animation * anim = animations->items[animation];
    anim->dirs[direction]->frames.push_back(image);
}

void Active::update_frame()
{
    int frame_count = get_direction_data()->frames.size();
    int & current_frame = get_frame();
    current_frame = int_max(0, int_min(current_frame, frame_count - 1));

    Image * img = get_image();
    if (img == NULL)
        return;

    active_col.set_image(img);
    update_action_point();
}

void Active::update_direction()
{
    loop_count = get_direction_data()->loop_count;
    update_frame();
}

void Active::update_action_point()
{
    Image * img = get_image();
    active_col.get_transform(img->action_x, img->action_y,
                             action_x, action_y);
    action_x -= active_col.hotspot_x;
    action_y -= active_col.hotspot_y;
}

void Active::update(float dt)
{
    // XXX is this a good idea?
    if (!visible)
        return;

    if (animation_finished == DISAPPEARING) {
        FrameObject::destroy();
        return;
    }

    animation_finished = -1;

    if (forced_frame != -1 || stopped)
        return;

    update_flash(dt, flash_interval, flash_time);

    if (loop_count == 0)
        return;

    Direction * dir = get_direction_data();
    counter += int(get_speed() * frame->timer_mul);
    int old_frame = animation_frame;

    while (counter > 100) {
        counter -= 100;
        animation_frame++;
        if (animation_frame < (int)dir->frames.size())
            continue;
        if (loop_count > 0)
            loop_count--;
        if (loop_count != 0) {
            animation_frame = dir->back_to;
            continue;
        }
        animation_finished = get_animation();
        animation_frame--;

        if (forced_animation == APPEARING)
            restore_animation();
    }
    if (animation_frame != old_frame)
        update_frame();
}

void Active::draw()
{
    Image * img = get_image();
    if (img == NULL) {
        std::cout << "Invalid image draw (" << get_name() << ")" << std::endl;
        return;
    }
    blend_color.apply();
    bool blend = transparent || blend_color.a < 255;
    if (!blend)
        glDisable(GL_BLEND);
    draw_image(img, x, y, angle, x_scale, y_scale, false, false);
    if (!blend)
        glEnable(GL_BLEND);
}

inline Image * Active::get_image()
{
    Animation * anim = animations->items[get_animation()];
    if (anim == NULL) {
        std::cout << "Invalid animation: " << get_animation() << std::endl;
        return NULL;
    }
    Direction * dir = anim->dirs[get_animation_direction()];
    if (get_frame() >= (int)dir->frames.size()) {
        std::cout << "Invalid frame: " << get_frame() << " " <<
            dir->frames.size() << " " <<
            "(" << get_name() << ")" << std::endl;
        return NULL;
    }
    return dir->frames[get_frame()];
}

int Active::get_action_x()
{
    return x + action_x;
}

int Active::get_action_y()
{
    return y + action_y;
}

void Active::set_angle(double angle, int quality)
{
    angle = mod(angle, 360.0f);
    this->angle = angle;
    active_col.set_angle(angle);
    update_action_point();
}

double Active::get_angle()
{
    return angle;
}

int & Active::get_frame()
{
    if (forced_frame != -1)
        return forced_frame;
    return animation_frame;
}

int Active::get_speed()
{
    if (forced_speed != -1)
        return forced_speed;
    return get_direction_data()->max_speed;
}

Direction * Active::get_direction_data(int & dir)
{
    Animation * anim = animations->items[get_animation()];
    return anim->dirs[dir];
}

Direction * Active::get_direction_data()
{
    return get_direction_data(get_animation_direction());
}

int Active::get_animation()
{
    if (forced_animation != -1)
        return forced_animation;
    return animation;
}

int Active::get_animation(int value)
{
    if (has_animation(value))
        return value;
    for (value = 0; value < animations->count; value++) {
        if (has_animation(value))
            break;
    }
    return value;
}

void Active::set_direction(int value, bool set_movement)
{
    if (forced_animation == DISAPPEARING)
        return;
    FrameObject::set_direction(value, set_movement);
    if (auto_rotate) {
        set_angle(double(value) * 11.25);
        value = 0;
    }
    Direction * old_dir = get_direction_data();
    animation_direction = direction;
    if (old_dir == get_direction_data())
        return;
    update_direction();
}

int & Active::get_animation_direction()
{
    if (forced_direction != -1)
        return forced_direction;
    return animation_direction;
}

void Active::set_scale(double value)
{
    value = std::max<double>(0.0f, value);
    x_scale = y_scale = value;
    active_col.set_scale(value);
    update_action_point();
}

void Active::set_x_scale(double value)
{
    x_scale = std::max<double>(0.0f, value);
    active_col.set_x_scale(x_scale);
    update_action_point();
}

void Active::set_y_scale(double value)
{
    y_scale = std::max<double>(0.0f, value);
    active_col.set_y_scale(y_scale);
    update_action_point();
}

void Active::paste(int collision_type)
{
    Image * img = get_image();
    layer->paste(img, x-img->hotspot_x, y-img->hotspot_y, 0, 0,
                 img->width, img->height, collision_type);
}

bool Active::test_direction(int value)
{
    return get_direction() == value;
}

bool Active::test_directions(int value)
{
    int direction = get_direction();
    return ((value >> direction) & 1) != 0;
}

bool Active::test_animation(int value)
{
    return value == get_animation();
}

void Active::stop_animation()
{
    stopped = true;
}

void Active::start_animation()
{
    stopped = false;
}

void Active::flash(float value)
{
    flash_interval = value;
    flash_time = 0.0f;
}

void Active::enable_flag(int index)
{
    flags |= 1 << index;
}

void Active::disable_flag(int index)
{
    flags &= ~(1 << index);
}

void Active::toggle_flag(int index)
{
    flags ^= 1 << index;
}

bool Active::is_flag_on(int index)
{
    return (flags & (1 << index)) != 0;
}

bool Active::is_flag_off(int index)
{
    return (flags & (1 << index)) == 0;
}

int Active::get_flag(int index)
{
    return int(is_flag_on(index));
}

bool Active::is_near_border(int border)
{
    int box[4];
    get_box(box);

    if (box[0] <= frame->frame_left() + border)
        return true;

    if (box[2] >= frame->frame_right() - border)
        return true;

    if (box[1] <= frame->frame_top() + border)
        return true;

    if (box[3] >= frame->frame_bottom() - border)
        return true;

    return false;
}

bool Active::is_animation_finished(int anim)
{
    return animation_finished == anim;
}

void Active::destroy()
{
    if (forced_animation == DISAPPEARING)
        return;
    if (!has_animation(DISAPPEARING)) {
        FrameObject::destroy();
        return;
    }
    clear_movements();
    force_animation(DISAPPEARING);
    collision = NULL;
}

bool Active::has_animation(int anim)
{
    if (anim >= animations->count)
        return false;
    if (animations->items[anim] == NULL)
        return false;
    return true;
}

// Text

FTTextureFont * small_font = NULL;
FTTextureFont * big_font = NULL;
std::string font_path;

void set_font_path(const std::string & value)
{
    if (!font_path.empty())
        return;
    font_path = value;
}

void init_font()
{
    static bool initialized = false;
    if (initialized)
        return;
    set_font_path(std::string("Font.dat")); // default font, could be set already
    load_fonts(font_path, &small_font, &big_font);
    initialized = true;
}

FTTextureFont * get_font(int size)
{
    init_font();
    if (size >= 24)
        return big_font;
    else
        return small_font;
}

Text::Text(int x, int y, int type_id)
: FrameObject(x, y, type_id), initialized(false), current_paragraph(0),
  draw_text_set(false), layout(NULL)
{
    collision = new InstanceBox(this);
}

Text::~Text()
{
    delete collision;
    delete layout;
}

void Text::add_line(std::string text)
{
    paragraphs.push_back(text);
    if (!initialized) {
        initialized = true;
        this->text = text;
    }
}

void Text::draw()
{
    update_draw_text();
    blend_color.apply();
    glPushMatrix();
    if (layout != NULL) {
        FTBBox bb = layout->BBox(draw_text.c_str(), -1);
        double off_x = x;
        double off_y = y;
        glTranslated((int)off_x, (int)off_y, 0.0);
        glScalef(1, -1, 1);
        layout->Render(draw_text.c_str(), -1, FTPoint());
        glPopMatrix();
    } else {
        FTTextureFont * font = get_font(size);
        FTBBox box = font->BBox(draw_text.c_str(), -1, FTPoint());
        double box_w = box.Upper().X() - box.Lower().X();
        double box_h = box.Upper().Y() - box.Lower().Y();
        double off_x = x;
        double off_y = y + font->Ascender();

        if (alignment & ALIGN_HCENTER)
            off_x += 0.5 * (width - box_w);
        else if (alignment & ALIGN_RIGHT)
            off_x += width - box_w;

        if (alignment & ALIGN_VCENTER) {
            off_y += height * 0.5 - font->LineHeight() * 0.5;
        } else if (alignment & ALIGN_BOTTOM) {
            off_y += font->LineHeight();
        }

#ifdef CHOWDREN_BIG_FONT_OFFY
        if (font == big_font)
            off_y += CHOWDREN_BIG_FONT_OFFY;
#endif

        glTranslated((int)off_x, (int)off_y, 0.0);
        glScalef(1, -1, 1);
        font->Render(draw_text.c_str(), -1, FTPoint(), FTPoint());
        glPopMatrix();
    }
}

void Text::set_string(std::string value)
{
    text = value;
    draw_text_set = false;
}

void Text::set_paragraph(unsigned int index)
{
    current_paragraph = index;
    set_string(get_paragraph(index));
}

void Text::next_paragraph()
{
    set_paragraph(current_paragraph + 1);
}

int Text::get_index()
{
    return current_paragraph;
}

int Text::get_count()
{
    return paragraphs.size();
}

bool Text::get_bold()
{
    return bold;
}

bool Text::get_italic()
{
    return italic;
}

void Text::set_bold(bool value)
{
    bold = value;
}

std::string Text::get_paragraph(int index)
{
    if (index < 0)
        index = 0;
    else if (index >= (int)paragraphs.size())
        index = paragraphs.size() - 1;
    return paragraphs[index];
}

void Text::update_draw_text()
{
    if (draw_text_set)
        return;
    // convert from windows-1252 to utf-8
    draw_text_set = true;
#ifdef CHOWDREN_TEXT_USE_UTF8
    draw_text = text;
#else
    // convert from windows-1252 to utf-8
    std::string::const_iterator it;
    draw_text.clear();
    for (it = text.begin(); it != text.end(); it++) {
        char c = *it;
        unsigned char cc = (unsigned char)c;
        if (cc < 128) {
            draw_text.push_back(c);
        } else {
            draw_text.push_back(char(0xC2 + (cc > 0xBF)));
            draw_text.push_back(char(cc & 0x3F + 0x80));
        }
    }
#endif
}

void Text::set_width(int w)
{
    width = w;
    if (layout == NULL) {
        layout = new FTSimpleLayout;
        layout->SetFont(get_font(size));
    }
    layout->SetLineLength(w);
}

int Text::get_width()
{
    if (layout == NULL)
        return width;
    update_draw_text();
    FTBBox bb = layout->BBox(draw_text.c_str(), text.size());
    return (int)(bb.Upper().X() - bb.Lower().X());
}

int Text::get_height()
{
    if (layout == NULL)
        return height;
    update_draw_text();
    FTBBox bb = layout->BBox(draw_text.c_str(), text.size());
    return (int)(bb.Upper().Y() - bb.Lower().Y());
}

// FontInfo

int FontInfo::get_width(FrameObject * obj)
{
    return ((Text*)obj)->get_width();
}

int FontInfo::get_height(FrameObject * obj)
{
    return ((Text*)obj)->get_height();
}

void FontInfo::set_width(FrameObject * obj, int width)
{
    ((Text*)obj)->set_width(width);
}

std::string FontInfo::vertical_tab("\x0B");

// Backdrop

Backdrop::Backdrop(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{
#if defined(CHOWDREN_IS_WIIU) || defined(CHOWDREN_EMULATE_WIIU)
    remote = CHOWDREN_HYBRID_TARGET;
#endif
}

Backdrop::~Backdrop()
{
    delete image;
    delete collision;
}

void Backdrop::draw()
{
#if defined(CHOWDREN_IS_WIIU) || defined(CHOWDREN_EMULATE_WIIU)
    int current_remote = platform_get_remote_value();
    if (remote == CHOWDREN_REMOTE_TARGET &&
        current_remote != CHOWDREN_HYBRID_TARGET)
        return;
#endif
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    image->draw(x, y);
}

// QuickBackdrop

QuickBackdrop::QuickBackdrop(int x, int y, int type_id)
: FrameObject(x, y, type_id), image(NULL)
{
}

QuickBackdrop::~QuickBackdrop()
{
    delete collision;
    delete image;
}

void QuickBackdrop::draw()
{
    if (image != NULL) {
        glEnable(GL_SCISSOR_TEST);
        glc_scissor_world(x, y, width, height);
        for (int xx = x; xx < x + width; xx += image->width)
        for (int yy = y; yy < y + height; yy += image->height) {
            image->draw(xx, yy);
        }
        glDisable(GL_SCISSOR_TEST);
    } else {
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        switch (gradient_type) {
            case NONE_GRADIENT:
                glColor4ub(color.r, color.g, color.b, blend_color.a);
                glVertex2f(x, y);
                glVertex2f(x + width, y);
                glVertex2f(x + width, y + height);
                glVertex2f(x, y + height);
                break;
            case VERTICAL_GRADIENT:
                glColor4ub(color.r, color.g, color.b, blend_color.a);
                glVertex2f(x, y);
                glVertex2f(x + width, y);
                glColor4ub(color2.r, color2.g, color2.b, blend_color.a);
                glVertex2f(x + width, y + height);
                glVertex2f(x, y + height);
                break;
            case HORIZONTAL_GRADIENT:
                glColor4ub(color.r, color.g, color.b, blend_color.a);
                glVertex2f(x, y + height);
                glVertex2f(x, y);
                glColor4ub(color2.r, color2.g, color2.b, blend_color.a);
                glVertex2f(x + width, y);
                glVertex2f(x + width, y + height);
                break;
        }
        glEnd();
    }
}

// Counter

Counter::Counter(int x, int y, int type_id)
: FrameObject(x, y, type_id), flash_interval(0.0f)
{
    for (int i = 0; i < 14; i++)
        images[i] = NULL;
}

Counter::~Counter()
{
    delete collision;
}

void Counter::calculate_box()
{
    if (type != IMAGE_COUNTER)
        return;
    width = 0;
    height = 0;
    for (std::string::const_iterator it = cached_string.begin();
         it != cached_string.end(); it++) {
        Image * image = get_image(it[0]);
        width += image->width;
        height = std::max(image->height, height);
    }
    ((OffsetInstanceBox*)collision)->set_offset(-width, -height);
}

Image * Counter::get_image(char c)
{
    switch (c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return images[c - '0'];
        case '-':
            return images[10];
        case '+':
            return images[11];
        case '.':
            return images[12];
        case 'e':
            return images[13];
        default:
            return NULL;
    }
}

void Counter::add(double value)
{
    set(this->value + value);
}

void Counter::subtract(double value)
{
    set(this->value - value);
}

void Counter::set(double value)
{
    value = std::max<double>(std::min<double>(value, maximum), minimum);
    this->value = value;

    if (type != IMAGE_COUNTER)
        return;

    std::ostringstream str;
    str << value;
    cached_string = str.str();

    if (collision == NULL) {
        collision = new OffsetInstanceBox(this);
    }

    calculate_box();
}

void Counter::set_max(double value)
{
    maximum = value;
    set(this->value);
}

void Counter::set_min(double value)
{
    minimum = value;
    set(this->value);
}

void Counter::update(float dt)
{
    update_flash(dt, flash_interval, flash_time);
}

void Counter::flash(float value)
{
    flash_interval = value;
    flash_time = 0.0f;
}

void Counter::draw()
{
    if (type == HIDDEN_COUNTER)
        return;

    blend_color.apply();

    if (type == IMAGE_COUNTER) {
        double current_x = x;
        for (std::string::reverse_iterator it = cached_string.rbegin();
             it != cached_string.rend(); it++) {
            Image * image = get_image(it[0]);
            if (image == NULL)
                continue;
            image->draw(current_x + image->hotspot_x - image->width,
                        y + image->hotspot_y - image->height);
            current_x -= image->width;
        }
    } else if (type == VERTICAL_UP_COUNTER) {
        float p = (value - minimum) /
                  (maximum - minimum);
        int draw_height = p * height;
        int x2 = x + width;
        int y2 = y + height;
        int y1 = y2 - draw_height;
        color1.apply();
        glBegin(GL_QUADS);
        glVertex2f(x, y1);
        glVertex2f(x2, y1);
        glVertex2f(x2, y2);
        glVertex2f(x, y2);
        glEnd();
    }
}

// Lives

Lives::Lives(int x, int y, int type_id)
: FrameObject(x, y, type_id), flash_interval(0.0f)
{
}

void Lives::flash(float value)
{
    flash_interval = value;
    flash_time = 0.0f;
}

void Lives::update(float dt)
{
    update_flash(dt, flash_interval, flash_time);
}

void Lives::draw()
{
    blend_color.apply();

    int xx = x;
    int i = 0;
    while (i < frame->manager->lives) {
        image->draw(xx, y);
        xx += image->width;
        i++;
    }
}

// INI

#include "ini.cpp"

inline bool match_wildcard(const std::string & pattern,
                           const std::string & value)
{
    if (pattern.empty() || value.empty())
        return pattern == value;
    if (pattern == "*")
        return true;
    else if (pattern[0] == '*') {
        size_t size = pattern.size() - 1;
        if (size > value.size())
            return false;
        return pattern.compare(1, size, value, value.size() - size, size) == 0;
    } else if (pattern[pattern.size() - 1] == '*') {
        size_t size = pattern.size() - 1;
        if (size > value.size())
            return false;
        return pattern.compare(0, size, value, 0, size) == 0;
    } else if (std::count(pattern.begin(), pattern.end(), '*') > 0) {
        std::cout << "Generic wildcard not implemented yet: " << pattern
            << std::endl;
        return false;
    }
    return value == pattern;
}

INI::INI(int x, int y, int type_id)
: FrameObject(x, y, type_id), overwrite(false), auto_save(false)
{
}

void INI::reset_global_data()
{
    global_data.clear();
}

int INI::_parse_handler(void* user, const char* section, const char* name,
                               const char* value)
{
    INI * reader = (INI*)user;
    reader->parse_handler(section, name, value);
    return 1;
}

void INI::parse_handler(const std::string & section, const std::string & name,
                        const std::string & value)
{
    if (!overwrite && has_item(section, name))
        return;
    data[section][name] = value;
}

void INI::set_group(const std::string & name, bool new_group)
{
    current_group = name;
}

std::string INI::get_string(const std::string & group, const std::string & item,
                            const std::string & def)
{
    SectionMap::const_iterator it = data.find(group);
    if (it == data.end())
        return def;
    OptionMap::const_iterator new_it = (*it).second.find(item);
    if (new_it == (*it).second.end())
        return def;
    return (*new_it).second;
}

std::string INI::get_string(const std::string & item, const std::string & def)
{
    return get_string(current_group, item, def);
}

std::string INI::get_string(const std::string & item)
{
    return get_string(item, empty_string);
}

std::string INI::get_string_index(const std::string & group, unsigned int index)
{
    SectionMap::const_iterator it = data.find(group);
    if (it == data.end())
        return empty_string;
    OptionMap::const_iterator new_it = (*it).second.begin();
    int current_index = 0;
    while (new_it != (*it).second.end()) {
        if (current_index == index)
            return (*new_it).second;
        new_it++;
        current_index++;
    }
    return empty_string;
}

std::string INI::get_string_index(unsigned int index)
{
    return get_string_index(current_group, index);
}

std::string INI::get_item_name(const std::string & group, unsigned int index)
{
    SectionMap::const_iterator it = data.find(group);
    if (it == data.end())
        return empty_string;
    OptionMap::const_iterator new_it = (*it).second.begin();
    int current_index = 0;
    while (new_it != (*it).second.end()) {
        if (current_index == index)
            return (*new_it).first;
        new_it++;
        current_index++;
    }
    return empty_string;
}

std::string INI::get_item_name(unsigned int index)
{
    return get_item_name(current_group, index);
}

std::string INI::get_group_name(unsigned int index)
{
    SectionMap::const_iterator it = data.begin();
    int current_index = 0;
    while (it != data.end()) {
        if (current_index == index)
            return (*it).first;
        it++;
        current_index++;
    }
    return empty_string;
}

double INI::get_value(const std::string & group, const std::string & item,
                      double def)
{
    SectionMap::const_iterator it = data.find(group);
    if (it == data.end())
        return def;
    OptionMap::const_iterator new_it = (*it).second.find(item);
    if (new_it == (*it).second.end())
        return def;
    return string_to_double((*new_it).second, def);
}

double INI::get_value(const std::string & item, double def)
{
    return get_value(current_group, item, def);
}

double INI::get_value(const std::string & item)
{
    return get_value(item, 0.0);
}

double INI::get_value_index(const std::string & group, unsigned int index)
{
    SectionMap::const_iterator it = data.find(group);
    if (it == data.end())
        return 0.0;
    OptionMap::const_iterator new_it = (*it).second.begin();
    int current_index = 0;
    while (new_it != (*it).second.end()) {
        if (current_index == index)
            return string_to_double((*new_it).second, 0.0);
        new_it++;
        current_index++;
    }
    return 0.0;
}

double INI::get_value_index(unsigned int index)
{
    return get_value_index(current_group, index);
}

void INI::set_value(const std::string & group, const std::string & item,
                    int pad, double value)
{
    set_string(group, item, number_to_string(value));
}

void INI::set_value(const std::string & item, int pad, double value)
{
    set_value(current_group, item, pad, value);
}

void INI::set_value(const std::string & item, double value)
{
    set_value(item, 0, value);
}

void INI::set_string(const std::string & group, const std::string & item,
                     const std::string & value)
{
    data[group][item] = value;
    save_auto();
}

void INI::set_string(const std::string & item, const std::string & value)
{
    set_string(current_group, item, value);
}

void INI::load_file(const std::string & fn, bool read_only, bool merge,
                    bool overwrite)
{
    this->read_only = read_only;
    filename = convert_path(fn);
    if (!merge)
        reset(false);
    std::cout << "Loading " << filename << " (" << get_name() << ")"
        << std::endl;
    create_directories(filename);
    int e = ini_parse_file(filename.c_str(), _parse_handler, this);
    std::cout << "Done loading" << std::endl;
    if (e != 0) {
        std::cout << "INI load failed (" << filename << ") with code " << e
        << std::endl;
    }
}

void INI::load_string(const std::string & data, bool merge)
{
    if (!merge)
        reset(false);
    int e = ini_parse_string(data, _parse_handler, this);
    if (e != 0) {
        std::cout << "INI load failed with code " << e << std::endl;
    }
}

void INI::merge_file(const std::string & fn, bool overwrite)
{
    load_file(fn, false, true, overwrite);
}

void INI::get_data(std::stringstream & out)
{
    SectionMap::const_iterator it1;
    OptionMap::const_iterator it2;
    for (it1 = data.begin(); it1 != data.end(); it1++) {
        out << "[" << (*it1).first << "]" << std::endl;
        for (it2 = (*it1).second.begin(); it2 != (*it1).second.end();
             it2++) {
            out << (*it2).first << "=" << (*it2).second << std::endl;
        }
        out << std::endl;
    }
}

void INI::save_file(const std::string & fn, bool force)
{
    if (fn.empty() || (read_only && !force))
        return;
    filename = convert_path(fn);
    create_directories(filename);
    std::stringstream out;
    get_data(out);
    FSFile fp(filename.c_str(), "w");
    std::string outs = out.str();
    fp.write(&outs[0], outs.size());
    fp.close();
}

std::string INI::as_string()
{
    std::stringstream out;
    get_data(out);
    return out.str();
}

void INI::save_file(bool force)
{
    save_file(filename, force);
}

void INI::save_auto()
{
    if (!auto_save)
        return;
    save_file(false);
}

int INI::get_item_count(const std::string & section)
{
    return data[section].size();
}

int INI::get_item_count()
{
    return get_item_count(current_group);
}

int INI::get_group_count()
{
    return data.size();
}

bool INI::has_group(const std::string & group)
{
    SectionMap::const_iterator it = data.find(group);
    if (it == data.end())
        return false;
    return true;
}

bool INI::has_item(const std::string & group, const std::string & option)
{
    SectionMap::const_iterator it = data.find(group);
    if (it == data.end())
        return false;
    OptionMap::const_iterator new_it = (*it).second.find(option);
    if (new_it == (*it).second.end())
        return false;
    return true;
}

bool INI::has_item(const std::string & option)
{
    return has_item(current_group, option);
}

void INI::search(const std::string & group, const std::string & item,
                 const std::string & value)
{
    search_results.clear();
    SectionMap::const_iterator it1;
    OptionMap::const_iterator it2;
    for (it1 = data.begin(); it1 != data.end(); it1++) {
        if (!match_wildcard(group, (*it1).first))
            continue;
        for (it2 = (*it1).second.begin(); it2 != (*it1).second.end();
             it2++) {
            if (!match_wildcard(item, (*it2).first))
                continue;
            if (!match_wildcard(value, (*it2).second))
                continue;
            search_results.push_back(
                std::pair<std::string, std::string>(
                    (*it1).first,
                    (*it2).first
                ));
        }
    }
}

void INI::delete_pattern(const std::string & group, const std::string & item,
                         const std::string & value)
{
    SectionMap::iterator it1;
    OptionMap::iterator it2;
    for (it1 = data.begin(); it1 != data.end(); it1++) {
        if (!match_wildcard(group, (*it1).first))
            continue;
        OptionMap & option_map = (*it1).second;
        it2 = option_map.begin();
        while (it2 != option_map.end()) {
            if (!match_wildcard(item, (*it2).first) ||
                !match_wildcard(value, (*it2).second)) {
                it2++;
                continue;
            }
            option_map.erase(it2++);
        }
    }
    save_auto();
}

void INI::sort_group_by_name(const std::string & group)
{

}

void INI::reset(bool save)
{
    data.clear();
    if (save)
        save_auto();
}

void INI::delete_group(const std::string & group)
{
    data.erase(group);
    save_auto();
}

void INI::delete_group()
{
    delete_group(current_group);
}

void INI::delete_item(const std::string & group, const std::string & item)
{
    data[group].erase(item);
    save_auto();
}

void INI::delete_item(const std::string & item)
{
    delete_item(current_group, item);
}

void INI::set_global_data(const std::string & key)
{
    data = global_data[key];
    global_key = key;
}

void INI::merge_object(INI * other, bool overwrite)
{
    merge_map(other->data, overwrite);
}

void INI::merge_group(INI * other, const std::string & src_group,
                     const std::string & dst_group, bool overwrite)
{
    merge_map(other->data, src_group, dst_group, overwrite);
}

void INI::merge_map(const SectionMap & data2, bool overwrite)
{
    SectionMap::const_iterator it1;
    OptionMap::const_iterator it2;
    for (it1 = data2.begin(); it1 != data2.end(); it1++) {
        for (it2 = (*it1).second.begin(); it2 != (*it1).second.end();
             it2++) {
            if (!overwrite && has_item((*it1).first, (*it2).first))
                continue;
            data[(*it1).first][(*it2).first] = (*it2).second;
        }
    }
    save_auto();
}

void INI::merge_map(SectionMap & data2, const std::string & src_group,
                    const std::string & dst_group, bool overwrite)
{
    OptionMap & items = data2[src_group];
    OptionMap::const_iterator it;
    for (it = items.begin(); it != items.end(); it++) {
        if (!overwrite && has_item(dst_group, (*it).first))
            continue;
        data[dst_group][(*it).first] = (*it).second;
    }
    save_auto();
}

size_t INI::get_search_count()
{
    return search_results.size();
}

std::string INI::get_search_result_group(int index)
{
    return search_results[index].first;
}

std::string INI::get_item_part(const std::string & group,
                               const std::string & item, int index,
                               const std::string & def)
{
    if (index < 0)
        return def;
    std::string value = get_string(group, item, "");
    std::vector<std::string> elem;
    split_string(value, ',', elem);
    if (index >= (int)elem.size())
        return def;
    return elem[index];
}

INI::~INI()
{
    if (global_key.empty())
        return;
    global_data[global_key] = data;
}

boost::unordered_map<std::string, SectionMap> INI::global_data;

// StringTokenizer

StringTokenizer::StringTokenizer(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{
}

void StringTokenizer::split(const std::string & text,
                            const std::string & delims)
{
    elements.clear();
    split_string(text, delims, elements);
}

const std::string & StringTokenizer::get(int index)
{
    return elements[index];
}

// File

const std::string & File::get_appdata_directory()
{
    return platform_get_appdata_dir();
}

bool File::file_exists(const std::string & path)
{
    FSFile fp(convert_path(path).c_str(), "r");
    return fp.is_open();
}

bool File::name_exists(const std::string & path)
{
    // XXX also test directories
    FSFile fp(convert_path(path).c_str(), "r");
    return fp.is_open();
}

void File::delete_file(const std::string & path)
{
    if (!platform_remove_file(path))
        std::cout << "Could not remove " << path << std::endl;
}

// WindowControl

bool WindowControl::has_focus()
{
    return platform_has_focus();
}

bool WindowControl::is_maximized()
{
    return false;
}

void WindowControl::set_focus(bool value)
{
    platform_set_focus(value);
}

void WindowControl::set_x(int x)
{
    std::cout << "Set window x: " << x << std::endl;
}

void WindowControl::set_y(int y)
{
    std::cout << "Set window y: " << y << std::endl;
}

void WindowControl::set_width(int w)
{
    std::cout << "Set window width: " << w << std::endl;
}

void WindowControl::set_height(int h)
{
    std::cout << "Set window height: " << h << std::endl;
}

void WindowControl::maximize()
{
    std::cout << "Maximize window" << std::endl;
}

void WindowControl::restore()
{
    std::cout << "Restore window" << std::endl;
}

int WindowControl::get_width()
{
    int w, h;
    platform_get_size(&w, &h);
    return w;
}

int WindowControl::get_height()
{
    int w, h;
    platform_get_size(&w, &h);
    return h;
}

int WindowControl::get_screen_width()
{
    int w, h;
    platform_get_screen_size(&w, &h);
    return w;
}

int WindowControl::get_screen_height()
{
    int w, h;
    platform_get_screen_size(&w, &h);
    return h;
}

// Workspace

Workspace::Workspace(BaseStream & stream)
{
    stream >> name;
    unsigned int len;
    stream >> len;
    stream.read(data, len);
}

Workspace::Workspace(const std::string & name)
: name(name)
{
}

// BinaryArray

BinaryArray::BinaryArray(int x, int y, int type_id)
: FrameObject(x, y, type_id), current(NULL)
{

}

void BinaryArray::load_workspaces(const std::string & filename)
{
    FSFile fp(convert_path(filename).c_str(), "r");
    FileStream stream(fp);
    Workspace * workspace;
    while (!stream.at_end()) {
        workspace = new Workspace(stream);
        workspaces[workspace->name] = workspace;
    }
    fp.close();
    switch_workspace(current);
}

BinaryArray::~BinaryArray()
{
    WorkspaceMap::const_iterator it;
    for (it = workspaces.begin(); it != workspaces.end(); it++)
        delete it->second;
}

void BinaryArray::create_workspace(const std::string & name)
{
    if (workspaces.find(name) != workspaces.end())
        return;
    Workspace * workspace = new Workspace(name);
    workspaces[name] = workspace;
}

void BinaryArray::switch_workspace(const std::string & name)
{
    WorkspaceMap::const_iterator it = workspaces.find(name);
    if (it == workspaces.end())
        return;
    switch_workspace(it->second);
}

void BinaryArray::switch_workspace(Workspace * workspace)
{
    current = workspace;
}

bool BinaryArray::has_workspace(const std::string & name)
{
    return workspaces.count(name) > 0;
}

void BinaryArray::load_file(const std::string & filename)
{
    size_t size;
    char * data;
    read_file(convert_path(filename).c_str(), &data, &size);
    current->data.write(data, size);
    delete[] data;
}

std::string BinaryArray::read_string(int pos, size_t size)
{
    DataStream stream(current->data);
    stream.seek(pos);
    std::string v;
    stream.read_string(v, size);
    return v;
}

size_t BinaryArray::get_size()
{
    std::stringstream & oss = current->data;
    std::stringstream::pos_type current = oss.tellg();
    oss.seekg(0, std::ios::end);
    std::stringstream::pos_type offset = oss.tellg();
    oss.seekg(current);
    return (size_t)offset;
}

// BinaryObject

BinaryObject::BinaryObject(int x, int y, int type_id)
: FrameObject(x, y, type_id), data(NULL), size(0)
{

}

BinaryObject::~BinaryObject()
{
    if (data == NULL)
        return;
    free(data);
}

void BinaryObject::load_file(const std::string & filename)
{
    if (data != NULL)
        free(data);
    std::cout << "Load binary array: " << filename << std::endl;
    read_file_c(convert_path(filename).c_str(), &data, &size);
}

void BinaryObject::save_file(const std::string & filename)
{
    std::cout << "Save binary array: " << filename << std::endl;
    FSFile fp(convert_path(filename).c_str(), "w");
    fp.write(data, size);
    fp.close();
}

void BinaryObject::set_byte(unsigned char byte, size_t addr)
{
    ((unsigned char*)data)[addr] = byte;
}

void BinaryObject::resize(size_t v)
{
    size = v;
    data = (char*)realloc(data, v);
}

int BinaryObject::get_byte(size_t addr)
{
    return ((unsigned char*)data)[addr];
}

int BinaryObject::get_short(size_t addr)
{
    unsigned char a = ((unsigned char*)data)[addr];
    unsigned char b = ((unsigned char*)data)[addr+1];
    unsigned short v = a | (b << 8);
    return v;
}

// ArrayObject

ArrayObject::ArrayObject(int x, int y, int type_id)
: FrameObject(x, y, type_id), array(NULL), strings(NULL)
{

}

void ArrayObject::initialize(bool numeric, int offset, int x, int y, int z)
{
    this->offset = offset;
    is_numeric = numeric;
    x_size = x;
    y_size = y;
    z_size = z;
    clear();
}

double & ArrayObject::get_value(int x, int y)
{
    x -= offset;
    y -= offset;
    return array[x + y * x_size];
}

std::string & ArrayObject::get_string(int x)
{
    x -= offset;
    return strings[x];
}

void ArrayObject::set_value(double value, int x, int y)
{
    get_value(x, y) = value;
}

void ArrayObject::set_string(const std::string & value, int x)
{
    get_string(x) = value;
}

ArrayObject::~ArrayObject()
{
    if (is_numeric)
        delete[] array;
    else
        delete[] strings;
}

void ArrayObject::clear()
{
    if (is_numeric) {
        delete[] array;
        array = new double[x_size * y_size * z_size]();
    } else {
        delete[] strings;
        strings = new std::string[x_size * y_size * z_size];
    }
}

// LayerObject

int LayerObject::sort_index;
bool LayerObject::sort_reverse;
double LayerObject::def;

LayerObject::LayerObject(int x, int y, int type_id)
: FrameObject(x, y, type_id), current_layer(0)
{

}

void LayerObject::set_layer(int value)
{
    current_layer = value;
}

void LayerObject::hide_layer(int index)
{
    frame->layers[index]->visible = false;
}

void LayerObject::show_layer(int index)
{
    frame->layers[index]->visible = true;
}

void LayerObject::set_position(int index, int x, int y)
{
    frame->layers[index]->set_position(x, y);
}

void LayerObject::set_x(int index, int x)
{
    Layer * layer = frame->layers[index];
    layer->set_position(x, layer->y);
}

void LayerObject::set_y(int index, int y)
{
    Layer * layer = frame->layers[index];
    layer->set_position(layer->x, y);
}

void LayerObject::set_alpha_coefficient(int index, int alpha)
{
    std::cout << "Alpha set for layer not supported: " << alpha << std::endl;
}

double LayerObject::get_alterable(FrameObject * instance)
{
    if (instance->values == NULL)
        return def;
    return instance->values->get(sort_index);
}

bool LayerObject::sort_func(FrameObject * a, FrameObject * b)
{
    double value1 = get_alterable(a);
    double value2 = get_alterable(b);
    if (sort_reverse)
        return value1 < value2;
    else
        return value1 > value2;
}

void LayerObject::sort_alt_decreasing(int index, double def)
{
    sort_index = index;
    sort_reverse = true;
    this->def = def;
    FlatObjectList & instances = frame->layers[current_layer]->instances;
    std::sort(instances.begin(), instances.end(), sort_func);
}

// Viewport

Viewport::Viewport(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Viewport::~Viewport()
{
    glDeleteTextures(1, &texture);
}

void Viewport::set_source(int center_x, int center_y, int width, int height)
{
    this->center_x = center_x;
    this->center_y = center_y;
    src_width = width;
    src_height = height;
}

void Viewport::set_width(int w)
{
    width = w;
}

void Viewport::set_height(int h)
{
    height = h;
}

void Viewport::draw()
{
    int src_x1 = center_x - src_width / 2;
    int src_y1 = center_y - src_height / 2;
    int src_x2 = src_x1 + src_width;
    int src_y2 = src_y1 + src_height;
    glc_copy_color_buffer_rect(texture, src_x1, src_y1, src_x2, src_y2);
    int x2 = x + width;
    int y2 = y + height;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(back_texcoords[0], back_texcoords[1]);
    glVertex2i(x, y);
    glTexCoord2f(back_texcoords[2], back_texcoords[3]);
    glVertex2i(x2, y);
    glTexCoord2f(back_texcoords[4], back_texcoords[5]);
    glVertex2i(x2, y2);
    glTexCoord2f(back_texcoords[6], back_texcoords[7]);
    glVertex2i(x, y2);
    glEnd();
    glEnable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

// AdvancedDirection

AdvancedDirection::AdvancedDirection(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{
}

void AdvancedDirection::find_closest(ObjectList & instances, int x, int y)
{
    float lowest_dist;
    closest = NULL;
    for (ObjectIterator it(instances); !it.end(); it++) {
        FrameObject * instance = *it;
        float dist = get_distance(x, y, instance->x, instance->y);
        if (closest != NULL && dist > lowest_dist)
            continue;
        closest = instance;
        lowest_dist = dist;
    }
}

void AdvancedDirection::find_closest(QualifierList & instances, int x, int y)
{
    float lowest_dist;
    closest = NULL;
    for (QualifierIterator it(instances); !it.end(); it++) {
        FrameObject * instance = *it;
        float dist = get_distance(x, y, instance->x, instance->y);
        if (closest != NULL && dist > lowest_dist)
            continue;
        closest = instance;
        lowest_dist = dist;
    }
}

FixedValue AdvancedDirection::get_closest(int n)
{
    return closest->get_fixed();
}

float AdvancedDirection::get_object_angle(FrameObject * a, FrameObject * b)
{
    return -::get_angle(a->x, a->y, b->x, b->y);
}

// TextBlitter

TextBlitter::TextBlitter(int x, int y, int type_id)
: FrameObject(x, y, type_id), flash_interval(0.0f)
{
}

void TextBlitter::initialize(const std::string & map_string)
{
    for (int i = 0; i < 256; i++) {
        charmap[i] = -1;
    }

    for (unsigned int i = 0; i < map_string.size(); i++) {
        unsigned char c = (unsigned char)map_string[i];
        charmap[c] = i;
    }

    image->upload_texture();
}

void TextBlitter::set_text(const std::string & value)
{
    text = value;
}

void TextBlitter::update(float dt)
{
    update_flash(dt, flash_interval, flash_time);
}

void TextBlitter::flash(float value)
{
    flash_interval = value;
    flash_time = 0.0f;
}

void TextBlitter::draw()
{
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, image->tex);

    std::string::const_iterator it;
    int xx = x;
    int yy = y;

    if (alignment & ALIGN_HCENTER)
        xx += width / 2 - (text.size() * char_width) / 2;
    if (alignment & ALIGN_VCENTER)
        yy += height / 2 - char_height / 2;

    for (it = text.begin(); it != text.end(); it++) {
        unsigned char c = (unsigned char)(*it);
        int i = charmap[c];
        int img_x = (i * char_width) % image->width;
        int img_y = ((i * char_width) / image->width) * char_height;

        float t_x1 = float(img_x) / float(image->width);
        float t_x2 = float(img_x+char_width) / float(image->width);
        float t_y1 = float(img_y) / float(image->height);
        float t_y2 = float(img_y+char_height) / float(image->height);

        glBegin(GL_QUADS);
        glTexCoord2f(t_x1, t_y1);
        glVertex2i(xx, yy);
        glTexCoord2f(t_x2, t_y1);
        glVertex2i(xx + char_width, yy);
        glTexCoord2f(t_x2, t_y2);
        glVertex2i(xx + char_width, yy + char_height);
        glTexCoord2f(t_x1, t_y2);
        glVertex2i(xx, yy + char_height);
        glEnd();

        if (c == '\n') {
            xx = x;
            yy += char_height;
        } else
            xx += char_width;
    }

    glDisable(GL_TEXTURE_2D);
}

// PlatformObject

// XXX hack
static PlatformObject * last_instance = NULL;

PlatformObject::PlatformObject(int x, int y, int type_id)
: FrameObject(x, y, type_id), instance(NULL), paused(false),
  add_x_vel(0), add_y_vel(0), x_move_count(0), y_move_count(0), x_vel(0),
  y_vel(0), left(false), right(false), obstacle_collision(false),
  platform_collision(false), on_ground(false), through_collision_top(false),
  jump_through(false)
{
    // XXX hack
    // if (x == -424 && y == -352)
    //     last_instance = this;
}

void PlatformObject::update(float dt)
{
    bool l = left;
    bool r = right;
    left = right = false;

    if (instance == NULL || paused || instance->destroying)
        return;

    // XXX hack
    // if (this != last_instance && !last_instance->paused)
    //     return;

    if (r && !l)
        x_vel += x_accel;
    if (l && !r)
        x_vel -= x_accel;
    if (x_vel != 0 && ((!l && !r) || (l && r))) {
        x_vel -= (x_vel / get_abs(x_vel)) * x_decel;
        if (x_vel <= x_decel && x_vel >= 0 - x_decel)
            x_vel = 0;
    }

    x_vel = std::min(std::max(x_vel, -max_x_vel), max_x_vel);
    y_vel = std::min(std::max(y_vel + gravity, -max_y_vel), max_y_vel);
    int x_vel_2 = x_vel + add_x_vel;
    int y_vel_2 = y_vel + add_y_vel;
    int x_vel_sign = sign_int(x_vel_2);
    int y_vel_sign = sign_int(y_vel_2);
    x_move_count += get_abs(x_vel_2);
    y_move_count += get_abs(y_vel_2);

    bool overlaps;

    while (x_move_count > 100) {
        overlaps = overlaps_obstacle();
        if (!overlaps) {
            instance->set_x(instance->x + x_vel_sign);
            overlaps = overlaps_obstacle();
        }
        if (overlaps) {
            for (int i = 0; i < step_up; i++) {
                instance->set_y(instance->y - 1);
                overlaps = overlaps_obstacle();
                if (!overlaps)
                    break;
            }
            if (overlaps) {
                instance->set_position(
                    instance->x - x_vel_sign,
                    instance->y + step_up);
                x_vel = x_move_count = 0;
            }
        }
        x_move_count -= 100;
    }

    while (y_move_count > 100) {
        overlaps = overlaps_obstacle();
        if (!overlaps) {
            instance->set_y(instance->y + y_vel_sign);
            on_ground = false;
            overlaps = overlaps_obstacle();
        }
        if (overlaps) {
            instance->set_y(instance->y - y_vel_sign);
            if (y_vel_2 > 0)
                on_ground = true;
            y_vel = y_move_count = 0;
        }
        if (overlaps_platform() && y_vel_2 > 0) {
            if (through_collision_top) {
                instance->set_y(instance->y - 1);
                if (!overlaps_platform()) {
                    instance->set_y(instance->y - y_vel_sign);
                    y_vel = y_move_count = 0;
                    on_ground = true;
                }
                instance->set_y(instance->y + 1);
            } else {
                instance->set_y(instance->y - y_vel_sign);
                y_vel = y_move_count = 0;
                on_ground = true;
            }
        }
        y_move_count -= 100;
    }

    if (slope_correction > 0 && y_vel_2 >= 0) {
        bool tmp = false;
        for (int i = 0; i < slope_correction; i++) {
            instance->set_y(instance->y + 1);
            if (overlaps_obstacle()) {
                instance->set_y(instance->y - 1);
                on_ground = true;
                tmp = true;
                break;
            }
        }
        if (!tmp)
            instance->set_y(instance->y - slope_correction);
    }
}

bool PlatformObject::overlaps_obstacle()
{
    obstacle_collision = false;
    call_overlaps_obstacle();
    return obstacle_collision;
}

bool PlatformObject::overlaps_platform()
{
    platform_collision = false;
    call_overlaps_platform();
    return platform_collision;
}

void PlatformObject::set_object(FrameObject * instance)
{
    this->instance = instance;
}

bool PlatformObject::is_falling()
{
    return !on_ground && y_vel > 0;
}

bool PlatformObject::is_jumping()
{
    return !on_ground && y_vel <= 0;
}

bool PlatformObject::is_moving()
{
    return get_abs(x_vel) > 0;
}

void PlatformObject::jump_in_air()
{
    y_vel -= jump_hold_height;
}

void PlatformObject::jump()
{
    y_vel = 0 - jump_strength;
}

// ActivePicture

ActivePicture::ActivePicture(int x, int y, int type_id)
: FrameObject(x, y, type_id), image(NULL), horizontal_flip(false),
  scale_x(1.0), scale_y(1.0), angle(0.0), has_transparent_color(false)
{
    collision = new SpriteCollision(this);
}

ActivePicture::~ActivePicture()
{
    remove_image();
    delete collision;
}

void ActivePicture::remove_image()
{
    if (image == NULL)
        return;
    delete image;
    image = NULL;
}

void ActivePicture::load(const std::string & fn)
{
#if defined(CHOWDREN_IS_WIIU) || defined(CHOWDREN_EMULATE_WIIU)
    // small hack to load language-specific files for menu
    size_t dir_end = fn.find_last_of(PATH_SEP);
    size_t dir_start = fn.find_last_of(PATH_SEP, dir_end-1);
    std::string dir = fn.substr(dir_start+1, dir_end-dir_start-1);
    if (dir == "Menu") {
        std::string name = fn.substr(dir_end + 1);
        filename = convert_path(fn.substr(0, dir_end+1) +
                                platform_get_language() + "/" + name);

    } else
        filename = convert_path(fn);
#else
    filename = convert_path(fn);
#endif
    remove_image();
    ImageCache::const_iterator it = image_cache.find(filename);
    if (it == image_cache.end()) {
        Color * transparent = NULL;
        if (has_transparent_color)
            transparent = &transparent_color;
        cached_image = new Image(filename, 0, 0, 0, 0, transparent);
        if (!cached_image->is_valid()) {
            delete cached_image;
            cached_image = NULL;
        }
        image_cache[filename] = cached_image;
    } else {
        cached_image = it->second;
    }

    if (cached_image != NULL) {
        image = new Image(*cached_image);
        ((SpriteCollision*)collision)->set_image(image);
        image->hotspot_x = image->hotspot_y = 0;
    }
}

void ActivePicture::set_transparent_color(const Color & color)
{
    transparent_color = color;
    has_transparent_color = true;
}

void ActivePicture::set_hotspot(int x, int y)
{
    if (image == NULL)
        return;
    this->x += x - image->hotspot_x;
    this->y += y - image->hotspot_y;
    image->hotspot_x = x;
    image->hotspot_y = y;
}

void ActivePicture::set_hotspot_mul(float x, float y)
{
    if (image == NULL)
        return;
    set_hotspot(image->width * x, image->height * y);
}

void ActivePicture::flip_horizontal()
{
    horizontal_flip = !horizontal_flip;
}

void ActivePicture::set_scale(double value)
{
    ((SpriteCollision*)collision)->set_scale(value);
    scale_x = scale_y = value;
}

void ActivePicture::set_zoom(double value)
{
    set_scale(value / 100.0);
}

void ActivePicture::set_angle(double value, int quality)
{
    ((SpriteCollision*)collision)->set_angle(value);
    angle = value;
}

double ActivePicture::get_zoom_x()
{
    return scale_x * 100.0;
}

int ActivePicture::get_width()
{
    if (image == NULL)
        return 0;
    return image->width;
}

int ActivePicture::get_height()
{
    if (image == NULL)
        return 0;
    return image->height;
}

void ActivePicture::draw()
{
    if (image == NULL)
        return;
    blend_color.apply();
    draw_image(image, x, y, angle, scale_x, scale_y, horizontal_flip);
}

void ActivePicture::paste(int dest_x, int dest_y, int src_x, int src_y,
                          int src_width, int src_height, int collision_type)
{
    if (image == NULL) {
        std::cout << "Invalid image paste: " << filename << std::endl;
        return;
    }
    layer->paste(cached_image, dest_x, dest_y, src_x, src_y,
                 src_width, src_height, collision_type);
}

ImageCache ActivePicture::image_cache;

// ListObject

ListObject::ListObject(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{

}

void ListObject::load_file(const std::string & name)
{
    FSFile fp(name.c_str(), "r");
    if (!fp.is_open())
        return;
    std::string line;
    while (!fp.at_end()) {
        fp.read_line(line);
        add_line(line);
    }
}

void ListObject::add_line(const std::string & value)
{
    lines.push_back(value);
}

const std::string & ListObject::get_line(int i)
{
    return lines[i];
}

int ListObject::get_count()
{
    return lines.size();
}

// MathHelper

MathHelper math_helper;

// joystick

int remap_button(int n)
{
#ifdef CHOWDREN_SNES_CONTROLLER
    switch (n) {
        case CHOWDREN_BUTTON_X:
            return CHOWDREN_BUTTON_A;
        case CHOWDREN_BUTTON_Y:
            return CHOWDREN_BUTTON_X;
        case CHOWDREN_BUTTON_GUIDE:
            return CHOWDREN_BUTTON_RIGHTSHOULDER;
        case CHOWDREN_BUTTON_BACK:
            return CHOWDREN_BUTTON_LEFTSHOULDER;
        case CHOWDREN_BUTTON_LEFTSHOULDER:
            return CHOWDREN_BUTTON_START;
        case CHOWDREN_BUTTON_B:
            return n;
        default:
            return CHOWDREN_BUTTON_INVALID;
    }
#else
    return n;
#endif
}

int get_joystick_direction(int n)
{
    float x = get_joystick_axis(n, CHOWDREN_AXIS_LEFTX);
    float y = -get_joystick_axis(n, CHOWDREN_AXIS_LEFTY);
#ifdef CHOWDREN_IS_DESKTOP
    static const float threshold = 0.4f;
    int dir;
    // emulate Joystick 2 very closely
    if (get_abs(x) < threshold && get_abs(y) < threshold)
        dir = 8;
    else if (x > threshold) {
        if (y > threshold)
            dir = 1;
        else if (y < -threshold)
            dir = 7;
        else
            dir = 0;
    } else if (x < -threshold) {
        if (y > threshold)
            dir = 3;
        else if (y < -threshold)
            dir = 5;
        else
            dir = 4;
    } else {
        if (y > threshold)
            dir = 2;
        else
            dir = 6;
    }
    return dir;
#else
    static const float threshold = 0.35f;
    if (get_length(x, y) < threshold)
        return 8; // center
    else {
        return int_round(atan2d(y, x) / 45.0f) & 7;
    }
    return 8;
#endif
}

int get_joystick_direction_flags(int n)
{
    int dir = get_joystick_direction(n);
    switch (dir) {
        case 0:
            return 8; // 1000
        case 1:
            return 9; // 1001
        case 2:
            return 1; // 0001
        case 3:
            return 5; // 0101
        case 4:
            return 4; // 0100
        case 5:
            return 6; // 0110
        case 6:
            return 2; // 0010
        case 7:
            return 10; // 1010
        default:
            return 0;
    }
}

bool test_joystick_direction_flags(int n, int flags)
{
    int f = get_joystick_direction_flags(n);
    return (f & flags) == flags;
}

int get_joystick_dpad(int n)
{
    bool up = is_joystick_pressed(n, CHOWDREN_BUTTON_DPAD_UP);
    bool down = is_joystick_pressed(n, CHOWDREN_BUTTON_DPAD_DOWN);
    bool left = is_joystick_pressed(n, CHOWDREN_BUTTON_DPAD_LEFT);
    bool right = is_joystick_pressed(n, CHOWDREN_BUTTON_DPAD_RIGHT);
    int dir = get_movement_direction(up, down, left, right);
    if (dir == -1)
        dir = 8;
    else
        dir /= 4;
    return dir;
}

float get_joystick_dpad_degrees(int n)
{
    int dir = get_joystick_dpad(n);
    if (dir == 8)
        return -1;
    return dir * 45.0f;
}

float get_joystick_rt(int n)
{
    return get_joystick_axis(n, CHOWDREN_AXIS_TRIGGERRIGHT) * 100.0f;
}

float get_joystick_lt(int n)
{
    return get_joystick_axis(n, CHOWDREN_AXIS_TRIGGERLEFT) * 100.0f;
}

float get_joystick_x(int n)
{
    return get_joystick_axis(n, CHOWDREN_AXIS_LEFTX) * 1000.0f;
}

float get_joystick_y(int n)
{
    return get_joystick_axis(n, CHOWDREN_AXIS_LEFTY) * 1000.0f;
}

// for checking overlap

std::vector<int> int_temp;
