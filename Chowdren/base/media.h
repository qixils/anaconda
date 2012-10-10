#include <vector>
#include <map>
#include "audio.h"

class Channel
{
public:
    bool locked;
    Music * music;
    double volume, frequency;

    Channel() 
    : music(NULL), locked(false), volume(100), frequency(0)
    {

    }

    void play(const std::string & filename, int loop)
    {
        if (music != NULL) {
            music->stop();  
            delete music;
        }
        music = new Music();
        if (!music->openFromFile(filename)) {
            std::cout << "Could not load " << filename << std::endl;
            return;
        }
        set_volume(volume);
        if (frequency != 0)
            set_frequency(frequency);
        if (loop == 0)
            music->setLoop(true);
        else if (loop > 1)
            std::cout << "Invalid number of loops (" << loop << ")" << std::endl;
        music->play();
    }

    void set_volume(double value)
    {
        volume = value;
        if (is_invalid())
            return;
        music->setVolume(value);
    }

    void set_frequency(double value)
    {
        frequency = value;
        if (is_invalid())
            return;
        music->setPitch(value / music->getSampleRate());
    }

    bool is_invalid()
    {
        return music == NULL;
    }

    bool is_stopped()
    {
        return is_invalid() || music->getStatus() == SoundSource::Stopped;
    }
};

class Media
{
public:
    std::map<std::string, std::string> sounds;
    Channel channels[32];

    Media()
    {
    }

    void play(const std::string & filename, int channel = -1, int loop = 1)
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
        channels[channel].play(filename, loop);
    }

    void play_name(const std::string & name, int channel = -1, int loop = 1)
    {
        play(sounds[name], channel, loop);
    }

    void lock(unsigned int channel)
    {
        channels[channel].locked = true;
    }

    void set_channel_volume(unsigned int channel, double volume)
    {
        channels[channel].set_volume(volume);
    }

    void set_channel_frequency(unsigned int channel, double freq)
    {
        channels[channel].set_frequency(freq);
    }

    void add_file(const std::string & name, const std::string & filename)
    {
        sounds[name] = filename;
    }
};