#include "movement.h"
#include "frameobject.h"
#include "common.h"
#include "mathcommon.h"
#include <iostream>

inline double get_pixels(int speed)
{
    return speed / 8.0;
}

const static int accelerators[] =
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

const static int movement_directions[] = 
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
  last_collision(NULL), back_col(false)
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

void Movement::stop()
{
    set_speed(0);
}

void Movement::bounce()
{
    set_speed(0);
}

bool Movement::is_stopped()
{
    return speed == 0;
}

void Movement::move(double x, double y)
{
    last_collision = NULL;
    back_col = false;
    add_x += x;
    add_y += y;
    int xx = int(add_x);
    int yy = int(add_y);
    add_x -= xx;
    add_y -= yy;
    old_x = instance->x;
    old_y = instance->y;
    instance->set_position(instance->x + xx,
                           instance->y + yy);
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

bool Movement::test_offset(float x, float y)
{
    return test_position(int(instance->x + x), int(instance->y + y));
}

bool Movement::test_position(int x, int y)
{
    if (!back_col && last_collision == NULL)
        return false;
    int old_x = instance->x;
    int old_y = instance->y;
    instance->set_position(x, y);
    bool ret;
    if (back_col)
        ret = instance->overlaps_background();
    else
        ret = instance->overlaps(last_collision);
    instance->set_position(old_x, old_y);
    return ret;
}

bool Movement::push_out()
{
    if (!back_col && last_collision == NULL)
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
    double m = get_pixels(speed) * dt * instance->frame->timer_base;
    move(add_x * m, add_y * m);
}

void BallMovement::bounce()
{
    instance->set_direction((instance->direction + 16) % 32, false);
    push_out();
}

// PathMovement

PathMovement::PathMovement(FrameObject * instance)
: Movement(instance), current_node(-1), distance_left(0.0f),
  dir(1), node_changed(false)
{

}

void PathMovement::set_path(bool loop, bool reverse, int end_x, int end_y)
{
    this->loop = loop;
    this->reverse = reverse;
    this->end_x = end_x;
    this->end_y = end_y;
}

void PathMovement::add_node(int speed, float x, float y, float length,
                            int dir, float pause)
{
    PathNode node;
    node.speed = speed;
    node.x = x;
    node.y = y;
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

void PathMovement::set_node(int i)
{
    current_node = i;
    PathNode & node = nodes[i];
    distance_left = node.length;
    instance->set_direction(node.direction, false);
    set_speed(node.speed);
}

void PathMovement::start()
{
    if (current_node == -1)
        set_node(0);
    else if (current_node == -2)
        return;
    else
        set_speed(nodes[current_node].speed);
}

void PathMovement::stop()
{
    set_speed(0);
}

void PathMovement::update(float dt)
{
    node_changed = false;
    if (current_node < 0)
        return;
    PathNode & node = nodes[current_node];
    float m = get_pixels(speed) * dt * instance->frame->timer_base;
    float move_m = std::min(m, distance_left) * float(dir);
    move(node.x * move_m, node.y * move_m);
    distance_left -= move_m;
    if (distance_left <= 0.0f) {
        node_changed = true;
        int next_node = current_node+dir;
        bool is_last = next_node == nodes.size() || next_node == -1;
        if (!is_last) {
            set_node(next_node);
            return;
        }
        if (reverse && dir == 1) {
            dir = -1;
            set_node(current_node);
            return;
        }
        move(-end_x, -end_y);
        if (!loop) {
            current_node = -2;
            return;
        }
        if (reverse)
            dir = -dir;
        next_node = (current_node+dir) % nodes.size();
        set_node(next_node);
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

void PinballMovement::stop()
{
    if (stopped)
        return;
    stopped = true;
    x_speed = y_speed = true;
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
        angle = 2.0 * M_PI - angle;
    return angle;
}

void PinballMovement::update(float dt)
{
    if (stopped)
        return;
    y_speed += gravity / 10.0f;
    float m = instance->frame->timer_base * dt;
    float angle = get_pinball_angle(x_speed, y_speed);
    float dist = get_length(x_speed, y_speed);
    float decel = deceleration * m;
    dist -= decel / 50.0f;
    if (dist < 0.0f)
        dist = 0.0f;
    x_speed = dist * cos(angle);
    y_speed = -dist * sin(angle);
    speed = int_min(int(dist), 100);
    instance->set_direction((angle * 32.0f) / (2.0 * M_PI), false);
    move((x_speed * m) / 10.0f, (y_speed * m) / 10.0f);
}

void PinballMovement::bounce()
{
    push_out();

    if (last_collision == NULL && !back_col) {
        x_speed = -x_speed;
        y_speed = -y_speed;
        return;
    }

    float angle = get_pinball_angle(x_speed, y_speed);
    float dist = get_length(x_speed, y_speed);

    float found_a = -1000.0f;
    for (float a = 0.0f; a < 2.0f * M_PI; a += M_PI / 32.0f) {
        float x_move = 16.0f * cos(angle + a);
        float y_move = -16.0f * sin(angle + a);

        if (!test_offset(x_move, y_move)) {
            found_a = a;
            break;
        }
    }

    if (found_a == -1000.0f) {
        x_speed = -x_speed;
        y_speed = -y_speed;
        return;
    }

    angle += found_a * 2.0f;
    if (angle > 2.0 * M_PI)
        angle -= 2.0 * M_PI;

    x_speed = dist * cos(angle);
    y_speed = -dist * sin(angle);

    last_collision = NULL;
    back_col = false;
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
    double m = get_pixels(speed) * dt * instance->frame->timer_base;
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

void EightDirections::stop()
{
    set_speed(0);
    push_out();
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
    double mul = dt * instance->frame->timer_base;

    double change;
    if (on)
        change = get_accelerator(acceleration);
    else
        change = -get_accelerator(deceleration);

    set_speed(int_min(speed + change * mul, max_speed));

    if (!on)
        return;

    double add_x, add_y;
    get_dir(instance->direction, add_x, add_y);
    double m = get_pixels(speed) * mul;
    move(add_x * m, add_y * m);
}
