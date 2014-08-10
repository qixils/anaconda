#include "manager.h"
#include "subapp.h"

SubApplication::SubApplication(int x, int y, int id)
: FrameObject(x, y, id), subapp_frame(global_manager)
{
    current = this;
}

SubApplication::~SubApplication()
{
	if (current == this)
		current = NULL;
}

void SubApplication::set_next_frame(int index)
{
    if (starting)
        return;
    subapp_frame.next_frame = index + frame_offset;
}

void SubApplication::restart(int index)
{
    done = false;
    starting = true;
    subapp_frame.next_frame = index + frame_offset;
}

void SubApplication::update(float dt)
{
    if (done)
        return;
    starting = false;
    Frame * old_frame = global_manager->frame;
    global_manager->frame = &subapp_frame;

    if (subapp_frame.next_frame != -1) {
        int next_frame = subapp_frame.next_frame;
        if (subapp_frame.index != -1)
            subapp_frame.on_end();
        global_manager->frame = old_frame;
        set_frame(next_frame);
        return;
    }

    bool ret = subapp_frame.update(dt);

    if (!ret)
        subapp_frame.on_end();

    global_manager->frame = old_frame;

    if (ret)
        return;
    done = true;
    set_visible(false);
}

void SubApplication::set_frame(int index)
{
    done = false;
    Frame * old_frame = global_manager->frame;
    global_manager->frame = &subapp_frame;

    subapp_frame.set_index(index);
    subapp_frame.on_start();

    global_manager->frame = old_frame;
}

SubApplication * SubApplication::current = NULL;
