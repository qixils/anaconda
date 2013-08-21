#include <vector>
#include <map>

#ifdef CHOWDREN_IS_DESKTOP
#include "desktop/audio.h"
#else
#include "wiiu/audio.h"
#endif

#include "math.h"
#include <algorithm>
#include "filecommon.h"
#include "path.h"
#include "media.h"

class Media;
void setup_sounds(Media * media);

inline double clamp_sound(double val)
{
    return std::max<double>(0, std::min<double>(val, 100));
}

static std::string cache_path("./sounds/");

void set_sounds_path(const std::string & path)
{
    std::cout << "Set sounds path: " << path << std::endl;
    cache_path = path + std::string("/");
}

class SoundData
{
public:
    std::string name;

    SoundData(const std::string & name)
    : name(name)
    {

    }

    virtual void load(ChowdrenAudio::SoundBase ** source, bool * is_music) {}
    virtual ~SoundData() {}
};

class SoundFile : public SoundData
{
public:
    std::string filename;

    SoundFile(const std::string & name, const std::string & filename) 
    : SoundData(name), filename(filename)
    {

    }

    void load(ChowdrenAudio::SoundBase ** source, bool * is_music)
    {
        std::cout << "Playing (stream) " << filename << std::endl;
        *source = new ChowdrenAudio::SoundStream(filename);
        // *source = NULL;
        *is_music = true;
    }
};

class SoundMemory : public SoundData
{
public:
    ChowdrenAudio::Sample * buffer;

    SoundMemory(const std::string & name, const std::string & filename)
    : SoundData(name)
    {
        buffer = new ChowdrenAudio::Sample(filename);
    }

    void load(ChowdrenAudio::SoundBase ** source, bool * is_music)
    {
        std::cout << "Playing: " << name << std::endl;
        *source = new ChowdrenAudio::Sound(*buffer);
        *is_music = false;
    }

    ~SoundMemory()
    {
        delete buffer;
    }
};

// Channel

Channel::Channel() 
: locked(false), volume(100), frequency(0), pan(0), sound(NULL)
{

}

void Channel::play(SoundData * data, int loop)
{
    name = data->name;
    stop();
    data->load(&sound, &is_music);
    if (sound == NULL) {
        std::cout << "Ignored play" << std::endl;
        return;
    }
    set_volume(volume);
    set_pan(pan);
    if (frequency != 0)
        set_frequency(frequency);
    if (loop == 0) {
        sound->set_loop(true);
    } else if (loop > 1)
        std::cout << "Invalid number of loops (" << loop << ")" << std::endl;
    sound->play();
}

void Channel::stop()
{
    if (is_invalid())
        return;
    sound->stop();
    // delete sound;
    sound->destroy();
    sound = NULL;
}

void Channel::set_volume(double value)
{
    volume = clamp_sound(value);
    if (is_invalid())
        return;
    sound->set_volume(value / 100.0);
}

void Channel::set_frequency(double value)
{
    frequency = value;
    if (is_invalid())
        return;
    sound->set_frequency(value);
}

void Channel::set_position(double value)
{
    if (is_invalid())
        return;
    sound->set_playing_offset(value / 1000.0);
}

double Channel::get_position()
{
    if (is_invalid())
        return 0.0;
    return sound->get_playing_offset() * 1000.0;
}

void Channel::set_pan(double value)
{
    pan = value;
    if (is_invalid())
        return;
    value /= 100;
    if (value > 1.0)
        value = 1.0;
    else if (value < -1.0)
        value = -1.0;
    sound->set_pan(value);
}

bool Channel::is_invalid()
{
    return sound == NULL || sound->closed;
}

bool Channel::is_stopped()
{
    if (is_invalid())
        return true;
    return sound->get_status() == ChowdrenAudio::SoundBase::Stopped;
}

// Media

Media::Media()
{
    ChowdrenAudio::open_audio();
}

Media::~Media()
{
    ChowdrenAudio::close_audio();
}

void Media::play(SoundData * data, int channel, int loop)
{
    if (channel == -1) {
        for (channel = 0; channel < 32; channel++) {
            Channel & channelp = channels[channel];
            if (channelp.is_stopped() && !channelp.locked)
                break;
        }
        if (channel == 32)
            return;
    }
    channels[channel].play(data, loop);
}

void Media::play(const std::string & filename, int channel, int loop)
{
    SoundFile data(filename, convert_path(filename));
    play(&data, channel, loop);
}

void Media::play_name(const std::string & name, int channel, int loop)
{
    static bool cache_initialized = false;
    if (!cache_initialized) {
        cache_initialized = true;
        setup_sounds(this);
    }
    play(sounds[name], channel, loop);
}

void Media::lock(unsigned int channel)
{
    if (!is_channel_valid(channel))
        return;
    channels[channel].locked = true;
}

void Media::set_channel_volume(unsigned int channel, double volume)
{
    if (!is_channel_valid(channel))
        return;
    channels[channel].set_volume(volume);
}

void Media::set_channel_frequency(unsigned int channel, double freq)
{
    if (!is_channel_valid(channel))
        return;
    channels[channel].set_frequency(freq);
}

void Media::set_channel_pan(unsigned int channel, double pan)
{
    if (!is_channel_valid(channel))
        return;
    channels[channel].set_pan(pan);
}

void Media::stop_channel(unsigned int channel)
{
    if (!is_channel_valid(channel))
        return;
    channels[channel].stop();
}

void Media::stop_samples()
{
    for (int i = 0; i < 32; i++) {
        stop_channel(i);
    }
}

double Media::get_channel_position(unsigned int channel)
{
    if (!is_channel_valid(channel))
        return 0.0;
    return channels[channel].get_position();
}

void Media::set_channel_position(unsigned int channel, double pos)
{
    if (!is_channel_valid(channel))
        return;
    return channels[channel].set_position(pos);
}

double Media::get_channel_volume(unsigned int channel)
{
    if (!is_channel_valid(channel))
        return 0.0;
    return channels[channel].volume;
}

bool Media::is_channel_playing(unsigned int channel)
{
    if (!is_channel_valid(channel))
        return false;
    return !channels[channel].is_stopped();
}

bool Media::is_sample_playing(const std::string & name)
{
    for (int i = 0; i < 32; i++) {
        if (channels[i].is_stopped())
            continue;
        if (channels[i].name == name)
            return true;
    }
    return false;
}

bool Media::is_channel_valid(unsigned int channel)
{
    return channel < 32;
}

void Media::add_cache(const std::string & name, const std::string & fn)
{
    add_file(name, cache_path + fn);
}

#ifdef CHOWDREN_IS_DESKTOP
#define STREAM_THRESHOLD 0.5
#else
#define STREAM_THRESHOLD 1.0
#endif

void Media::add_file(const std::string & name, const std::string & fn)
{
    std::string filename = convert_path(fn);
    SoundMap::const_iterator it = sounds.find(name);
    if (it != sounds.end()) {
        delete it->second;
    }
    SoundData * data;
    if (get_path_ext(filename) == "wav")
        data = new SoundMemory(name, filename);
    else if (get_file_size(filename.c_str()) > STREAM_THRESHOLD * 1024 * 1024)
        data = new SoundFile(name, filename);
    else
        data = new SoundMemory(name, filename);
    sounds[name] = data;
}

double Media::get_main_volume()
{
    return ChowdrenAudio::Listener::get_volume() * 100.0;
}

void Media::set_main_volume(double volume)
{
    ChowdrenAudio::Listener::set_volume(clamp_sound(volume) / 100.0);
}

#include "sounds.h"
