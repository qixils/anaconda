#ifdef CHOWDREN_IS_DESKTOP
#include "desktop/audio.h"
#else
#include "audio.h"
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
    return std::max(0.0, std::min(val, 100.0));
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
        // std::cout << "Playing (stream) " << filename << std::endl;
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
        // std::cout << "Playing: " << name << std::endl;
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
    sound->set_loop(loop == 0);
    if (loop > 1)
        std::cout << "Invalid number of loops (" << loop << ")" << std::endl;
    sound->play();
}

void Channel::resume()
{
    if (is_invalid())
        return;
    if (sound->get_status() == ChowdrenAudio::SoundBase::Stopped)
        return;
    sound->play();
}

void Channel::pause()
{
    if (is_invalid())
        return;
    sound->pause();
}

void Channel::stop()
{
    if (is_invalid())
        return;
    sound->destroy();
    sound = NULL;
}

void Channel::set_volume(double value)
{
    volume = clamp_sound(value);
    if (is_invalid())
        return;
    sound->set_volume(volume / 100.0);
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
            if (!channelp.is_stopped() || channelp.locked)
                continue;
            // unspecified channel does not inherit settings
            channelp.volume = 100;
            channelp.frequency = 0;
            channelp.pan = 0;
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

void Media::resume_channel(unsigned int channel)
{
    if (!is_channel_valid(channel))
        return;
    channels[channel].resume();
}

void Media::pause_channel(unsigned int channel)
{
    if (!is_channel_valid(channel))
        return;
    channels[channel].pause();
}

Channel * Media::get_sample(const std::string & name)
{
    for (int i = 0; i < 32; i++) {
        if (channels[i].name != name)
            continue;
        return &channels[i];
    }
    return NULL;
}

void Media::set_sample_volume(const std::string & name, double volume)
{
    Channel * channel = get_sample(name);
    if (channel == NULL)
        return;
    channel->set_volume(volume);
}

void Media::set_sample_pan(const std::string & name, double pan)
{
    Channel * channel = get_sample(name);
    if (channel == NULL)
        return;
    channel->set_pan(pan);
}

void Media::set_sample_position(const std::string & name, double pos)
{
    Channel * channel = get_sample(name);
    if (channel == NULL)
        return;
    channel->set_position(pos);
}

double Media::get_sample_position(const std::string & name)
{
    Channel * channel = get_sample(name);
    if (channel == NULL)
        return 0.0;
    return channel->get_position();
}

void Media::stop_sample(const std::string & name)
{
    Channel * channel = get_sample(name);
    if (channel == NULL)
        return;
    channel->stop();
}

void Media::stop_samples()
{
    for (int i = 0; i < 32; i++) {
        stop_channel(i);
    }
}

void Media::pause_samples()
{
    for (int i = 0; i < 32; i++) {
        pause_channel(i);
    }
}

void Media::resume_samples()
{
    for (int i = 0; i < 32; i++) {
        resume_channel(i);
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
#define STREAM_THRESHOLD 0.75
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
