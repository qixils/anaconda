#ifndef CHOWDREN_MOVEMENT_H
#define CHOWDREN_MOVEMENT_H

#include <string>

class FrameObject;

class Movement
{
public:
    int speed;
    double add_x, add_y;
    FrameObject * instance;
    unsigned int directions;

    Movement(FrameObject * instance);
    virtual void update(float dt);
    virtual void set_max_speed(int speed);
    virtual void set_speed(int speed);
    virtual void start();
    virtual void stop();
    virtual void bounce();
    virtual bool is_stopped();
    virtual int get_speed();
    virtual void set_node(const std::string & node);
    virtual bool is_path_finished();
    virtual bool is_node_reached();
    void set_directions(unsigned int directions);
    void move(double add_x, double add_y);
};

class StaticMovement : public Movement
{
public:
    StaticMovement(FrameObject * instance);
};

class BallMovement : public Movement
{
public:
    BallMovement(FrameObject * instance);
    void update(float dt);
};

class ShootMovement : public Movement
{
public:
    ShootMovement(FrameObject * instance);
    void update(float dt);
};

#endif // CHOWDREN_MOVEMENT_H
