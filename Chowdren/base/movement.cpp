#include "movement.h"
#include "frameobject.h"
#include "common.h"
#include "mathcommon.h"
#include <iostream>

inline double get_pixels(int speed)
{
    return speed / 8.0;
}

static const int accelerators[] =
{
    0x0002,0x0003,0x0004,0x0006,0x0008,0x000a,0x000c,0x0010,0x0014,0x0018,
    0x0030,0x0038,0x0040,0x0048,0x0050,0x0058,0x0060,0x0068,0x0070,0x0078,
    0x0090,0x00A0,0x00B0,0x00c0,0x00d0,0x00e0,0x00f0,0x0100,0x0110,0x0120,
    0x0140,0x0150,0x0160,0x0170,0x0180,0x0190,0x01a0,0x01b0,0x01c0,0x01e0,
    0x0200,0x0220,0x0230,0x0250,0x0270,0x0280,0x02a0,0x02b0,0x02d0,0x02e0,
    0x0300,0x0310,0x0330,0x0350,0x0360,0x0380,0x03a0,0x03b0,0x03d0,0x03e0,
    0x0400,0x0460,0x04c0,0x0520,0x05a0,0x0600,0x0660,0x06c0,0x0720,0x07a0,
    0x0800,0x08c0,0x0980,0x0a80,0x0b40,0x0c00,0x0cc0,0x0d80,0x0e80,0x0f40,
    0x1000,0x1990,0x1332,0x1460,0x1664,0x1800,0x1999,0x1b32,0x1cc6,0x1e64,
    0x2000,0x266c,0x2d98,0x3404,0x3a70,0x40dc,0x4748,0x4db4,0x5400,0x6400,
    0x6400
};

inline double get_accelerator(int value)
{
    if (value < 100)
        return accelerators[value] / 256.0;
    return value;
}

static const int movement_directions[] =
{
    -1,             // 0000 Static
    8,              // 0001
    24,             // 0010
    -1,             // 0011 Static
    16,             // 0100
    12,             // 0101
    20,             // 0110
    16,             // 0111
    0,              // 1000
    4,              // 1001
    28,             // 1010
    0,              // 1011
    -1,             // 1100 Static
    8,              // 1101
    24,             // 1110
    -1              // 1111 Static
};

int get_movement_direction(bool up, bool down, bool left, bool right)
{
    int v = int(up) | (int(down) << 1) | (int(left) << 2) | (int(right) << 3);
    return movement_directions[v];
}

int get_movement_direction(int v)
{
    return movement_directions[v];
}

// Movement

Movement::Movement(FrameObject * instance)
: instance(instance), speed(0), add_x(0), add_y(0), max_speed(0),
  back_col(false)
{

}

void Movement::update(float dt)
{

}

void Movement::set_deceleration(int value)
{

}

void Movement::set_acceleration(int value)
{

}

void Movement::set_gravity(int value)
{

}

void Movement::set_max_speed(int v)
{
    max_speed = v;
}

int Movement::get_speed()
{
    return speed;
}

void Movement::set_speed(int v)
{
    speed = v;
}

void Movement::set_direction(int value)
{

}

void Movement::start()
{
    set_speed(max_speed);
}

void Movement::reverse()
{
    set_speed(-speed);
}

void Movement::stop(bool collision)
{
    set_speed(0);
}

void Movement::bounce(bool collision)
{
    set_speed(0);
}

bool Movement::is_stopped()
{
    return speed == 0;
}

void Movement::clear_collisions()
{
    collisions.clear();
    back_col = false;
}

void Movement::move(double x, double y)
{
    add_x += x;
    add_y += y;
    double xx = floor(add_x);
    double yy = floor(add_y);
    add_x -= xx;
    add_y -= yy;
    old_x = instance->x;
    old_y = instance->y;
    instance->set_position(instance->x + xx,
                           instance->y + yy);
    clear_collisions();
}

void Movement::set_directions(unsigned int directions)
{
    this->directions = directions;
}

void Movement::set_node(const std::string & node)
{
}

bool Movement::is_path_finished()
{
    return false;
}

bool Movement::is_node_reached()
{
    return false;
}

bool Movement::test_direction(int dir, int displacement)
{
    float add_x, add_y;
    get_dir(dir, add_x, add_y);
    add_x *= displacement;
    add_y *= displacement;
    return test_offset(add_x, add_y);
}

bool Movement::test_offset(float x, float y)
{
    return test_position(int(instance->x + x), int(instance->y + y));
}

bool Movement::test_position(int x, int y)
{
    if (!back_col && collisions.empty())
        return false;
    int old_x = instance->x;
    int old_y = instance->y;
    instance->set_position(x, y);
    bool ret = false;
    if (back_col)
        ret = instance->overlaps_background();
    if (!ret) {
        ObjectList::const_iterator it;
        for (it = collisions.begin(); it != collisions.end(); it++) {
            FrameObject * obj = *it;
            if (!instance->overlaps(obj))
                continue;
            ret = true;
            break;
        }
    }
    instance->set_position(old_x, old_y);
    return ret;
}

static const int fix_pos_table[] = {
    0,-1,   0,1,    0,-2,   0,2,    0,-3,   0,3,    -2,0,   -3,0,
    -1,-1,  1,1,    -2,-2,  2,2,    -3,-3,  3,3,    -2,2,   -3,3,
    -1, 0,  1,0,    -2,0,   2,0,    -3,0,   3,0,    0,2,    0,3,
    -1, 1,  1,-1,   -2,2,   2,-2,   -3,3,   3,-3,   2,2,    3,3,
    0, 1,   0,-1,   0,2,    0,-2,   0,3,    0,-3,   2,0,    3,0,
    1,1,    -1,-1,  2,2,    -2,-2,  3,3,    -3,-3,  2,-2,   3,-3,
    1,0,    -1,0,   2,0,    -2,0,   3,0,    -3,0,   0,-2,   0,-3,
    1,-1,   -1,1,   2,-2,   -2,2,   3,-3,   -3,3,   -2,-2,  -3,-3,
};

bool Movement::fix_position()
{
    if (push_out())
        return true;

    int table_index = (instance->direction/4) * 16;

    for (int i = 0; i < 8; i++) {
        int x = instance->x + fix_pos_table[table_index]*2;
        int y = instance->y + fix_pos_table[table_index+1]*2;
        table_index += 2;
        if (test_position(x, y))
            continue;
        instance->set_position(x, y);
        return true;
    }

    instance->set_position(old_x, old_y);
    return false;
}

bool Movement::push_out()
{
    if (!back_col && collisions.empty())
        return true;
    int src_x = old_x;
    int src_y = old_y;
    int dst_x = instance->x;
    int dst_y = instance->y;

    int x = (dst_x+src_x)/2;
    int y = (dst_y+src_y)/2;
    int old_x, old_y;

    while (true) {
        if (test_position(x, y)) {
            dst_x = old_x = x;
            dst_y = old_y = y;
            x = (src_x+dst_x)/2;
            y = (src_y+dst_y)/2;
            if (x != old_x || y!=old_y)
                continue;
            if (src_x != dst_x || src_y != dst_y) {
                if (!test_position(src_x, src_y)) {
                    instance->set_position(src_x, src_y);
                    return true;
                }
            }
            instance->set_position(x, y);
            return false;
        } else {
            src_x = old_x = x;
            src_y = old_y = y;
            x = (src_x+dst_x)/2;
            y = (src_y+dst_y)/2;
            if (x != old_x || y != old_y)
                continue;
            if (src_x != dst_x || src_y != dst_y) {
                if (!test_position(dst_x, dst_y)) {
                    x = dst_x;
                    y = dst_y;
                }
            }
            instance->set_position(x, y);
            return true;
        }
    }
    return false;
}

void Movement::add_collision(FrameObject * obj)
{
    collisions.push_back(obj);
}

void Movement::set_background_collision()
{
    back_col = true;
}

// StaticMovement

StaticMovement::StaticMovement(FrameObject * instance)
: Movement(instance)
{

}

// BallMovement

BallMovement::BallMovement(FrameObject * instance)
: Movement(instance)
{

}

void BallMovement::update(float dt)
{
    double add_x, add_y;
    get_dir(instance->direction, add_x, add_y);
    double m = get_pixels(speed) * instance->frame->timer_mul;
    move(add_x * m, add_y * m);
}

void BallMovement::bounce(bool collision)
{
    fix_position();

    float angle = rad(instance->direction * 11.25f);
    float found_a = -1.0f;
    for (float a = 0.0f; a < (CHOW_PI*2.0f); a += (CHOW_PI*2.0f) / 16.0f) {
        float x_move = 10.0f * cos(angle + a);
        float y_move = -10.0f * sin(angle + a);

        int x = instance->x + x_move;
        int y = instance->y + y_move;

        if (!test_position(x, y)) {
            found_a = a;
            break;
        }
    }

    if (found_a == -1.0f) {
        instance->set_direction((instance->direction + 16) % 32, false);
        return;
    }

    angle += found_a * 2.0f;
    if (angle > 2.0 * CHOW_PI)
        angle -= 2.0 * CHOW_PI;

    instance->set_direction(deg(angle) / 11.25f, false);
}

// PathMovement

PathMovement::PathMovement(FrameObject * instance)
: Movement(instance), current_node(-1), distance_left(0.0f),
  dir(1), node_changed(false), start_x(0), start_y(0)
{

}

void PathMovement::set_path(bool loop, bool reverse, int end_x, int end_y)
{
    this->loop = loop;
    this->has_reverse = reverse;
    this->end_x = end_x;
    this->end_y = end_y;
}

void PathMovement::add_node(int speed, int x, int y, float dir_x, float dir_y,
                            int length, int dir, float pause)
{
    PathNode node;
    node.speed = speed;
    node.x = x;
    node.y = y;
    node.dir_x = dir_x;
    node.dir_y = dir_y;
    node.length = length;
    node.direction = dir;
    node.pause = pause;
    nodes.push_back(node);
}

void PathMovement::add_named_node(int i, const std::string & name)
{
    NamedNode node;
    node.index = i;
    node.name = name;
    named_nodes.push_back(node);
}

void PathMovement::set_current_node(int i)
{
    start_x = start_y = 0;
    current_node = i;
    PathNode & node = nodes[i];
    distance_left = float(node.length);
    instance->set_direction(node.direction, false);
    set_speed(node.speed);
}

void PathMovement::start()
{
    if (current_node == -1)
        set_current_node(0);
    else if (current_node == -2)
        return;
    else
        set_speed(nodes[current_node].speed);
}

void PathMovement::stop(bool collision)
{
    set_speed(0);
}

void PathMovement::update(float dt)
{
    node_changed = false;
    if (current_node < 0) {
        instance->set_animation(STOPPED);
        return;
    }
    instance->set_animation(WALKING);
    PathNode & node = nodes[current_node];
    float m = get_pixels(speed) * instance->frame->timer_mul;
    float move_dist = std::min<float>(m, distance_left);
    float move_val = move_dist * dir;
    int old_x = instance->x;
    int old_y = instance->y;
    move(node.dir_x * move_val, node.dir_y * move_val);
    start_x -= instance->x - old_x;
    start_y -= instance->y - old_y;
    distance_left -= move_dist;
    if (distance_left <= 0.0f) {
        int final_x = instance->x + start_x + node.x * dir;
        int final_y = instance->y + start_y + node.y * dir;
        instance->set_position(final_x, final_y);
        add_x = add_y = 0;
        start_x = start_y = 0;
        node_changed = true;
        int next_node = current_node+dir;
        bool is_last = next_node == nodes.size() || next_node == -1;
        if (!is_last) {
            set_current_node(next_node);
            return;
        }
        if (has_reverse && dir == 1) {
            dir = -1;
            set_current_node(current_node);
            return;
        }
        if (!has_reverse && dir == 1)
            move(-end_x, -end_y);
        if (!loop) {
            current_node = -2;
            return;
        }
        if (has_reverse || dir == -1)
            dir = -dir;
        next_node = (current_node+dir) % nodes.size();
        set_current_node(next_node);
    }
}

bool PathMovement::is_path_finished()
{
    return current_node == -2;
}

bool PathMovement::is_node_reached()
{
    return node_changed;
}

void PathMovement::set_node(const std::string & value)
{
}

void PathMovement::reverse()
{
    dir = -dir;
    if (current_node >= 0) {
        PathNode & node = nodes[current_node];
        distance_left = node.length - distance_left;
        start_x += node.x * -dir;
        start_y += node.y * -dir;
        return;
    }
    int n;
    if (dir == 1)
        n = 0;
    else
        n = nodes.size() - 1;
    set_current_node(n);
}

// PinballMovement

PinballMovement::PinballMovement(FrameObject * instance)
: Movement(instance), x_speed(0.0f), y_speed(0.0f), stopped(true)
{

}

void PinballMovement::start()
{
    if (!stopped)
        return;
    stopped = false;
    Movement::start();
    get_dir(instance->direction, x_speed, y_speed);
    x_speed *= speed;
    y_speed *= speed;
}

void PinballMovement::stop(bool collision)
{
    if (stopped)
        return;
    stopped = true;
}

void PinballMovement::set_direction(int value)
{
    float dist = get_length(x_speed, y_speed);
    get_dir(value, x_speed, y_speed);
    x_speed *= dist;
    y_speed *= dist;
}

float get_pinball_angle(float x, float y)
{
    float d = get_length(x, y);
    if (d == 0.0f)
        return 0.0f;
    float angle = acos(x / d);
    if (y > 0.0f)
        angle = 2.0 * CHOW_PI - angle;
    return angle;
}

void PinballMovement::update(float dt)
{
    if (stopped)
        return;
    y_speed += gravity / 10.0f;
    float m = instance->frame->timer_mul;
    float angle = get_pinball_angle(x_speed, y_speed);
    float dist = get_length(x_speed, y_speed);
    float decel = deceleration * m;
    dist -= decel / 50.0f;
    if (dist < 0.0f)
        dist = 0.0f;
    x_speed = dist * cos(angle);
    y_speed = -dist * sin(angle);
    speed = int_min(int(dist), 100);
    instance->set_direction((angle * 32.0f) / (2.0 * CHOW_PI), false);
    move((x_speed * m) / 10.0f, (y_speed * m) / 10.0f);
}

void PinballMovement::bounce(bool collision)
{
    add_x = add_y = 0.0;

    if (!collision) {
        x_speed = -x_speed;
        y_speed = -y_speed;
        return;
    }

    push_out();

    float angle = get_pinball_angle(x_speed, y_speed);
    float dist = get_length(x_speed, y_speed);

    float found_a = -1.0f;
    for (float a = 0.0f; a < (2.0f*CHOW_PI); a += (2.0f*CHOW_PI) / 32.0f) {
        float x_move = 10.0f * cos(angle + a);
        float y_move = -10.0f * sin(angle + a);

        if (!test_offset(x_move, y_move)) {
            found_a = a;
            break;
        }
    }

    if (found_a == -1.0f) {
        x_speed = -x_speed;
        y_speed = -y_speed;
        return;
    }

    angle += found_a * 2.0f;
    if (angle > 2.0 * CHOW_PI)
        angle -= 2.0 * CHOW_PI;

    // add some randomness
    angle += randrange(-0.3f, 0.3f);
    dist += randrange(0.0f, 15.0f);

    x_speed = dist * cos(angle);
    y_speed = -dist * sin(angle);
}

void PinballMovement::set_deceleration(int value)
{
    deceleration = value;
}

void PinballMovement::set_gravity(int value)
{
    gravity = value;
}

void PinballMovement::set_speed(int speed)
{
    float angle = get_pinball_angle(x_speed, y_speed);
    x_speed = speed * cos(angle);
    y_speed = -speed * sin(angle);
    Movement::set_speed(speed);
}

// ShootMovement

ShootMovement::ShootMovement(FrameObject * instance)
: Movement(instance)
{

}

void ShootMovement::update(float dt)
{
    double add_x, add_y;
    get_dir(instance->direction, add_x, add_y);
    double m = get_pixels(speed) * instance->frame->timer_mul;
    move(add_x * m, add_y * m);
}

// EightDirections

EightDirections::EightDirections(FrameObject * instance)
: Movement(instance)
{
}

void EightDirections::set_deceleration(int value)
{
    deceleration = value;
}

void EightDirections::set_acceleration(int value)
{
    acceleration = value;
}

void EightDirections::start()
{
}

template <class T>
int rounded_sign(T x)
{
    return sign_int(int_round(x));
}

void EightDirections::stop(bool collision)
{
    // set_speed(0);
    if (!collision)
        return;

    // float angle = rad(instance->direction * 11.25);

    // int x, y;
    // float angle2;

    // for (float a = rad(90); a <= rad(180); a += rad(90) / 8.0f) {
    //     const float d = 8.0f;
    //     angle2 = angle + a;
    //     x = instance->x + cos(angle2) * d;
    //     y = instance->y - sin(angle2) * d;
    //     if (!test_position(x, y)) {
    //         instance->set_position(x, y);
    //         return;
    //     }
    //     angle2 = angle - a;
    //     x = instance->x + cos(angle2) * d;
    //     y = instance->y - sin(angle2) * d;
    //     if (!test_position(x, y)) {
    //         instance->set_position(x, y);
    //         return;
    //     }
    // }

    // int dir1 = instance->direction + 6;
    // int dir2 = instance->direction - 6;

    // int x, y;
    // float add_x, add_y;

    // get_dir(dir1, add_x, add_y);
    // add_x *= last_move;
    // add_y *= last_move;
    // x = old_x + add_x;
    // y = old_y + add_y;
    // if (!test_position(x, y)) {
    //     instance->set_position(x, y);
    //     return;
    // }

    // get_dir(dir2, add_x, add_y);
    // add_x *= last_move;
    // add_y *= last_move;
    // x = old_x + add_x;
    // y = old_y + add_y;
    // if (!test_position(x, y)) {
    //     instance->set_position(x, y);
    //     return;
    // }

    if (!test_position(old_x, instance->y)) {
        instance->set_position(old_x, instance->y);
        return;
    }

    if (!test_position(instance->x, old_y)) {
        instance->set_position(instance->x, old_y);
        return;
    }

    fix_position();
}

void EightDirections::update(float dt)
{
    if (max_speed == 0)
        return;

    bool on = false;
    int dir = get_joystick_direction(1);
    if (dir == 8)
        dir = instance->direction;
    else {
        on = true;
        dir *= 4;
        instance->set_direction(dir, false);
    }
    double mul = instance->frame->timer_mul;

    double change;
    if (on)
        change = get_accelerator(acceleration);
    else
        change = -get_accelerator(deceleration);

    set_speed(int_max(0, int_min(speed + change * mul, max_speed)));

    if (speed == 0)
        return;

    double add_x, add_y;
    get_dir(instance->direction, add_x, add_y);
    double m = get_pixels(speed) * mul;
    move(add_x * m, add_y * m);
    last_move = m;
}
