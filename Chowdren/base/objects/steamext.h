#ifndef CHOWDREN_STEAMEXT_H
#define CHOWDREN_STEAMEXT_H

#include <string>
#include "frameobject.h"

class SteamObject : public FrameObject
{
public:
    SteamObject(int x, int y, int type_id);
    bool is_ready();
    void update(float dt);
    void unlock_achievement(const std::string & name);
    bool is_achievement_unlocked(const std::string & name);
};

#endif // CHOWDREN_STEAMEXT_H
