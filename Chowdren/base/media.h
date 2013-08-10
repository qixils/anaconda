#ifndef CHOWDREN_MEDIA_H
#define CHOWDREN_MEDIA_H

void set_sounds_path(const std::string & path);

class SoundData;

namespace ChowdrenAudio
{
    class SoundBase;
}

class Channel
{
public:
    std::string name;
    bool locked;
    ChowdrenAudio::SoundBase * sound;
    double volume, frequency, pan;
    bool is_music;

    Channel();
    void play(SoundData * data, int loop);
    void stop();
    void set_volume(double value);
    void set_frequency(double value);
    void set_position(double value);
    double get_position();
    void set_pan(double value);
    bool is_invalid();
    bool is_stopped();
};

typedef std::map<std::string, SoundData*> SoundMap;

class Media
{
public:
    SoundMap sounds;
    Channel channels[32];

    Media();
    ~Media();
    void play(SoundData * data, int channel = -1, int loop = 1);
    void play(const std::string & filename, int channel = -1, int loop = 1);
    void play_name(const std::string & name, int channel = -1, int loop = 1);
    void lock(unsigned int channel);
    void set_channel_volume(unsigned int channel, double volume);
    void set_channel_frequency(unsigned int channel, double freq);
    void set_channel_pan(unsigned int channel, double pan);
    void stop_channel(unsigned int channel);
    void stop_samples();
    double get_channel_position(unsigned int channel);
    void set_channel_position(unsigned int channel, double pos);
    double get_channel_volume(unsigned int channel);
    bool is_sample_playing(const std::string & name);
    bool is_channel_playing(unsigned int channel);
    bool is_channel_valid(unsigned int channel);
    void add_cache(const std::string & name, const std::string & fn);
    void add_file(const std::string & name, const std::string & fn);
    double get_main_volume();
    void set_main_volume(double volume);
};

#endif // CHOWDREN_MEDIA_H
