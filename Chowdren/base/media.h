#include <vector>
#include <map>
#include "audio.h"
#include "math.h"
#include <algorithm>
#include "filecommon.h"
#include <fstream>
#include "path.h"

void setup_sounds(Media * media);

double clamp_sound(double val)
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
    virtual void load(ChowdrenAudio::SoundBase ** source, bool * is_music) {}
    virtual ~SoundData() {}
};

class SoundFile : public SoundData
{
public:
    std::string filename;

    SoundFile(const std::string & filename) : filename(filename)
    {

    }

    void load(ChowdrenAudio::SoundBase ** source, bool * is_music)
    {
        // std::cout << "Loading " << filename << std::endl;
        *source = new ChowdrenAudio::SoundStream(filename);
        *is_music = true;
    }
};

class SoundMemory : public SoundData
{
public:
    ChowdrenAudio::Sample * buffer;

    SoundMemory(const std::string & filename)
    {
        // std::cout << "Loading buffer " << filename << std::endl;
        buffer = new ChowdrenAudio::Sample(filename);
    }

    void load(ChowdrenAudio::SoundBase ** source, bool * is_music)
    {
        *source = new ChowdrenAudio::Sound(*buffer);
        *is_music = false;
    }

    ~SoundMemory()
    {
        delete buffer;
    }
};

class Channel
{
public:
    bool locked;
    ChowdrenAudio::SoundBase * sound;
    double volume, frequency, pan;
    bool is_music;

    Channel() 
    : locked(false), volume(100), frequency(0), pan(0), sound(NULL)
    {

    }

    void play(SoundData & data, int loop)
    {
        stop();
        data.load(&sound, &is_music);
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

    void stop()
    {
        if (is_invalid())
            return;
        sound->stop();
        delete sound;
        sound = NULL;
    }

    void set_volume(double value)
    {
        volume = clamp_sound(value);
        if (is_invalid())
            return;
        sound->set_volume(value / 100.0);
    }

    void set_frequency(double value)
    {
        frequency = value;
        if (is_invalid())
            return;
        sound->set_frequency(value);
    }

    void set_position(double value)
    {
        if (is_invalid())
            return;
        sound->set_playing_offset(value / 1000.0);
    }

    double get_position()
    {
        if (is_invalid())
            return 0.0;
        return sound->get_playing_offset() * 1000.0;
    }

    void set_pan(double value)
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

    bool is_invalid()
    {
        return sound == NULL || sound->closed;
    }

    bool is_stopped()
    {
        if (is_invalid())
            return true;
        return sound->get_status() == ChowdrenAudio::SoundBase::Stopped;
    }
};

typedef std::map<std::string, SoundData*> SoundMap;

class Media
{
public:
    SoundMap sounds;
    Channel channels[32];

    Media()
    {
        ChowdrenAudio::open_audio();
    }

    ~Media()
    {
        ChowdrenAudio::close_audio();
    }

    void play(SoundData & data, int channel = -1, int loop = 1)
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

    void play(const std::string & filename, int channel = -1, int loop = 1)
    {
        SoundFile data(convert_path(filename));
        play(data, channel, loop);
    }

    void play_name(const std::string & name, int channel = -1, int loop = 1)
    {
        static bool cache_initialized = false;
        if (!cache_initialized) {
            cache_initialized = true;
            setup_sounds(this);
        }
        play(*sounds[name], channel, loop);
    }

    void lock(unsigned int channel)
    {
        if (!is_channel_valid(channel))
            return;
        channels[channel].locked = true;
    }

    void set_channel_volume(unsigned int channel, double volume)
    {
        if (!is_channel_valid(channel))
            return;
        channels[channel].set_volume(volume);
    }

    void set_channel_frequency(unsigned int channel, double freq)
    {
        if (!is_channel_valid(channel))
            return;
        channels[channel].set_frequency(freq);
    }

    void set_channel_pan(unsigned int channel, double pan)
    {
        if (!is_channel_valid(channel))
            return;
        channels[channel].set_pan(pan);
    }

    void stop_channel(unsigned int channel)
    {
        if (!is_channel_valid(channel))
            return;
        channels[channel].stop();
    }

    void stop_samples()
    {
        for (int i = 0; i < 32; i++) {
            stop_channel(i);
        }
    }

    double get_channel_position(unsigned int channel)
    {
        if (!is_channel_valid(channel))
            return 0.0;
        return channels[channel].get_position();
    }

    void set_channel_position(unsigned int channel, double pos)
    {
        if (!is_channel_valid(channel))
            return;
        return channels[channel].set_position(pos);
    }

    double get_channel_volume(unsigned int channel)
    {
        if (!is_channel_valid(channel))
            return 0.0;
        return channels[channel].volume;
    }

    bool is_channel_playing(unsigned int channel)
    {
        if (!is_channel_valid(channel))
            return false;
        return !channels[channel].is_stopped();
    }

    bool is_channel_valid(unsigned int channel)
    {
        return (channel >= 0 && channel < 32);
    }

    void add_cache(const std::string & name, const std::string & fn)
    {
        add_file(name, cache_path + fn);
    }

    void add_file(const std::string & name, const std::string & fn)
    {
        std::string filename = convert_path(fn);
        SoundMap::const_iterator it = sounds.find(name);
        if (it != sounds.end()) {
            delete it->second;
        }
        SoundData * data;
        if (get_file_size(filename.c_str()) > 0.5 * 1024 * 1024) // 0.5 mb
            data = new SoundFile(filename);
        else
            data = new SoundMemory(filename);
        sounds[name] = data;
    }

    double get_main_volume()
    {
        return ChowdrenAudio::Listener::get_volume() * 100.0;
    }

    void set_main_volume(double volume)
    {
        ChowdrenAudio::Listener::set_volume(clamp_sound(volume) / 100.0);
    }
};

#include "sounds.h"