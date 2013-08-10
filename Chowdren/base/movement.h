#ifndef CHOWDREN_MOVEMENT_H
#define CHOWDREN_MOVEMENT_H

class FrameObject;

class Movement
{
public:
    int speed;
    double add_x, add_y;
    FrameObject * instance;

    Movement(FrameObject * instance);
    virtual void update(float dt);
    virtual void set_speed(int speed);
    virtual void stop();
    virtual bool is_stopped();
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

#endif // CHOWDREN_MOVEMENT_H
