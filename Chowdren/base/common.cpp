#include "common.h"
#include "fileio.h"
#include <string>
#include "chowconfig.h"
#include "font.h"
#include <iterator>
#include <iomanip>
#include "md5.h"
#include "intern.cpp"

#ifdef CHOWDREN_USE_VALUEADD
#include "extra_keys.cpp"
#endif

std::string newline_character("\r\n");
std::string empty_string("");

static const char hex_characters[] = "0123456789abcdef";

std::string get_md5(const std::string & value)
{
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, &value[0], value.size());
    std::string res;
    res.resize(32);
    unsigned char digest[16];
    MD5_Final(digest, &ctx);

    for (int i = 0; i < 16; i++) {
        unsigned char b = digest[i];
        res[i*2] = hex_characters[(b >> 4) & 0xf];
        res[i*2+1] = hex_characters[b & 0xF];
    }

    return res;
}

void swap_position(const FlatObjectList & list)
{
    if (list.size() <= 1)
        return;
    int num1 = randrange(list.size());
    int num2;
    while (true) {
        num2 = randrange(list.size());
        if (num2 != num1)
            break;
    }

    FrameObject * a = list[num1];
    FrameObject * b = list[num2];

    int x1 = a->get_x();
    int y1 = a->get_y();
    int x2 = b->get_x();
    int y2 = b->get_y();

    a->set_global_position(x2, y2);
    b->set_global_position(x1, y1);
}

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
    for (it = items.begin(); it != items.end(); ++it) {
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
                     x, y, x+1, y+1)) {
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
                     x, y, x+1, y+1)) {
            delete item;
            it = col_items.erase(it);
        } else
            ++it;
    }
}

void Background::paste(Image * img, int dest_x, int dest_y,
                       int src_x, int src_y, int src_width, int src_height,
                       int collision_type, const Color & color)
{
    // collision types:
    // 0: not an obstacle
    // 1: obstacle
    // 4: no effect on collisions
    src_width = std::min<int>(img->width, src_x + src_width) - src_x;
    src_height = std::min<int>(img->height, src_y + src_height) - src_y;

    if (src_width <= 0 || src_height <= 0)
        return;

    if (collision_type == 1) {
        col_items.push_back(new BackgroundItem(img, dest_x, dest_y,
                                               src_x, src_y,
                                               src_width, src_height,
                                               collision_type, color));
#ifndef CHOWDREN_OBSTACLE_IMAGE
        return;
#endif
    }

    if (color.a == 0 || color.a == 1)
        return;

    items.push_back(new BackgroundItem(img, dest_x, dest_y,
                                       src_x, src_y,
                                       src_width, src_height,
                                       collision_type, color));
}

void Background::draw(int v[4])
{
    BackgroundItems::const_iterator it;
    for (it = items.begin(); it != items.end(); ++it) {
        BackgroundItem * item = *it;
        if (!collides(item->aabb, v))
            continue;
        item->draw();
    }
}

CollisionBase * Background::collide(CollisionBase * a)
{
    BackgroundItems::iterator it;
    for (it = col_items.begin(); it != col_items.end(); ++it) {
        BackgroundItem * item = *it;
        if (::collide(a, item))
            return item;
    }
    return NULL;
}

// Layer

static Layer default_layer(0, 1.0, 1.0, false, false, false);

Layer::Layer()
{
    reset();
}

Layer::Layer(int index, double scroll_x, double scroll_y, bool visible,
             bool wrap_x, bool wrap_y)
{
    init(index, scroll_x, scroll_y, visible, wrap_x, wrap_y);
}

Layer::Layer(const Layer & layer)
{
    // this should never be called, but need to make vector::resize happy
    reset();
}

Layer & Layer::operator=(const Layer & other)
{
    // this should never be called, but need to make vector::resize happy
    return *this;
}

void Layer::reset()
{
    x = y = 0;
    off_x = off_y = 0;
    back = NULL;

#ifdef CHOWDREN_IS_3DS
    depth = 0.0f;
#endif
}

void Layer::init(int index, double scroll_x, double scroll_y, bool visible,
                 bool wrap_x, bool wrap_y)
{
    reset();

#if defined(CHOWDREN_HAS_MRT)
    remote = CHOWDREN_TV_TARGET;
#endif

    this->index = index;
    this->scroll_x = scroll_x;
    this->scroll_y = scroll_y;
    this->visible = visible;
    this->wrap_x = wrap_x;
    this->wrap_y = wrap_y;

    scroll_active = scroll_x != 1.0 || scroll_y != 1.0;

    if (this == &default_layer)
        return;

    broadphase.init();
}

Layer::~Layer()
{
    delete back;

    // layers are in charge of deleting background instances
    FlatObjectList::const_iterator it;
    for (it = background_instances.begin(); it != background_instances.end();
         ++it) {
        FrameObject * obj = *it;
        obj->dealloc();
    }
}

void Layer::scroll(int off_x, int off_y, int dx, int dy)
{
    this->off_x = off_x;
    this->off_y = off_y;

    LayerInstances::iterator it;
    for (it = instances.begin(); it != instances.end(); ++it) {
        FrameObject * object = &*it;
        if (object->flags & SCROLL)
            continue;
        object->set_position(object->x + dx, object->y + dy);
    }

#ifdef CHOWDREN_LAYER_WRAP
    if (wrap_x) {
        dy = 0;
    } else if (wrap_y) {
        dx = 0;
    } else
        return;
    FlatObjectList::const_iterator it2;
    for (it2 = background_instances.begin(); it2 != background_instances.end();
         ++it2) {
        FrameObject * object = *it2;
        object->set_position(object->x + dx, object->y + dy);
        object->set_backdrop_offset(-dx, -dy);
    }
#endif
}

void Layer::set_position(int x, int y)
{
    int dx = x - this->x;
    int dy = y - this->y;

    this->x = x;
    this->y = y;
    LayerInstances::iterator it;

    for (it = instances.begin(); it != instances.end(); ++it) {
        FrameObject * object = &*it;
        if (object->flags & SCROLL)
            continue;
        object->set_position(object->x + dx, object->y + dy);
    }

    FlatObjectList::const_iterator it2;
    for (it2 = background_instances.begin(); it2 != background_instances.end();
         ++it2) {
        FrameObject * item = *it2;
#ifdef CHOWDREN_LAYER_WRAP
        if (wrap_x) {
            item->set_backdrop_offset(dx, 0);
            continue;
        }
        if (wrap_y) {
            item->set_backdrop_offset(0, dy);
            continue;
        }
#endif
        item->set_position(item->x + dx, item->y + dy);
    }
}

void Layer::add_background_object(FrameObject * instance)
{
    static int depth = 0;
    instance->depth = depth;
    depth++;
    // instance->depth = background_instances.size();
    background_instances.push_back(instance);
}

void Layer::remove_background_object(FrameObject * instance)
{
    FlatObjectList::iterator it;
    for (it = background_instances.begin(); it != background_instances.end();
         ++it) {
        if (*it != instance)
            continue;
        background_instances.erase(it);
        break;
    }
}

// means we can store about 20,000 objects in both directions
#define LAYER_DEPTH_SPACING 100000
#define LAYER_DEPTH_START 0x7FFFFFFF

inline unsigned int add_depth(unsigned int start, unsigned int add,
                              bool * reset)
{
    if (0xFFFFFFFF - start < add) {
        *reset = true;
        return 0;
    }
    return start + add;
}

inline unsigned int sub_depth(unsigned int start, unsigned int sub,
                              bool * reset)
{
    if (start < sub) {
        *reset = true;
        return 0;
    }
    return start - sub;
}

void Layer::add_object(FrameObject * instance)
{
    bool reset = false;
    if (instances.empty())
        instance->depth = LAYER_DEPTH_START;
    else
        instance->depth = add_depth(instances.back().depth,
                                    LAYER_DEPTH_SPACING,
                                    &reset);

    instances.push_back(*instance);

    if (reset) {
#ifndef NDEBUG
        std::cout << "Flush in add_object" << std::endl;
#endif
        reset_depth();
    }
}

void Layer::insert_object(FrameObject * instance, int index)
{
    bool reset = false;

    if (index == 0) {
        if (instances.empty())
            instance->depth = LAYER_DEPTH_START;
        else
            instance->depth = sub_depth(instances.front().depth,
                                        LAYER_DEPTH_SPACING,
                                        &reset);

        instances.push_front(*instance);

        if (reset) {
#ifndef NDEBUG
            std::cout << "Flush in insert_object (1)" << std::endl;
#endif
            reset_depth();
        }
        return;
    }
    LayerInstances::iterator it = instances.begin();
    std::advance(it, index - 1);
    unsigned int prev = it->depth;
    ++it;
    unsigned int next = it->depth;
    unsigned int new_depth = (prev + next) / 2;
    instance->depth = new_depth;
    instances.insert(it, *instance);

    if (new_depth == next || new_depth == prev) {
#ifndef NDEBUG
        std::cout << "Flush in insert_object (2)" << std::endl;
#endif
        reset_depth();
    }
}

void Layer::reset_depth()
{
    LayerInstances::iterator it;
    unsigned int i = LAYER_DEPTH_START;
    for (it = instances.begin(); it != instances.end(); ++it) {
        it->depth = i;
        i += LAYER_DEPTH_SPACING;
    }
}

void Layer::remove_object(FrameObject * instance)
{
    instances.erase(LayerInstances::s_iterator_to(*instance));
}

void Layer::set_level(FrameObject * instance, int new_index)
{
    if (instance->flags & BACKGROUND)
        return;
    remove_object(instance);
    if (new_index == -1 || new_index >= int(instances.size()))
        add_object(instance);
    else
        insert_object(instance, new_index);
}

int Layer::get_level(FrameObject * instance)
{
    LayerInstances::const_iterator it;
    int i = 0;
    for (it = instances.begin(); it != instances.end(); ++it) {
        if (&*it == instance)
            return i;
        i++;
    }
    return -1;
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
    CollisionBase * other;

    BackgroundCallback(CollisionBase * col)
    : col(col)
    {
    }

    bool on_callback(void * data)
    {
        FrameObject * obj = (FrameObject*)data;
        if (obj->id != BACKGROUND_TYPE)
            return true;
        if (obj->collision == NULL)
            return true;
        other = obj->collision;
        return !collide(col, other);
    }
};

CollisionBase * Layer::test_background_collision(CollisionBase * a)
{
    if (back != NULL) {
        CollisionBase * ret = back->collide(a);
        if (ret)
            return ret;
    }
    BackgroundCallback callback(a);
    if (!broadphase.query_static(a->aabb, callback))
        return callback.other;
    return NULL;
}

CollisionBase * Layer::test_background_collision(int x, int y)
{
    x -= off_x;
    y -= off_y;
    PointCollision col(x, y);
    return test_background_collision(&col);
}

void Layer::paste(Image * img, int dest_x, int dest_y,
                  int src_x, int src_y, int src_width, int src_height,
                  int collision_type, const Color & color)
{
    if (collision_type != 0 && collision_type != 1 && collision_type != 4) {
        std::cout << "Collision type " << collision_type << " not supported"
            << std::endl;
    }
    if (back == NULL)
        back = new Background;
    back->paste(img, dest_x, dest_y, src_x, src_y,
                src_width, src_height, collision_type, color);
}

struct DrawCallback
{
    FlatObjectList & list;
    int * aabb;

    DrawCallback(FlatObjectList & list, int v[4])
    : list(list), aabb(v)
    {
    }

    bool on_callback(void * data)
    {
        FrameObject * item = (FrameObject*)data;
        if (!(item->flags & VISIBLE) || item->flags & DESTROYING)
            return true;
        if (!collide_box(item, aabb))
            return true;
        list.push_back(item);
        return true;
    }
};

inline bool sort_depth_comp(FrameObject * obj1, FrameObject * obj2)
{
    if (obj1->flags & BACKGROUND && !(obj2->flags & BACKGROUND))
        return true;
    if (obj2->flags & BACKGROUND && !(obj1->flags & BACKGROUND))
        return false;
    return obj1->depth < obj2->depth;
}

inline void sort_depth(FlatObjectList & list)
{
    std::sort(list.begin(), list.end(), sort_depth_comp);
}

void Layer::draw(int off_x, int off_y)
{
    if (!visible)
        return;
#ifdef CHOWDREN_IS_3DS
    glc_set_global_depth(depth);
#endif

    // draw backgrounds
    int x1 = off_x * scroll_x;
    int y1 = off_y * scroll_y;
    int x2 = x1+WINDOW_WIDTH;
    int y2 = y1+WINDOW_HEIGHT;

    PROFILE_BEGIN(Layer_draw_instances);

    static FlatObjectList draw_list;
    draw_list.clear();
#ifdef CHOWDREN_USE_VIEWPORT
    int v[4];
    Viewport * view = Viewport::instance;
    if (view != NULL && index <= view->layer->index) {
        v[0] = x1 + view->center_x - view->src_width / 2;
        v[1] = y1 + view->center_y - view->src_height / 2;
        v[2] = v[0] + view->src_width;
        v[3] = v[1] + view->src_height;
    } else {
        v[0] = x1;
        v[1] = y1;
        v[2] = x2;
        v[3] = y2;
    }
#else
    int v[4] = {x1, y1, x2, y2};
#endif
    DrawCallback callback(draw_list, v);
    broadphase.query(v, callback);

    sort_depth(draw_list);

    FlatObjectList::const_iterator it;
    for (it = draw_list.begin(); it != draw_list.end(); ++it) {
        FrameObject * obj = *it;
        if (!(obj->flags & BACKGROUND))
            break;
        obj->draw();
    }

    PROFILE_BEGIN(Layer_draw_pasted);

    // draw pasted items
    if (back != NULL)
        back->draw(v);

    PROFILE_END();

    for (; it != draw_list.end(); ++it) {
        (*it)->draw();
    }

    PROFILE_END();
}

#if defined(CHOWDREN_HAS_MRT)
void Layer::set_remote(int value)
{
    remote = value;
}
#endif

// FrameData

FrameData::FrameData()
{
}

void FrameData::event_callback(int id)
{
}

void FrameData::on_start()
{
}

void FrameData::on_end()
{
}

void FrameData::on_app_end()
{
}

void FrameData::handle_events()
{
}

void FrameData::handle_pre_events()
{
}

// Frame

Frame::Frame()
: off_x(0), off_y(0), new_off_x(0), new_off_y(0), has_quit(false),
  last_key(-1), next_frame(-1), loop_count(0), frame_time(0.0),
  index(-1)
{
}

void Frame::pause()
{
}

void Frame::restart()
{
    next_frame = -2;
}

void Frame::set_timer(double value)
{
    frame_time = value;
}

void Frame::set_lives(int value)
{
    manager.lives = value;
    if (value != 0)
        return;
    manager.player_died = true;
}

void Frame::set_score(int value)
{
    manager.score = value;
}

void Frame::set_display_center(int x, int y)
{
    /* note: there is a bug in MMF where you can only set the center if it
       is not equivalent to the old one, even if a new position was set.
       we need to emulate that bug here. */
    if (x != -1) {
        x = int_max(0, x - WINDOW_WIDTH / 2);
        x = int_min(x, virtual_width - WINDOW_WIDTH);
        if (x != off_x)
            new_off_x = x;
    }
    if (y != -1) {
        y = int_max(0, y - WINDOW_HEIGHT / 2);
        y = int_min(y, virtual_height - WINDOW_HEIGHT);
        if (y != off_y)
            new_off_y = y;
    }
}

void Frame::update_display_center()
{
    if (off_x == new_off_x && off_y == new_off_y)
        return;

    vector<Layer>::iterator it;
    for (it = layers.begin(); it != layers.end(); ++it) {
        Layer & layer = *it;
        int x1 = off_x * layer.scroll_x;
        int y1 = off_y * layer.scroll_y;
        int x2 = new_off_x * layer.scroll_x;
        int y2 = new_off_y * layer.scroll_y;
        int layer_off_x = new_off_x - x2;
        int layer_off_y = new_off_y - y2;
        layer.scroll(layer_off_x, layer_off_y, x2 - x1, y2 - y1);
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

void Frame::set_width(int width, bool adjust)
{
    std::cout << "Set frame width: " << width << " " << adjust << std::endl;
}

void Frame::set_height(int height, bool adjust)
{
    std::cout << "Set frame height: " << height << " " << adjust << std::endl;
}

void Frame::set_background_color(const Color & color)
{
    background_color = color;
}

void Frame::get_mouse_pos(int * x, int * y)
{
    *x = manager.mouse_x + off_x;
    *y = manager.mouse_y + off_y;
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

CollisionBase * Frame::test_background_collision(int x, int y)
{
    if (x < 0 || y < 0 || x > width || y > height)
        return NULL;
    vector<Layer>::iterator it;
    for (it = layers.begin(); it != layers.end(); ++it) {
        CollisionBase * ret = (*it).test_background_collision(x, y);
        if (ret == NULL)
            continue;
        return ret;
    }
    return NULL;
}

int Frame::get_background_mask(int x, int y)
{
    CollisionBase * other = test_background_collision(x, y);
    if (other == NULL)
        return 0;
    if (other->flags & LADDER_OBSTACLE)
        return 2;
    else
        return 1;
}

bool Frame::test_obstacle(int x, int y)
{
    return get_background_mask(x, y) == 1;
}

bool Frame::test_ladder(int x, int y)
{
    return get_background_mask(x, y) == 2;
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
    platform_set_vsync(value);
}

#define INSTANCE_MAP instances

int Frame::get_instance_count()
{
    int size = 0;
    ObjectList::iterator it;
    for (unsigned int i = 0; i < MAX_OBJECT_ID; i++) {
        ObjectList & list = INSTANCE_MAP.items[i];
        size += list.size();
    }
    return size;
}

void Frame::clean_instances()
{
    FlatObjectList::const_iterator it;
    for (it = destroyed_instances.begin(); it != destroyed_instances.end();
         ++it) {
        FrameObject * instance = *it;
        INSTANCE_MAP.items[instance->id].remove(instance);
        if (instance->flags & BACKGROUND)
            instance->layer->remove_background_object(instance);
        else
            instance->layer->remove_object(instance);
        instance->dealloc();
    }
    destroyed_instances.clear();
}

void Frame::update_objects()
{
}

void Frame::load_static_images()
{
}

bool Frame::update()
{
    frame_time += manager.dt;

    if (timer_base == 0) {
        timer_mul = 1.0f;
    } else {
        timer_mul = manager.dt * timer_base;
    }

    PROFILE_BEGIN(handle_pre_events);
    data->handle_pre_events();
    PROFILE_END();

    PROFILE_BEGIN(frame_update_objects);
    update_objects();
    PROFILE_END();

    if (loop_count > 0) {
        PROFILE_BEGIN(clean_instances);
        clean_instances();
        PROFILE_END();
    }

    PROFILE_BEGIN(handle_events);
    data->handle_events();
    update_display_center();
    PROFILE_END();

    manager.player_died = false;
    last_key = -1;
    loop_count++;

    return !has_quit;
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
#ifdef CHOWDREN_HAS_MRT
    if (remote == CHOWDREN_REMOTE_TARGET)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    else
#endif
    {
        background_color.apply_clear_color();
    }
#ifdef CHOWDREN_HAS_MRT
    if (remote != CHOWDREN_REMOTE_ONLY)
#endif
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    PROFILE_END();

    vector<Layer>::iterator it;
    for (it = layers.begin(); it != layers.end(); ++it) {
        Layer & layer = *it;

#ifdef CHOWDREN_HAS_MRT
        if (remote == CHOWDREN_HYBRID_TARGET) {
            if (layer.remote == CHOWDREN_REMOTE_TARGET)
                continue;
        } else if (remote == CHOWDREN_REMOTE_TARGET) {
            if (layer.remote == CHOWDREN_TV_TARGET)
                continue;
        } else if (remote == CHOWDREN_TV_TARGET) {
            if (layer.remote != CHOWDREN_TV_TARGET)
                continue;
        } else if (remote == CHOWDREN_REMOTE_ONLY) {
            if (layer.remote != CHOWDREN_REMOTE_TARGET)
                continue;
        }
#endif

        glLoadIdentity();
        glTranslatef(-floor(off_x * layer.scroll_x),
                     -floor(off_y * layer.scroll_y), 0.0);
        layer.draw(off_x, off_y);
    }

// #ifdef CHOWDREN_USE_BOX2D
//     Box2D::draw_debug();
// #endif
}

void setup_default_instance(FrameObject * obj)
{
    obj->layer = &default_layer;
    obj->width = obj->height = 0;
}

FrameObject * Frame::add_object(FrameObject * instance, Layer * layer)
{
    instance->frame = this;
    instance->layer = layer;
    INSTANCE_MAP.items[instance->id].add(instance);
    layer->add_object(instance);
    if (instance->collision) {
        instance->collision->update_aabb();
        instance->collision->create_proxy();
    }
    return instance;
}

FrameObject * Frame::add_object(FrameObject * instance, int layer_index)
{
    layer_index = int_max(0, int_min(layer_index, layers.size() - 1));
    return add_object(instance, &layers[layer_index]);
}

void Frame::add_background_object(FrameObject * instance, int layer_index)
{
    instance->frame = this;
    Layer * layer = &layers[layer_index];
    instance->layer = layer;
    if (instance->id != BACKGROUND_TYPE)
        INSTANCE_MAP.items[instance->id].add(instance);
    layer->add_background_object(instance);
    if (instance->collision) {
        instance->collision->update_aabb();
        instance->collision->create_static_proxy();
    } else {
        int bb[4] = {instance->x,
                     instance->y,
                     instance->x + instance->width,
                     instance->y + instance->height};
        layer->broadphase.add(instance, bb);
    }
}

void Frame::set_object_layer(FrameObject * instance, int new_layer)
{
    if (instance->flags & BACKGROUND) {
        std::cout << "Cannot move background object layer" << std::endl;
        return;
    }
    new_layer = clamp(new_layer, 0, layers.size()-1);
    Layer * layer = &layers[new_layer];
    if (layer == instance->layer)
        return;
    instance->layer->remove_object(instance);
    if (instance->collision)
        instance->collision->remove_proxy();
    layer->add_object(instance);
    int x = instance->get_x();
    int y = instance->get_y();
    instance->layer = layer;
    instance->set_global_position(x, y);
    if (instance->collision)
        instance->collision->create_proxy();
}

int Frame::get_loop_index(const std::string & name)
{
    return *((*loops)[name].index);
}

void Frame::reset()
{
    ObjectList::iterator it;
    for (unsigned int i = 0; i < MAX_OBJECT_ID; i++) {
        ObjectList & list = INSTANCE_MAP.items[i];
        for (it = list.begin(); it != list.end(); ++it) {
            if (it->obj->flags & BACKGROUND)
                continue;
            it->obj->dealloc();
        }
    }

    layers.clear();
    INSTANCE_MAP.clear();
    destroyed_instances.clear();
    next_frame = -1;
    loop_count = 0;
    off_x = new_off_x = 0;
    off_y = new_off_y = 0;
    frame_time = 0.0;
}

// FrameObject

FrameObject::FrameObject(int x, int y, int type_id)
: x(x), y(y), id(type_id), flags(SCROLL | VISIBLE), shader(NULL),
  alterables(NULL), shader_parameters(NULL), direction(0),
  movement(NULL), movements(NULL), movement_count(0), collision(NULL)
{
#ifdef CHOWDREN_USE_BOX2D
    body = -1;
#endif

#ifdef CHOWDREN_USE_VALUEADD
    extra_alterables = NULL;
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
    if (!(flags & GLOBAL))
        Alterables::destroy(alterables);
#ifdef CHOWDREN_USE_VALUEADD
    delete extra_alterables;
#endif
}

void FrameObject::draw_image(Image * img, int x, int y, float angle,
                             float x_scale, float y_scale, bool flip_x,
                             bool flip_y)
{
    GLuint back_tex = 0;
    bool has_tex_param = false;

    if (shader != NULL) {
        shader->begin(this, img);
        back_tex = shader->get_background_texture();
        has_tex_param = shader->has_texture_param();
    }

    img->draw(x, y, angle, x_scale, y_scale, flip_x, flip_y, back_tex,
              has_tex_param);

    if (shader != NULL)
        shader->end(this);
}

void FrameObject::begin_draw(int width, int height)
{
    if (shader != NULL)
        shader->begin(this, width, height);
}

void FrameObject::begin_draw()
{
    begin_draw(width, height);
}

void FrameObject::end_draw()
{
    if (shader != NULL)
        shader->end(this);
}

void FrameObject::set_position(int new_x, int new_y)
{
    if (new_x == x && new_y == y)
        return;
    if (collision == NULL) {
        x = new_x;
        y = new_y;
        return;
    }
    int dx = new_x - x;
    int dy = new_y - y;
    x = new_x;
    y = new_y;
    collision->aabb[0] += dx;
    collision->aabb[1] += dy;
    collision->aabb[2] += dx;
    collision->aabb[3] += dy;
    collision->update_proxy();
}

void FrameObject::set_global_position(int x, int y)
{
    set_position(x - layer->off_x, y - layer->off_y);
}

void FrameObject::set_x(int new_x)
{
    new_x -= layer->off_x;
    if (x == new_x)
        return;
    if (collision == NULL) {
        x = new_x;
        return;
    }
    int d = new_x - x;
    x = new_x;
    collision->aabb[0] += d;
    collision->aabb[2] += d;
    collision->update_proxy();
}

void FrameObject::set_y(int new_y)
{
    new_y -= layer->off_y;
    if (y == new_y)
        return;
    if (collision == NULL) {
        y = new_y;
        return;
    }
    int d = new_y - y;
    y = new_y;
    collision->aabb[1] += d;
    collision->aabb[3] += d;
    collision->update_proxy();
}

void FrameObject::create_alterables()
{
    alterables = Alterables::create();
}

void FrameObject::set_visible(bool value)
{
    flash(0);

    if (value)
        flags |= VISIBLE;
    else
        flags &= ~VISIBLE;
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
    if (flags & DESTROYING)
        return false;
    int x, y;
    frame->get_mouse_pos(&x, &y);
    x -= layer->off_x;
    y -= layer->off_y;
    PointCollision col1(x, y);
    return collide(&col1, collision);
}

bool FrameObject::overlaps(FrameObject * other)
{
    if (flags & INACTIVE || other->flags & INACTIVE)
        return false;
    if (other == this)
        return false;
    if (collision->type == NONE_COLLISION)
        return false;
    if (other->collision->type == NONE_COLLISION)
        return false;
    if (other->layer != layer)
        return false;
    return collide(collision, other->collision);
}

struct BackgroundOverlapCallback
{
    CollisionBase * collision;

    BackgroundOverlapCallback(CollisionBase * collision)
    : collision(collision)
    {
    }

    bool on_callback(void * data)
    {
        FrameObject * obj = (FrameObject*)data;
        if (obj->id != BACKGROUND_TYPE)
            return true;
        CollisionBase * other = (CollisionBase*)obj->collision;
        if (other == NULL)
            return true;
        if (other->flags & LADDER_OBSTACLE)
            return true;
        if (!collide(collision, other))
            return true;
        return false;
    }
};

bool FrameObject::overlaps_background()
{
    if (flags & DESTROYING || collision == NULL)
        return false;
    if (layer->back != NULL && layer->back->collide(collision))
        return true;
    BackgroundOverlapCallback callback(collision);
    if (!layer->broadphase.query_static(collision->proxy, callback))
        return true;
    return false;
}

bool FrameObject::overlaps_background_save()
{
    bool ret = overlaps_background();
    if (ret && movement != NULL) {
        movement->set_background_collision();
    }
    return ret;
}

bool FrameObject::outside_playfield()
{
    int * box = collision->aabb;
    int x1 = box[0] + layer->off_x;
    int y1 = box[1] + layer->off_y;
    int x2 = box[2] + layer->off_x;
    int y2 = box[3] + layer->off_y;
    return x1 > frame->width || y1 > frame->height || x2 < 0 || y2 < 0;
}

bool FrameObject::is_near_border(int border)
{
    int * box = collision->aabb;
    int off_x = layer->off_x;
    int off_y = layer->off_y;

    if (box[0] + off_x <= frame->frame_left() + border)
        return true;

    if (box[2] + off_x >= frame->frame_right() - border)
        return true;

    if (box[1] + off_y <= frame->frame_top() + border)
        return true;

    if (box[3] + off_y >= frame->frame_bottom() - border)
        return true;

    return false;
}

int FrameObject::get_box_index(int index)
{
    int ret;
    if (collision == NULL) {
        switch (index) {
            case 0:
                ret = x;
                break;
            case 1:
                ret = y;
                break;
            case 2:
                ret = x + width;
                break;
            case 3:
                ret = y + height;
                break;
        }
    } else
        ret = collision->aabb[index];
    if (index == 0 || index == 2)
        ret += layer->off_x;
    else
        ret += layer->off_y;
    return ret;
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

void FrameObject::set_shader_parameter(const std::string & name, Image & img)
{
    img.upload_texture();
    set_shader_parameter(name, (double)img.tex);
}

void FrameObject::set_shader_parameter(const std::string & name,
                                       const std::string & path)
{
    Image * img = get_image_cache(path, 0, 0, 0, 0, TransparentColor());
    set_shader_parameter(name, *img);
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
    if (flags & DESTROYING)
        return;
    flags |= DESTROYING;
    if (collision != NULL)
        collision->type = NONE_COLLISION;
    frame->destroyed_instances.push_back(this);
}

void FrameObject::set_level(int index)
{
    index = int_max(0, index);
    layer->set_level(this, index);
}

int FrameObject::get_level()
{
    return depth;
}

void FrameObject::move_back()
{
    layer->set_level(this, 0);
}

void FrameObject::move_front()
{
    layer->set_level(this, -1);
}

inline unsigned int get_next_depth(unsigned int prev, unsigned int next,
                                   bool * reset)
{
    unsigned int d = next - prev;
    unsigned int v = LAYER_DEPTH_SPACING / 10;
    while (v >= d && v != 0)
        v /= 10;
    if (v == 0)
        *reset = true;
    return v;
}

void FrameObject::move_relative(FrameObject * other, int disp)
{
    // XXX not 100% correct behaviour, but good enough for our purposes
    if (other == NULL)
        return;
    if (other->layer != layer)
        return;
    LayerInstances::iterator it = LayerInstances::s_iterator_to(*other);
    if (disp < 0) {
        while (disp < 0 && it != layer->instances.begin()) {
            it--;
            disp++;
        }
        move_back(&*it);
    } else {
        while (disp > 0) {
            it++;
            disp--;
            if (it != layer->instances.end())
                continue;
            it--;
            break;
        }
        move_front(&*it);
    }
}

void FrameObject::move_back(FrameObject * other)
{
    if (other == NULL)
        return;
    if (other->layer != layer)
        return;
    if (depth <= other->depth)
        return;

    LayerInstances::iterator it = LayerInstances::s_iterator_to(*other);
    unsigned int next = it->depth;

    bool reset = false;
    if (it == layer->instances.begin()) {
        depth = sub_depth(next, LAYER_DEPTH_SPACING, &reset);
    } else {
        unsigned int prev = (--LayerInstances::s_iterator_to(*other))->depth;
        depth = prev + get_next_depth(prev, next, &reset);
    }

    layer->instances.erase(LayerInstances::s_iterator_to(*this));
    layer->instances.insert(it, *this);

    if (reset) {
#ifndef NDEBUG
        std::cout << "Flush in move_back" << std::endl;
#endif
        layer->reset_depth();
    }
}

void FrameObject::move_front(FrameObject * other)
{
    if (other == NULL)
        return;
    if (other->layer != layer)
        return;
    if (depth >= other->depth)
        return;

    LayerInstances::iterator it = LayerInstances::s_iterator_to(*other);
    unsigned int prev = it->depth;
    ++it;

    bool reset = false;
    if (it == layer->instances.end()) {
        depth = add_depth(prev, LAYER_DEPTH_SPACING, &reset);
    } else {
        unsigned int next = it->depth;
        depth = next - get_next_depth(prev, next, &reset);

        if (reset) {
            std::cout << "move_front flush: " << next << " " << prev << std::endl;
        }
    }

    layer->instances.erase(LayerInstances::s_iterator_to(*this));
    layer->instances.insert(it, *this);

    if (reset) {
#ifndef NDEBUG
        std::cout << "Flush in move_front: " << depth << std::endl;
#endif
        layer->reset_depth();
    }
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

void FrameObject::advance_movement(int dir)
{
    set_movement(movement->index + dir);
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
    if (movement == NULL) {
        movement = new StaticMovement(this);
        movement->index = 0;
    }
    return movement;
}

int FrameObject::get_action_x()
{
    return get_x();
}

int FrameObject::get_action_y()
{
    return get_y();
}

void FrameObject::shoot(FrameObject * other, int speed, int direction)
{
    if (direction == -1)
        direction = this->direction;
    other->set_global_position(get_action_x(), get_action_y());
    other->set_direction(direction);
    delete other->movement;
    other->movement = new ShootMovement(other);
    other->movement->set_max_speed(speed);
    other->movement->start();
}

float FrameObject::get_angle()
{
    return 0.0f;
}

void FrameObject::set_angle(float angle, int quality)
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

void FrameObject::rotate_toward(int dir)
{
    dir = dir % 32;
    int cc = dir - direction;
    if (cc < 0)
        cc += 32;
    int cl = direction - dir;
    if (cl < 0)
        cl += 32;
    int angle;
    if (cc < cl)
        angle = cc;
    else
        angle = cl;
    // angle to turn, default to 1
    if (angle > 1)
        angle = 1;
    if (cl < cc)
        angle = -angle;
    direction += angle;
    if (direction >= 32)
        direction -= 32;
    if (direction < 0)
        direction += 32;
    set_direction(direction);
}

bool FrameObject::test_direction(int value)
{
    return get_direction() == value;
}

bool FrameObject::test_directions(int value)
{
    int direction = get_direction();
    return ((value >> direction) & 1) != 0;
}

void FrameObject::update_flash(float interval, float & t)
{
    if (interval == 0.0f)
        return;
    t += manager.dt;
    if (t < interval)
        return;
    t = 0.0f;
    flags ^= VISIBLE;
}

void FrameObject::set_animation(int value)
{
}

void FrameObject::set_backdrop_offset(int dx, int dy)
{
}

void FrameObject::get_screen_aabb(int box[4])
{
    int off_x = layer->off_x - frame->off_x;
    int off_y = layer->off_y - frame->off_y;
    int * aabb = collision->aabb;
    box[0] = aabb[0] + off_x;
    box[1] = aabb[1] + off_y;
    box[2] = aabb[2] + off_x;
    box[3] = aabb[3] + off_y;
}

#define INACTIVE_X 64
#define INACTIVE_Y 16

void FrameObject::update_inactive()
{
    flags &= ~INACTIVE;

    int box[4];
    get_screen_aabb(box);
    if (box[0] > WINDOW_WIDTH + INACTIVE_X ||
        box[1] > WINDOW_HEIGHT + INACTIVE_Y ||
        box[2] < -INACTIVE_X || box[3] < -INACTIVE_Y)
    {
        flags |= INACTIVE;
    }
}

// FixedValue

FixedValue::FixedValue(FrameObject * object)
: object(object)
{

}

FixedValue::operator double() const
{
    double v2 = 0.0;
    memcpy(&v2, &object, sizeof(FrameObject*));
    return v2;
}

FixedValue::operator std::string() const
{
#ifdef CHOWDREN_PERSISTENT_FIXED_STRING
    return number_to_string(object->x) + "&" + number_to_string(object->y);
#else
    intptr_t val = intptr_t(object);
    return number_to_string(val);
#endif
}

FixedValue::operator FrameObject*() const
{
    return object;
}

#ifdef CHOWDREN_USE_DYNAMIC_NUMBER
FixedValue::operator DynamicNumber() const
{
    double v = double(*this);
    return DynamicNumber(v);
}
#endif

// XXX move everything to 'objects' directory

// Text

FontList fonts;
static bool has_fonts = false;
static FTTextureFont * big_font = NULL;

bool init_font()
{
    static bool initialized = false;
    if (initialized)
        return has_fonts;

    // default font, could be set already
    has_fonts = load_fonts(fonts);

    FontList::const_iterator it;
    for (it = fonts.begin(); it != fonts.end(); ++it) {
        FTTextureFont * font = *it;
        if (big_font != NULL && big_font->size > font->size)
            continue;
        big_font = font;
    }

    initialized = true;
    return has_fonts;
}

FTTextureFont * get_font(int size)
{
    init_font();

    FTTextureFont * picked = NULL;
    int diff = 0;
    FontList::const_iterator it;

    for (it = fonts.begin(); it != fonts.end(); ++it) {
        FTTextureFont * font = *it;
        int new_diff = get_abs(font->size - size);

        if (picked == NULL || new_diff < diff) {
            picked = font;
            diff = new_diff;
        }
    }

    return picked;
}

void draw_gradient(int x1, int y1, int x2, int y2, int gradient_type,
                   Color & color, Color & color2, int alpha)
{
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    switch (gradient_type) {
        case NONE_GRADIENT:
            glColor4ub(color.r, color.g, color.b, alpha);
            glVertex2f(x1, y1);
            glVertex2f(x2, y1);
            glVertex2f(x2, y2);
            glVertex2f(x1, y2);
            break;
        case VERTICAL_GRADIENT:
            glColor4ub(color.r, color.g, color.b, alpha);
            glVertex2f(x1, y1);
            glVertex2f(x2, y1);
            glColor4ub(color2.r, color2.g, color2.b, alpha);
            glVertex2f(x2, y2);
            glVertex2f(x1, y2);
            break;
        case HORIZONTAL_GRADIENT:
            glColor4ub(color.r, color.g, color.b, alpha);
            glVertex2f(x1, y2);
            glVertex2f(x1, y1);
            glColor4ub(color2.r, color2.g, color2.b, alpha);
            glVertex2f(x2, y1);
            glVertex2f(x2, y2);
            break;
    }
    glEnd();
}

// File

const std::string & File::get_appdata_directory()
{
    return platform_get_appdata_dir();
}

void File::create_directory(const std::string & path)
{
    platform_create_directories(path);
}

bool File::file_exists(const std::string & path)
{
    return platform_is_file(path);
}

bool File::file_readable(const std::string & path)
{
    FSFile fp(convert_path(path).c_str(), "r");
    return fp.is_open();
}

bool File::name_exists(const std::string & path)
{
    return platform_path_exists(path);
}

bool File::directory_exists(const std::string & path)
{
    return platform_is_directory(path);
}

void File::delete_file(const std::string & path)
{
    if (platform_remove_file(path))
        return;
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
        return int_round(atan2_deg(y, x) / 45.0f) & 7;
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

float get_joystick_dummy(int n)
{
    return 0.0f;
}

float get_joystick_dpad_degrees(int n)
{
    int dir = get_joystick_dpad(n);
    if (dir == 8)
        return -1;
    return dir * 45.0f;
}

float get_joystick_degrees(int n)
{
    int dir = get_joystick_direction(n);
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

vector<int> int_temp;
