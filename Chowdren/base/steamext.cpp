#include "steamext.h"
#include <stdlib.h>
#include <iostream>

// SteamGlobal

#ifdef CHOWDREN_ENABLE_STEAM
#include "sdk/public/steam/steam_api.h"
#include "sdk/public/steam/steamtypes.h"

class SteamGlobal
{
public:
    bool initialized;
    bool steam_initialized;
    bool has_data;

    SteamGlobal();
    static void on_close();
    bool is_ready();
    void update();
    void unlock_achievement(const std::string & name);
    bool is_achievement_unlocked(const std::string & name);

    STEAM_CALLBACK(SteamGlobal, receive_callback, UserStatsReceived_t, 
                   receive_callback_data);
};

SteamGlobal::SteamGlobal()
: steam_initialized(false), has_data(false),
  receive_callback_data(this, &SteamGlobal::receive_callback)
{
    steam_initialized = SteamAPI_Init();
    if (!steam_initialized) {
        std::cout << "Could not initialize Steam API" << std::endl;
        return;
    }
    SteamUserStats()->RequestCurrentStats();
}

void SteamGlobal::on_close()
{
    SteamAPI_Shutdown();
}

bool SteamGlobal::is_ready()
{
    return has_data;
}

void SteamGlobal::update()
{
    SteamAPI_RunCallbacks();
}

void SteamGlobal::unlock_achievement(const std::string & name)
{
    SteamUserStats()->SetAchievement(name.c_str());
    SteamUserStats()->StoreStats();
}

void SteamGlobal::receive_callback(UserStatsReceived_t * callback)
{
    if (SteamUtils()->GetAppID() != callback->m_nGameID)
        return;
    has_data = true;
}

bool SteamGlobal::is_achievement_unlocked(const std::string & name)
{
    bool achieved;
    SteamUserStats()->GetAchievement(name.c_str(), &achieved);
    return achieved;
}

static SteamGlobal steam;
#endif

// SteamObject

SteamObject::SteamObject(const std::string & name, int x, int y, int type_id) 
: FrameObject(name, x, y, type_id)
{
}

bool SteamObject::is_ready()
{
#ifdef CHOWDREN_ENABLE_STEAM
    return steam.is_ready();
#else
    return true;
#endif
}

void SteamObject::update(float dt)
{
#ifdef CHOWDREN_ENABLE_STEAM
    steam.update();
#endif
}

void SteamObject::unlock_achievement(const std::string & name)
{
#ifdef CHOWDREN_ENABLE_STEAM
    steam.unlock_achievement(name);
#endif
}

bool SteamObject::is_achievement_unlocked(const std::string & name)
{
#ifdef CHOWDREN_ENABLE_STEAM
    return steam.is_achievement_unlocked(name);
#else
    return false;
#endif
}

