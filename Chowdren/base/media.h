#ifndef CHOWDREN_MEDIA_H
#define CHOWDREN_MEDIA_H

#include <boost/unordered_map.hpp>

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
    void resume();
    void pause();
    void stop();
    void set_volume(double value);
    void set_frequency(double value);
    void set_position(double value);
    double get_position();
    double get_duration();
    void set_pan(double value);
    bool is_invalid();
    bool is_stopped();
};

typedef boost::unordered_map<std::string, SoundData*> SoundMap;

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
    void resume_channel(unsigned int channel);
    void pause_channel(unsigned int channel);
    void stop_channel(unsigned int channel);
    Channel * get_sample(const std::string & name);
    void set_sample_volume(const std::string & name, double volume);
    void set_sample_pan(const std::string & name, double pan);
    void set_sample_position(const std::string & name, double pos);
    void set_sample_frequency(const std::string & name, double freq);
    double get_sample_position(const std::string & name);
    double get_sample_duration(const std::string & name);
    void stop_sample(const std::string & name);
    void stop_samples();
    void pause_samples();
    void resume_samples();
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
