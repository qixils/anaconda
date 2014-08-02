#include "common.h"
#include "filecommon.h"
#include <string>
#include "chowconfig.h"
#include "font.h"
#include <iterator>
#include <iomanip>
#include "md5.h"
#include "strings.cpp"

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

bool Background::collide(CollisionBase * a)
{
    BackgroundItems::iterator it;
    for (it = col_items.begin(); it != col_items.end(); it++) {
        BackgroundItem * item = *it;
        if (::collide(a, item))
            return true;
    }
    return false;
}

// Layer

Layer::Layer()
{
    reset();
}

Layer::Layer(int index, double scroll_x, double scroll_y, bool visible,
             bool wrap_x, bool wrap_y)
{
    init(index, scroll_x, scroll_y, visible, wrap_x, wrap_y);
}

void Layer::reset()
{
    x = y = 0;
    off_x = off_y = 0;
    order_changed = false;
    back = NULL;
}

void Layer::init(int index, double scroll_x, double scroll_y, bool visible,
                 bool wrap_x, bool wrap_y)
{
    reset();

#if defined(CHOWDREN_IS_WIIU) || defined(CHOWDREN_EMULATE_WIIU)
    remote = CHOWDREN_TV_TARGET;
#endif

    this->index = index;
    this->scroll_x = scroll_x;
    this->scroll_y = scroll_y;
    this->visible = visible;
    this->wrap_x = wrap_x;
    this->wrap_y = wrap_y;

    scroll_active = scroll_x != 1.0 || scroll_y != 1.0;
}

Layer::~Layer()
{
    delete back;

    // layers are in charge of deleting background instances
    FlatObjectList::const_iterator it;
    for (it = background_instances.begin(); it != background_instances.end();
         it++) {
        FrameObject * obj = *it;
        delete obj;
    }
}

void Layer::scroll(int off_x, int off_y, int dx, int dy)
{
    this->off_x = off_x;
    this->off_y = off_y;

    LayerInstances::const_iterator it;
    for (it = instances.begin(); it != instances.end(); it++) {
        FrameObject * object = *it;
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
         it2++) {
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
    LayerInstances::const_iterator it;

    for (it = instances.begin(); it != instances.end(); it++) {
        FrameObject * object = *it;
        if (object->flags & SCROLL)
            continue;
        object->set_position(object->x + dx, object->y + dy);
    }

    FlatObjectList::const_iterator it2;
    for (it2 = background_instances.begin(); it2 != background_instances.end();
         it2++) {
        FrameObject * item = *it2;
#ifdef CHOWDREN_LAYER_WRAP
        if (wrap_x) {
            item->set_backdrop_offset(dx, 0);
            continue;
        } if (wrap_y) {
            item->set_backdrop_offset(0, dy);
            continue;
        }
#endif
        item->set_position(item->x + dx, item->y + dy);
    }
}

void Layer::add_background_object(FrameObject * instance)
{
    instance->depth = background_instances.size();
    background_instances.push_back(instance);
}

void Layer::add_object(FrameObject * instance)
{
    instance->depth = instances.size();
    instances.push_back(instance);
}

void Layer::insert_object(FrameObject * instance, int index)
{
    order_changed = true;
    if (index == 0) {
        instances.push_front(instance);
        return;
    }
    LayerInstances::iterator it = instances.begin();
    std::advance(it, index);
    instances.insert(it, instance);
}

void Layer::remove_object(FrameObject * instance)
{
    order_changed = true;
    LayerInstances::iterator it;
    for (it = instances.begin(); it != instances.end(); it++) {
        if (*it != instance)
            continue;
        instances.erase(it);
        break;
    }
}

void Layer::set_level(int old_index, int new_index)
{
    LayerInstances::iterator it = instances.begin();
    std::advance(it, old_index);
    FrameObject * instance = *it;
    instances.erase(it);
    if (new_index == -1 || new_index >= int(instances.size()))
        add_object(instance);
    else
        insert_object(instance, new_index);
}

void Layer::set_level(FrameObject * instance, int new_index)
{
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
    for (it = instances.begin(); it != instances.end(); it++) {
        if (*it == instance)
            return i;
        i++;
    }
    return -1;
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
        FrameObject * obj = (FrameObject*)data;
        if (obj->id != BACKGROUND_TYPE)
            return true;
        if (obj->collision == NULL)
            return true;
        return !collide(col, obj->collision);
    }
};

bool Layer::test_background_collision(CollisionBase * a)
{
    if (back != NULL && back->collide(a))
        return true;
    BackgroundCallback callback(a);
    if (!broadphase.query(a->aabb, callback))
        return true;
    return false;
}

bool Layer::test_background_collision(int x, int y)
{
    x -= off_x;
    y -= off_y;
    PointCollision col(x, y);
    return test_background_collision(&col);
}

void Layer::paste(Image * img, int dest_x, int dest_y,
                  int src_x, int src_y, int src_width, int src_height,
                  int collision_type)
{
    create_background();
    back->paste(img, dest_x, dest_y, src_x, src_y,
        src_width, src_height, collision_type);
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

    PROFILE_BEGIN(Layer_draw_instances);

    if (order_changed) {
        LayerInstances::iterator it;
        int i = 0;
        for (it = instances.begin(); it != instances.end(); it++) {
            (*it)->depth = i;
            i++;
        }
        order_changed = false;
    }

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
    draw_sorted_list(draw_list);

    PROFILE_END();

    PROFILE_BEGIN(Layer_draw_pasted);

    // draw pasted items.
    // this is bending the rules a bit -- we're supposed to draw this *after*
    // background instances and *before* dynamic instances, but we'd have to
    // insert the pasted items in the broadphase as FrameObjects then
    if (back != NULL)
        back->draw();

    PROFILE_END();
}

#if defined(CHOWDREN_IS_WIIU) || defined(CHOWDREN_EMULATE_WIIU)
void Layer::set_remote(int value)
{
    remote = value;
}
#endif

// FrameData

FrameData::FrameData(Frame * frame)
: frame(frame)
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

void FrameData::handle_events()
{
}

// Frame

Frame::Frame(GameManager * manager)
: off_x(0), off_y(0), new_off_x(0), new_off_y(0), has_quit(false),
  last_key(-1), next_frame(-1), loop_count(0), frame_time(0.0),
  frame_iteration(0), index(-1), manager(manager)
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
    manager->lives = value;
}

void Frame::set_score(int value)
{
    manager->score = value;
}

void Frame::set_display_center(int x, int y)
{
    if (x != -1) {
        new_off_x = int_max(0, x - WINDOW_WIDTH / 2);
        new_off_x = int_min(new_off_x, virtual_width - WINDOW_WIDTH);
    }
    if (y != -1) {
        new_off_y = int_max(0, y - WINDOW_HEIGHT / 2);
        new_off_y = int_min(new_off_y, virtual_height - WINDOW_HEIGHT);
    }
}

void Frame::update_display_center()
{
    if (off_x == new_off_x && off_y == new_off_y)
        return;

    vector<Layer>::iterator it;
    for (it = layers.begin(); it != layers.end(); it++) {
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
    if (x < 0 || y < 0 || x > width || y > height)
        return false;
    vector<Layer>::iterator it;
    for (it = layers.begin(); it != layers.end(); it++) {
        if ((*it).test_background_collision(x, y))
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
    platform_set_vsync(value);
}

int Frame::get_instance_count()
{
    int size = 0;
    ObjectList::iterator it;
    for (unsigned int i = 0; i < MAX_OBJECT_ID; i++) {
        ObjectList & list = GameManager::instances.items[i];
        size += list.size();
    }
    return size;
}

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

extern void global_object_update(float dt);

bool Frame::update(float dt)
{
    frame_time += dt;

    if (timer_base == 0) {
        timer_mul = 1.0f;
    } else {
        timer_mul = dt * timer_base;
    }

    PROFILE_BEGIN(frame_update_objects);
    global_object_update(dt);
    PROFILE_END();

    PROFILE_BEGIN(clean_instances);
    clean_instances();
    PROFILE_END();

    PROFILE_BEGIN(handle_events);
    data->handle_events();
    update_display_center();
    PROFILE_END();

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

    vector<Layer>::iterator it;
    for (it = layers.begin(); it != layers.end(); it++) {
        Layer & layer = *it;

#if defined(CHOWDREN_IS_WIIU) || defined(CHOWDREN_EMULATE_WIIU)
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
        layer.draw();
    }

// #ifdef CHOWDREN_USE_BOX2D
//     Box2D::draw_debug();
// #endif
}

static Layer default_layer(0, 1.0, 1.0, false, false, false);

void setup_default_instance(FrameObject * obj)
{
    obj->layer = &default_layer;
    obj->width = obj->height = 0;
}

class DefaultActive : public Active
{
public:
    DefaultActive()
    : Active(0, 0, 0)
    {
        setup_default_instance(this);
        collision = new InstanceBox(this);
        create_alterables();
    }
};

static DefaultActive default_active;
FrameObject * default_active_instance = &default_active;

#ifdef CHOWDREN_USE_BLITTER

class DefaultBlitter : public TextBlitter
{
public:
    DefaultBlitter()
    : TextBlitter(0, 0, 0)
    {
        setup_default_instance(this);
        collision = new InstanceBox(this);
        create_alterables();
    }
};

static DefaultBlitter default_blitter;
FrameObject * default_blitter_instance = &default_blitter;

#endif

FrameObject * Frame::add_object(FrameObject * instance, Layer * layer)
{
    instance->frame = this;
    instance->layer = layer;
    GameManager::instances.items[instance->id].add(instance);
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
    if (instance->id != BACKGROUND_TYPE) {
        GameManager::instances.items[instance->id].add(instance);
    }
    layer->add_background_object(instance);
    if (instance->collision) {
        instance->collision->update_aabb();
        instance->collision->create_proxy();
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
    new_layer = clamp(new_layer, 0, layers.size()-1);
    instance->layer->remove_object(instance);
    Layer * layer = &layers[new_layer];
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
        ObjectList & list = GameManager::instances.items[i];
        for (it = list.begin(); it != list.end(); it++) {
            if (it->obj->flags & BACKGROUND)
                continue;
            delete it->obj;
        }
    }

    layers.clear();
    GameManager::instances.clear();
    destroyed_instances.clear();
    next_frame = -1;
    loop_count = 0;
    off_x = new_off_x = 0;
    off_y = new_off_y = 0;
    frame_time = 0.0;
    frame_iteration++;
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
    delete alterables;
#ifdef CHOWDREN_USE_VALUEADD
    delete extra_alterables;
#endif
}

void FrameObject::draw_image(Image * img, int x, int y, double angle,
                             double x_scale, double y_scale, bool flip_x,
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

void FrameObject::set_position(int x, int y)
{
    this->x = x;
    this->y = y;
    if (collision == NULL)
        return;
    collision->update_aabb();
}

void FrameObject::set_global_position(int x, int y)
{
    set_position(x - layer->off_x, y - layer->off_y);
}

void FrameObject::set_x(int x)
{
    this->x = x - layer->off_x;
    if (collision == NULL)
        return;
    collision->update_aabb();
}

int FrameObject::get_x()
{
    return x + layer->off_x;
}

void FrameObject::set_y(int y)
{
    this->y = y - layer->off_y;
    if (collision == NULL)
        return;
    collision->update_aabb();
}

int FrameObject::get_y()
{
    return y + layer->off_y;
}

void FrameObject::create_alterables()
{
    alterables = new Alterables;
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

bool FrameObject::overlaps_background()
{
    if (flags & DESTROYING || collision == NULL)
        return false;
    if (layer->back != NULL && layer->back->collide(collision))
        return true;
    return collision->overlaps_type(BACKGROUND_TYPE);
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
    return !collides(box[0] + layer->off_x, box[1] + layer->off_y,
                     box[2] + layer->off_x, box[3] + layer->off_y,
                     0, 0, frame->width, frame->height);
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
    if (!layer->order_changed)
        return depth;
    return layer->get_level(this);
}

void FrameObject::move_back(FrameObject * other)
{
    if (other == NULL)
        return;
    if (other->layer != layer)
        return;
    int level = get_level();
    int level2 = other->get_level();
    if (level < level2)
        return;
    layer->set_level(level, level2);
}

void FrameObject::move_back()
{
    layer->set_level(this, 0);
}

void FrameObject::move_front()
{
    layer->set_level(this, -1);
}

void FrameObject::move_front(FrameObject * other)
{
    if (other == NULL)
        return;
    if (other->layer != layer)
        return;
    int level = get_level();
    int level2 = other->get_level();
    if (level > level2)
        return;
    layer->set_level(level, level2);
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

void FrameObject::update_flash(float dt, float interval, float & t)
{
    if (interval == 0.0f)
        return;
    t += dt;
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

// XXX move everything to 'objects' directory

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

Animations::Animations(int count, Animation ** items)
: count(count), items(items)
{
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
  animation_frame(0), counter(0), angle(0.0f), forced_frame(-1),
  forced_speed(-1), forced_direction(-1), x_scale(1.0f), y_scale(1.0f),
  animation_direction(0), stopped(false), flash_interval(0.0f),
  animation_finished(-1), transparent(false), image(NULL)
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
    if (flags & FADEOUT)
        return;
    forced_frame = value;
    update_frame();
}

void Active::force_speed(int value)
{
    if (flags & FADEOUT)
        return;
    Direction * dir = get_direction_data();
    int delta = dir->max_speed - dir->min_speed;
    if (delta != 0) {
        value = (value * delta) / 100 + dir->min_speed;
        value = std::min(dir->max_speed, value);
    }
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

void Active::restore_direction()
{
    if (flags & FADEOUT)
        return;
    forced_direction = -1;
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
    if (flags & FADEOUT)
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

    image = get_image();
    if (image == NULL)
        return;

    active_col.set_image(image, image->hotspot_x, image->hotspot_y);
    update_action_point();
}

void Active::update_direction()
{
    loop_count = get_direction_data()->loop_count;
    update_frame();
}

void Active::update_action_point()
{
    active_col.get_transform(image->action_x, image->action_y,
                             action_x, action_y);
    action_x -= active_col.new_hotspot_x;
    action_y -= active_col.new_hotspot_y;
}

void Active::update(float dt)
{
    if (flags & FADEOUT && animation_finished == DISAPPEARING) {
        FrameObject::destroy();
        return;
    }

    update_flash(dt, flash_interval, flash_time);

    animation_finished = -1;

    if (forced_frame != -1 || stopped)
        return;

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

#ifdef CHOWDREN_RESTORE_ANIMATIONS
        if (forced_animation != -1 && (flags & FADEOUT) == 0)
            restore_animation();
#else
        if (forced_animation == APPEARING)
            restore_animation();
#endif
    }
    if (animation_frame != old_frame)
        update_frame();
}

inline int get_active_load_point(int value, int max)
{
    if (value == 100000) {
        return max / 2;
    } else if (value == 110000) {
        return max;
    }
    return value;
}

void Active::load(const std::string & filename, int anim, int dir, int frame,
                  int hot_x, int hot_y, int action_x, int action_y,
                  int transparent_color)
{
    Image * new_image = new Image(convert_path(filename), 0, 0, 0, 0);

    if (!new_image->is_valid()) {
        std::cout << "Could not load image " << filename << std::endl;
        delete new_image;
        return;
    }

    new_image->hotspot_x = get_active_load_point(hot_x, new_image->width);
    new_image->hotspot_y = get_active_load_point(hot_x, new_image->height);
    new_image->action_x = get_active_load_point(action_x, new_image->width);
    new_image->action_y = get_active_load_point(action_y, new_image->height);

    Animation * animation = animations->items[anim];
    Direction * direction = animation->dirs[dir];

    Image * old_image = direction->frames[frame];
    if (old_image->handle == -1)
        delete old_image;

    new_image->upload_texture();
    direction->frames[frame] = new_image;

    // update the frame for all actives of this type
    // this may break in the future, maybe
    ObjectList::iterator it;
    ObjectList & list = GameManager::instances.items[id];
    for (it = list.begin(); it != list.end(); it++) {
        Active * obj = (Active*)it->obj;
        obj->update_frame();
    }
}

void Active::draw()
{
    if (image == NULL) {
        std::cout << "Invalid image draw (" << get_name() << ")" << std::endl;
        return;
    }
    blend_color.apply();
    bool blend = transparent || blend_color.a < 255;
    if (!blend)
        glDisable(GL_BLEND);
    draw_image(image, x, y, angle, x_scale, y_scale, false, false);
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
    return get_x() + action_x;
}

int Active::get_action_y()
{
    return get_y() + action_y;
}

void Active::set_angle(float angle, int quality)
{
    angle = mod(angle, 360.0f);
    this->angle = angle;
    active_col.set_angle(angle);
    update_action_point();
}

float Active::get_angle()
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
    if (flags & FADEOUT)
        return;
    FrameObject::set_direction(value, set_movement);
    if (auto_rotate) {
        set_angle(float(value) * 11.25f);
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

void Active::set_scale(float value)
{
    value = std::max(0.0f, value);
    x_scale = y_scale = value;
    active_col.set_scale(value);
    update_action_point();
}

void Active::set_x_scale(float value)
{
    x_scale = std::max(0.0f, value);
    active_col.set_x_scale(x_scale);
    update_action_point();
}

void Active::set_y_scale(float value)
{
    y_scale = std::max(0.0f, value);
    active_col.set_y_scale(y_scale);
    update_action_point();
}

void Active::paste(int collision_type)
{
    layer->paste(image, x-image->hotspot_x, y-image->hotspot_y, 0, 0,
                 image->width, image->height, collision_type);
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

bool Active::is_near_border(int border)
{
    int * box = active_col.aabb;
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

bool Active::is_animation_finished(int anim)
{
    return animation_finished == anim;
}

void Active::destroy()
{
    if (flags & FADEOUT)
        return;
    if (!has_animation(DISAPPEARING)) {
        FrameObject::destroy();
        return;
    }
    flags |= FADEOUT;
    clear_movements();
    force_animation(DISAPPEARING);
    collision->type = NONE_COLLISION;
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

FontList fonts;
static bool has_fonts = false;
static FTTextureFont * big_font = NULL;
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
    has_fonts = load_fonts(font_path, fonts);

    FontList::const_iterator it;
    for (it = fonts.begin(); it != fonts.end(); it++) {
        FTTextureFont * font = *it;
        if (big_font != NULL && big_font->size > font->size)
            continue;
        big_font = font;
    }

    initialized = true;
}

FTTextureFont * get_font(int size)
{
    init_font();

    FTTextureFont * picked = NULL;
    int diff = 0;
    FontList::const_iterator it;

    for (it = fonts.begin(); it != fonts.end(); it++) {
        FTTextureFont * font = *it;
        int new_diff = get_abs(font->size - size);

        if (picked == NULL || new_diff < diff) {
            picked = font;
            diff = new_diff;
        }
    }

    return picked;
}

Text::Text(int x, int y, int type_id)
: FrameObject(x, y, type_id), initialized(false), current_paragraph(0),
  draw_text_set(false), layout(NULL), scale(1.0f)
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
    init_font();
    if (!has_fonts) {
        set_visible(false);
        return;
    }

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
        glScalef(scale, -scale, scale);
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
    // XXX should have aabb update
    width = w;
    if (layout == NULL) {
        layout = new FTSimpleLayout;
        layout->SetFont(font);
    }
    layout->SetLineLength(w);
}

void Text::set_scale(float scale)
{
    // XXX should have aabb update
    this->scale = scale;
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

void FontInfo::set_scale(FrameObject * obj, float scale)
{
    ((Text*)obj)->set_scale(scale);
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
    blend_color.apply();
    draw_image(image, x + image->hotspot_x, y + image->hotspot_y);
}

// QuickBackdrop

QuickBackdrop::QuickBackdrop(int x, int y, int type_id)
: FrameObject(x, y, type_id), image(NULL)
{
#ifdef CHOWDREN_LAYER_WRAP
    x_offset = y_offset = 0;
#endif
}

#ifdef CHOWDREN_LAYER_WRAP
void QuickBackdrop::set_backdrop_offset(int dx, int dy)
{
    x_offset = (x_offset + dx) % image->width;
    y_offset = (y_offset + dy) % image->height;
}
#endif

QuickBackdrop::~QuickBackdrop()
{
    delete collision;
}

static void draw_gradient(int x1, int y1, int x2, int y2, int gradient_type,
                          Color & color, Color & color2, float alpha)
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

static int align_pos(int a, int b)
{
    return (a / b) * b;
}

void QuickBackdrop::draw()
{
    if (image != NULL) {
#ifdef CHOWDREN_LAYER_WRAP
        int x, y;
        int width, height;

        // this is a cheap implementation of the wrap feature.
        // we expect objects to extend on either the X or Y axis.
        if (layer->wrap_x) {

            x = frame->off_y * layer->scroll_y + x_offset - image->width;
            width = WINDOW_WIDTH + image->width * 2;
        } else if (layer->wrap_y) {
            y = frame->off_y * layer->scroll_y + y_offset - image->height;
            height = WINDOW_HEIGHT + image->height * 2;
        } else
#endif
        {
            x = this->x;
            y = this->y;
            width = this->width;
            height = this->height;
        }

        glEnable(GL_SCISSOR_TEST);
        glc_scissor_world(x, y, width, height);
        blend_color.apply();
        for (int xx = x; xx < x + width; xx += image->width)
        for (int yy = y; yy < y + height; yy += image->height) {
            draw_image(image, xx + image->hotspot_x, yy + image->hotspot_y);
        }
        glDisable(GL_SCISSOR_TEST);
    } else {
        begin_draw();
        glDisable(GL_TEXTURE_2D);
        int x1 = x;
        int y1 = y;
        int x2 = x + width;
        int y2 = y + height;
        if (outline > 0) {
            glBegin(GL_QUADS);
            glColor4ub(outline_color.r, outline_color.g, outline_color.b,
                       blend_color.a);
            glVertex2f(x1, y1);
            glVertex2f(x2, y1);
            glVertex2f(x2, y2);
            glVertex2f(x1, y2);
            glEnd();
            x1 += outline;
            y1 += outline;
            x2 -= outline;
            y2 -= outline;
        }

        draw_gradient(x1, y1, x2, y2, gradient_type, color, color2,
                      blend_color.a);
        end_draw();
    }
}

// Counter

Counter::Counter(int x, int y, int type_id)
: FrameObject(x, y, type_id), flash_interval(0.0f), zero_pad(0)
{
}

Counter::~Counter()
{
    delete collision;
}

void Counter::calculate_box()
{
    if (type == IMAGE_COUNTER) {
        width = 0;
        height = 0;
        for (std::string::const_iterator it = cached_string.begin();
             it != cached_string.end(); it++) {
            Image * image = get_image(it[0]);
            width += image->width;
            height = std::max(image->height, height);
        }
        ((OffsetInstanceBox*)collision)->set_offset(-width, -height);
    } else if (type == ANIMATION_COUNTER) {
        Image * image = get_image();
        width = image->width;
        height = image->height;
        ((OffsetInstanceBox*)collision)->update_aabb();
    }
}

Image * Counter::get_image()
{
    if (maximum <= minimum)
        return images[0];
    int max_index = image_count - 1;
    int i = (((value - minimum) * max_index) / (maximum - minimum));
    return images[i];
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

    if (type == HIDDEN_COUNTER)
        return;

    if (collision == NULL)
        collision = new OffsetInstanceBox(this);

    if (type == IMAGE_COUNTER) {
        std::ostringstream str;
        if (zero_pad > 0)
            str << std::setw(zero_pad) << std::setfill('0') << value;
        else
            str << value;
        cached_string = str.str();
        calculate_box();
    } else if (type == ANIMATION_COUNTER) {
        calculate_box();
    }
}

void Counter::set_max(int value)
{
    maximum = value;
    set(this->value);
}

void Counter::set_min(int value)
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

    if (type == IMAGE_COUNTER) {
        blend_color.apply();
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
        int draw_height = ((value - minimum) * height) /
                          (maximum - minimum);

        int x1 = x;
        int y2 = y + height;
        int y1 = y2 - draw_height;
        int x2 = x + width;

        draw_gradient(x1, y1, x2, y2, gradient_type, color1, color2,
                      blend_color.a);
    } else if (type == HORIZONTAL_LEFT_COUNTER) {
        int draw_width = ((value - minimum) * width) /
                          (maximum - minimum);

        int x1 = x;
        int y1 = y;
        int x2 = x + draw_width;
        int y2 = y + height;

        draw_gradient(x1, y1, x2, y2, gradient_type, color1, color2,
                      blend_color.a);
    } else if (type == ANIMATION_COUNTER) {
        blend_color.apply();
        Image * image = get_image();
        image->draw(x + image->hotspot_x, y + image->hotspot_y);
    }
}

// Lives

Lives::Lives(int x, int y, int type_id)
: FrameObject(x, y, type_id), flash_interval(0.0f)
{
    collision = new InstanceBox(this);
}

Lives::~Lives()
{
    delete collision;
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

void File::create_directory(const std::string & path)
{
    create_directories(path);
}

bool File::file_exists(const std::string & path)
{
    FSFile fp(convert_path(path).c_str(), "r");
    return fp.is_open();
}

bool File::file_readable(const std::string & path)
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
    if (platform_remove_file(path))
        return;
    // std::cout << "Could not remove " << path << std::endl;
}

// WindowControl

WindowControl::WindowControl(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{
}

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

int WindowControl::get_x()
{
    // std::cout << "Get window x" << std::endl;
    return 0;
}

int WindowControl::get_y()
{
    // std::cout << "Get window y" << std::endl;
    return 0;
}

void WindowControl::set_x(int x)
{
    // std::cout << "Set window x: " << x << std::endl;
}

void WindowControl::set_y(int y)
{
    // std::cout << "Set window y: " << y << std::endl;
}

void WindowControl::set_position(int x, int y)
{
    set_x(x);
    set_y(y);
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

#define CT_ARRAY_MAGIC "CNC ARRAY"
#define ARRAY_MAJOR_VERSION 2
#define ARRAY_MINOR_VERSION 0

#define NUMERIC_FLAG 1
#define TEXT_FLAG 2
#define BASE1_FLAG 4

void ArrayObject::load(const std::string & filename)
{
    FSFile fp(convert_path(filename).c_str(), "r");
    if (!fp.is_open()) {
        std::cout << "Could not load array " << filename << std::endl;
        return;
    }

    FileStream stream(fp);

    std::string magic;
    stream.read_string(magic, sizeof(CT_ARRAY_MAGIC));

    if (magic != CT_ARRAY_MAGIC) {
        std::cout << "Invalid CT_ARRAY_MAGIC" << std::endl;
        return;
    }

    short major, minor;

    stream >> major;
    if (major != ARRAY_MAJOR_VERSION) {
        std::cout << "Invalid ARRAY_MAJOR_VERSION" << std::endl;
        return;
    }

    stream >> minor;
    if (minor != ARRAY_MINOR_VERSION) {
        std::cout << "Invalid ARRAY_MINOR_VERSION" << std::endl;
        return;
    }

    stream >> x_size;
    stream >> y_size;
    stream >> z_size;

    int flags;
    stream >> flags;

    is_numeric = (flags & NUMERIC_FLAG) != 0;
    offset = int((flags & BASE1_FLAG) != 0);

    delete[] array;
    delete[] strings;
    array = NULL;
    strings = NULL;
    clear();

    for (int i = 0; i < x_size * y_size * z_size; i++) {
        if (is_numeric) {
            int value;
            stream >> value;
            array[i] = double(value);
        } else {
            int len;
            stream >> len;
            stream.read_string(strings[i], len);
        }
    }

    fp.close();
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
    frame->layers[index].visible = false;
}

void LayerObject::show_layer(int index)
{
    frame->layers[index].visible = true;
}

void LayerObject::set_position(int index, int x, int y)
{
    frame->layers[index].set_position(x, y);
}

void LayerObject::set_x(int index, int x)
{
    Layer & layer = frame->layers[index];
    layer.set_position(x, layer.y);
}

void LayerObject::set_y(int index, int y)
{
    Layer & layer = frame->layers[index];
    layer.set_position(layer.x, y);
}

void LayerObject::set_alpha_coefficient(int index, int alpha)
{
    Layer * layer = &frame->layers[index];
    FlatObjectList::const_iterator it;
    for (it = layer->background_instances.begin();
         it != layer->background_instances.end(); it++) {
        FrameObject * obj = *it;
        obj->blend_color.set_alpha_coefficient(alpha);
    }
}

double LayerObject::get_alterable(FrameObject * instance)
{
    if (instance->alterables == NULL)
        return def;
    return instance->alterables->values.get(sort_index);
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
    Layer * layer = &frame->layers[current_layer];
    layer->order_changed = true;
    LayerInstances & instances = layer->instances;
#ifdef LAYER_USE_STD_SORT
    std::sort(instances.begin(), instances.end(), sort_func);
#else
    instances.sort(sort_func);
#endif
}

// Viewport

Viewport * Viewport::instance = NULL;

Viewport::Viewport(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    collision = new InstanceBox(this);
    instance = this;
}

Viewport::~Viewport()
{
    glDeleteTextures(1, &texture);
    delete collision;
    instance = NULL;
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
    collision->update_aabb();
}

void Viewport::set_height(int h)
{
    height = h;
    collision->update_aabb();
}

void Viewport::draw()
{
    if (src_width == width && src_height == height)
        return;
    if (src_width == 0 || src_height == 0)
        return;
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
: FrameObject(x, y, type_id), flash_interval(0.0f), x_spacing(0), y_spacing(0),
  x_scroll(0), y_scroll(0), anim_type(BLITTER_ANIMATION_NONE),
  charmap_ref(true), has_transparent(false), callback_line_count(0)
{
    collision = new InstanceBox(this);
}

void TextBlitter::load(const std::string & filename)
{
    Color * color = NULL;
    if (has_transparent)
        color = &transparent_color;

    Image * new_image = new Image(filename, 0, 0, 0, 0, color);
    if (!new_image->is_valid()) {
        std::cout << "Could not load Text Blitter image " << filename
            << std::endl;
        delete new_image;
        return;
    }

    if (image->handle == -1)
        delete image;

    image = new_image;
    image->upload_texture();
}

TextBlitter::~TextBlitter()
{
    if (image != NULL && image->handle == -1)
        delete image;
    if (!charmap_ref) {
        delete[] charmap;
        delete charmap_str;
    }
    delete collision;
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

int TextBlitter::get_x_align()
{
    if (alignment & ALIGN_LEFT)
        return 0;
    if (alignment & ALIGN_HCENTER)
        return 1;
    if (alignment & ALIGN_RIGHT)
        return 2;
    return 0;
}

int TextBlitter::get_y_align()
{
    if (alignment & ALIGN_TOP)
        return 0;
    if (alignment & ALIGN_VCENTER)
        return 1;
    if (alignment & ALIGN_BOTTOM)
        return 2;
    return 0;
}

void TextBlitter::set_x_align(int value)
{
    alignment &= ~(ALIGN_LEFT | ALIGN_HCENTER | ALIGN_RIGHT);
    switch (value) {
        case 0:
            alignment |= ALIGN_LEFT;
            break;
        case 1:
            alignment |= ALIGN_HCENTER;
            break;
        case 2:
            alignment |= ALIGN_RIGHT;
            break;
    }
}

void TextBlitter::set_y_align(int value)
{
    alignment &= ~(ALIGN_TOP | ALIGN_VCENTER | ALIGN_BOTTOM);
    switch (value) {
        case 0:
            alignment |= ALIGN_TOP;
            break;
        case 1:
            alignment |= ALIGN_VCENTER;
            break;
        case 2:
            alignment |= ALIGN_BOTTOM;
            break;
    }
}

void TextBlitter::set_x_spacing(int value)
{
    x_spacing = value;
}

void TextBlitter::set_y_spacing(int value)
{
    y_spacing = value;
}

void TextBlitter::set_x_scroll(int value)
{
    x_scroll = value;
}

void TextBlitter::set_y_scroll(int value)
{
    y_scroll = value;
}

void TextBlitter::set_width(int w)
{
    width = w;
    collision->update_aabb();
}

void TextBlitter::set_height(int h)
{
    height = h;
    collision->update_aabb();
}

void TextBlitter::set_text(const std::string & value)
{
    text = value;
    update_lines();
}

void TextBlitter::update_lines()
{
    lines.clear();

    if (text.empty()) {
        lines.push_back(LineReference(NULL, 0));
        return;
    }

    int x_add = char_width + x_spacing;
    int y_add = char_height + y_spacing;

    char * text_c = &text[0];

    for (unsigned int i = 0; i < text.size(); i++) {
        int start = i;
        int size = 0;
        int last_space = -1;

        // find start + end of line
        while (true) {
            if (i >= text.size())
                break;
            if (text_c[i] == '\n')
                break;
            if (wrap && size * x_add > width) {
                if (last_space != -1) {
                    size = last_space - start;
                    i = last_space;
                }
                i--;
                break;
            }
            unsigned char c = (unsigned char)text_c[i];
            i++;
            if (c == ' ')
                last_space = i;
            if (c == '\r')
                continue;
            // remove leading spaces
            if (i-1 == start && c == ' ') {
                start++;
                continue;
            }
            size++;
        }

        lines.push_back(LineReference(&text_c[start], size));
    }
}

const std::string & TextBlitter::get_text()
{
    return text;
}

int TextBlitter::get_line_count()
{
    return int(lines.size());
}

std::string TextBlitter::get_line(int index)
{
    if (index < 0 || index >= int(lines.size()))
        return empty_string;
    return std::string(lines[index].start, lines[index].size);
}

std::string TextBlitter::get_map_char(int i)
{
    return charmap_str->substr(i, 1);
}

void TextBlitter::replace_color(int from, int to)
{
    Color color1(from);
    Color color2(to);
    // std::cout << "Replace color not implemented: " <<
    //     int(color1.r) << " " << int(color1.g) << " " << int(color1.b)
    //     << " -> " <<
    //     int(color2.r) << " " << int(color2.g) << " " << int(color2.b)
    //     << std::endl;
}

void TextBlitter::set_transparent_color(int v)
{
    has_transparent = true;
    transparent_color = Color(v);
}

void TextBlitter::update(float dt)
{
    update_flash(dt, flash_interval, flash_time);

    if (anim_type != BLITTER_ANIMATION_SINWAVE)
        return;
    anim_frame++;
}

void TextBlitter::flash(float value)
{
    flash_interval = value;
    flash_time = 0.0f;
}

void TextBlitter::set_animation_type(int value)
{
    anim_type = value;
}

void TextBlitter::set_animation_parameter(int index, int value)
{
    switch (index) {
        case 1:
            wave_freq = value;
            break;
        case 2:
            wave_height = value;
            break;
        default:
            std::cout << "Invalid Text Blitter parameter: " << index
                << std::endl;
            break;
    }
}

const std::string & TextBlitter::get_charmap()
{
    return *charmap_str;
}

void TextBlitter::set_charmap(const std::string & charmap)
{
    if (charmap_ref) {
        this->charmap = new int[256];
        charmap_ref = false;
    } else {
        delete charmap_str;
    }
    charmap_str = new std::string(charmap);
    initialize(charmap);
}

void TextBlitter::draw()
{
    begin_draw();

    blend_color.apply();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, image->tex);

    int x_add = char_width + x_spacing;
    int y_add = char_height + y_spacing;

    int yy = y + y_scroll;
    if (alignment & ALIGN_VCENTER)
        yy += height / 2 - lines.size() * char_height / 2
              - (lines.size() - 1) * y_spacing;

    vector<LineReference>::const_iterator it;

    callback_line_count = int(lines.size());

    for (it = lines.begin(); it != lines.end(); it++) {
        const LineReference & line = *it;

        int xx = x + x_scroll;

        if (alignment & ALIGN_HCENTER)
            xx += (width - line.size * char_width -
                   (line.size - 1) * x_spacing) / 2;
        else if (alignment & ALIGN_RIGHT)
            xx += width - line.size * char_width
                  - (line.size - 1) * x_spacing;

        // draw line
        for (int i = 0; i < line.size; i++) {
            unsigned char c = (unsigned char)line.start[i];

            int ci = charmap[c] - char_offset;
            int img_x = (ci * char_width) % image->width;
            img_x = clamp(img_x + x_off, 0, image->width);
            int img_y = ((ci * char_width) / image->width) * char_height;
            img_y = clamp(img_y + y_off, 0, image->height);

            float t_x1 = float(img_x) / float(image->width);
            float t_x2 = float(img_x+char_width) / float(image->width);
            float t_y1 = float(img_y) / float(image->height);
            float t_y2 = float(img_y+char_height) / float(image->height);

            int yyy = yy;
            if (anim_type == BLITTER_ANIMATION_SINWAVE) {
                double t = double(anim_frame / anim_speed + x_add * i);
                t /= double(wave_freq);
                yyy += int(sin(t) * wave_height);
            }

            glBegin(GL_QUADS);
            glTexCoord2f(t_x1, t_y1);
            glVertex2i(xx, yyy);
            glTexCoord2f(t_x2, t_y1);
            glVertex2i(xx + char_width, yyy);
            glTexCoord2f(t_x2, t_y2);
            glVertex2i(xx + char_width, yyy + char_height);
            glTexCoord2f(t_x1, t_y2);
            glVertex2i(xx, yyy + char_height);
            glEnd();

            xx += x_add;
        }

        yy += y_add;
    }

    glDisable(GL_TEXTURE_2D);

    end_draw();
}

// ActivePicture

ActivePicture::ActivePicture(int x, int y, int type_id)
: FrameObject(x, y, type_id), image(NULL), horizontal_flip(false),
  scale_x(1.0f), scale_y(1.0f), angle(0.0f), has_transparent_color(false)
{
    collision = new SpriteCollision(this);
}

ActivePicture::~ActivePicture()
{
    image = NULL;
    delete collision;
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
    Color * transparent = NULL;
    if (has_transparent_color)
        transparent = &transparent_color;

    image = get_image_cache(filename, 0, 0, 0, 0, transparent);

    if (image == NULL)
        return;

    SpriteCollision * col = (SpriteCollision*)collision;
    col->set_image(image, 0, 0);
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
    SpriteCollision * col = (SpriteCollision*)collision;
    this->x += x - col->hotspot_x;
    this->y += y - col->hotspot_y;
    ((SpriteCollision*)collision)->set_hotspot(x, y);
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

void ActivePicture::set_scale(float value)
{
    ((SpriteCollision*)collision)->set_scale(value);
    scale_x = scale_y = value;
}

void ActivePicture::set_zoom(float value)
{
    set_scale(value / 100.0);
}

void ActivePicture::set_angle(float value, int quality)
{
    ((SpriteCollision*)collision)->set_angle(value);
    angle = value;
}

float ActivePicture::get_zoom_x()
{
    return scale_x * 100.0f;
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
    SpriteCollision * col = (SpriteCollision*)collision;
    image->hotspot_x = col->hotspot_x;
    image->hotspot_y = col->hotspot_y;
    draw_image(image, x, y, angle, scale_x, scale_y, horizontal_flip);
}

void ActivePicture::paste(int dest_x, int dest_y, int src_x, int src_y,
                          int src_width, int src_height, int collision_type)
{
    if (image == NULL) {
        std::cout << "Invalid image paste: " << filename << std::endl;
        return;
    }
    image->hotspot_x = 0;
    image->hotspot_y = 0;
    layer->paste(image, dest_x, dest_y, src_x, src_y,
                 src_width, src_height, collision_type);
}

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
