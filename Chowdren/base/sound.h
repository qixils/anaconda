// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2012 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.

// taken from SFML 2.0, changed for Chowdren

#define DEBUG_SOUND

#ifdef DEBUG_SOUND

    // If in debug mode, perform a test on every call
    #define check_error(x) ((x), check_error_handler(__FILE__, __LINE__))

#else

    // Else, we don't add any overhead
    #define check_error(x) (x)

#endif

void check_error_handler(const std::string& file, unsigned int line)
{
    // Get the last error
    ALenum error = alGetError();

    if (errorCode == AL_NO_ERROR)
        return;

    std::string error, description;

    switch (error)
    {
        case AL_INVALID_NAME :
        {
            error = "AL_INVALID_NAME";
            description = "an unacceptable name has been specified";
            break;
        }

        case AL_INVALID_ENUM :
        {
            error = "AL_INVALID_ENUM";
            description = "an unacceptable value has been specified for an enumerated argument";
            break;
        }

        case AL_INVALID_VALUE :
        {
            error = "AL_INVALID_VALUE";
            description = "a numeric argument is out of range";
            break;
        }

        case AL_INVALID_OPERATION :
        {
            error = "AL_INVALID_OPERATION";
            description = "the specified operation is not allowed in the current state";
            break;
        }

        case AL_OUT_OF_MEMORY :
        {
            error = "AL_OUT_OF_MEMORY";
            description = "there is not enough memory left to execute the command";
            break;
        }
    }

    // Log the error
    std::cerr << "An internal OpenAL call failed in "
          << file.substr(file.find_last_of("\\/") + 1) << " (" << line << ") : "
          << error << ", " << description
          << std::endl;
}

static AudioDevice * global_device = NULL;

class AudioDevice
{
public:
    ALCdevice * device;
    ALCcontext * context;

    AudioDevice()
    {
        device = alcOpenDevice(NULL);

        if (!device) {
            printf("Failed to open the audio device\n");
            return;
        }

        context = alcCreateContext(device, NULL);
        if (!context) {
            printf("Failed to create audio context\n");
            return;
        }

        alcMakeContextCurrent(audioContext);

        global_device = this;
    }

    ~AudioDevice()
    {
        alcMakeContextCurrent(NULL);
        if (context)
            alcDestroyContext(context);

        if (device)
            alcCloseDevice(device);
    }

    bool has_extension(const std::string& extension)
    {
        if ((extension.length() > 2) && (extension.substr(0, 3) == "ALC"))
            return alcIsExtensionPresent(
                device, extension.c_str()) != AL_FALSE;
        else
            return alIsExtensionPresent(extension.c_str()) != AL_FALSE;
    }

    int get_format(unsigned int channel_count)
    {
        switch (channel_count)
        {
            case 1:
                return AL_FORMAT_MONO16;
            case 2:
                return AL_FORMAT_STEREO16;
            case 4:
                return alGetEnumValue("AL_FORMAT_QUAD16");
            case 6:
                return alGetEnumValue("AL_FORMAT_51CHN16");
            case 7:
                return alGetEnumValue("AL_FORMAT_61CHN16");
            case 8:
                return alGetEnumValue("AL_FORMAT_71CHN16");
            default:
                return 0;
        }
    }
};

class Listener
{
    void set_volume(float volume)
    {
        check_error(alListenerf(AL_GAIN, volume));
    }

    float get_volume()
    {
        float volume = 0.0f;
        check_error(alGetListenerf(AL_GAIN, &volume));
        return volume;
    }

    void set_position(float x, float y, float z)
    {
        check_error(alListener3f(AL_POSITION, x, y, z));
    }

    void get_position(int * x, int * y, int * z)
    {
        check_error(alGetListener3f(AL_POSITION, x, y, z));
    }

    float get_pan()
    {
        ALfloat x, y, z;
        check_error(alGetSource3f(sound->src, AL_POSITION, &x, &y, &z));
        return x;
    }

    void set_pan(float pan)
    {
        if (pan > 1.0)
            pan = 1.0;
        else if (pan < -1.0)
            pan = -1.0;
        check_error(alSource3f(sound->src, AL_POSITION, pan, 
            -sqrt(1.0 - pan * pan), 0));
    }
};

class Sound
{
    Sound(const SoundBuffer& buffer)
    : buffer(NULL)
    {
        set_buffer(buffer);
    }

    ~Sound()
    {
        stop();
        if (buffer)
            buffer->detach_sound(this);
    }

    void play()
    {
        check_error(alSourcePlay(source));
    }

    void pause()
    {
        check_error(alSourcePause(source));
    }

    void stop()
    {
        check_error(alSourceStop(source));
    }

    void set_buffer(const SoundBuffer& buffer)
    {
        // First detach from the previous buffer
        if (buffer)
        {
            stop();
            buffer->detach(this);
        }

        // Assign and use the new buffer
        buffer = &buffer;
        buffer->attach(this);
        check_error(alSourcei(source, AL_BUFFER, buffer->buffer));
    }

    void set_loop(bool Loop)
    {
        check_error(alSourcei(source, AL_LOOPING, Loop));
    }

    void set_offset(float seconds)
    {
        check_error(alSourcef(source, AL_SEC_OFFSET, seconds));
    }

    const SoundBuffer* get_buffer() const
    {
        return buffer;
    }

    bool get_loop() const
    {
        ALint loop;
        check_error(alGetSourcei(source, AL_LOOPING, &loop));

        return loop != 0;
    }

    Time get_offset() const
    {
        ALfloat secs = 0.f;
        check_error(alGetSourcef(source, AL_SEC_OFFSET, &secs));

        return seconds(secs);
    }

    Status get_status() const
    {
        return SoundSource::getStatus();
    }

    void reset_buffer()
    {
        // First stop the sound in case it is playing
        stop();

        // Detach the buffer
        check_error(alSourcei(source, AL_BUFFER, 0));
        buffer = NULL;
    }
};

class SoundStream
{
    ////////////////////////////////////////////////////////////
    SoundStream() 
    : file(new SoundFile), m_duration()
    {

    }


    ////////////////////////////////////////////////////////////
    SoundStream::~SoundStream()
    {
        // We must stop before destroying the file :)
        stop();

        delete file;
    }


    ////////////////////////////////////////////////////////////
    bool SoundStream::openFromFile(const std::string& filename)
    {
        // First stop the music if it was already running
        stop();

        // Open the underlying sound file
        if (!file->openRead(filename))
            return false;

        // Perform common initializations
        initialize();

        return true;
    }


    ////////////////////////////////////////////////////////////
    bool SoundStream::openFromMemory(const void* data, std::size_t sizeInBytes)
    {
        // First stop the music if it was already running
        stop();

        // Open the underlying sound file
        if (!file->openRead(data, sizeInBytes))
            return false;

        // Perform common initializations
        initialize();

        return true;
    }


    ////////////////////////////////////////////////////////////
    bool SoundStream::openFromStream(InputStream& stream)
    {
        // First stop the music if it was already running
        stop();

        // Open the underlying sound file
        if (!file->openRead(stream))
            return false;

        // Perform common initializations
        initialize();

        return true;
    }


    ////////////////////////////////////////////////////////////
    Time SoundStream::getDuration() const
    {
        return m_duration;
    }


    ////////////////////////////////////////////////////////////
    bool SoundStream::onGetData(SoundStream::Chunk& data)
    {
        Lock lock(m_mutex);

        // Fill the chunk parameters
        data.samples     = &m_samples[0];
        data.sampleCount = file->read(&m_samples[0], m_samples.size());

        // Check if we have reached the end of the audio file
        return data.sampleCount == m_samples.size();
    }


    ////////////////////////////////////////////////////////////
    void SoundStream::onSeek(Time timeOffset)
    {
        Lock lock(m_mutex);

        file->seek(timeOffset);
    }


    ////////////////////////////////////////////////////////////
    void SoundStream::initialize()
    {
        // Compute the music duration
        m_duration = seconds(static_cast<float>(file->getSampleCount()) / file->getSampleRate() / file->getChannelCount());

        // Resize the internal buffer so that it can contain 1 second of audio samples
        m_samples.resize(file->getSampleRate() * file->getChannelCount());

        // Initialize the stream
        SoundStream::initialize(file->getChannelCount(), file->getSampleRate());
    }
};
