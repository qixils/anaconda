#include <vorbis/vorbisfile.h>
#include <string>
#include "string.h"

namespace ChowdrenAudio {

class SoundDecoder
{
public:
    std::size_t samples;
    unsigned int channels;
    unsigned int sample_rate;

    virtual bool is_valid() = 0;
    virtual std::size_t read(signed short * data, std::size_t samples) = 0;
    virtual void seek(double value) = 0;
    virtual ~SoundDecoder() {};
};

class OggDecoder : public SoundDecoder 
{
private:
    OggVorbis_File ogg_file;
    vorbis_info * ogg_info;
    int ogg_bitstream;

public:
    double total_time;

    OggDecoder(const std::string & filename)
    : ogg_info(NULL), ogg_bitstream(0)
    {
        if (ov_fopen(filename.c_str(), &ogg_file) != 0)
            return;

        ogg_info = ov_info(&ogg_file, -1);
        if (!ogg_info) {
            ov_clear(&ogg_file);
            return;
        }

        channels = ogg_info->channels;
        sample_rate = ogg_info->rate;
        samples = ov_pcm_total(&ogg_file, -1) * channels;
    }

    ~OggDecoder()
    {
        if(ogg_info)
            ov_clear(&ogg_file);
        ogg_info = NULL;
    }

    bool is_valid()
    {
        return ogg_info != NULL;
    }
    
    std::size_t read(signed short * sdata, std::size_t samples)
    {
        if (!(sdata && samples))
            return 0;
        ALuint got = 0;
        int bytes = samples * 2;
        char * data = (char*)sdata;
        while(bytes > 0) {
            // XXX support big-endian architectures?
            int res = ov_read(&ogg_file, &data[got], bytes, 
                              0, 2, 1, &ogg_bitstream);
            if(res <= 0)
                break;
            bytes -= res;
            got += res;
        }
        // XXX support exotic channel formats?
        return got / 2;
    }

    void seek(double value)
    {
        if(ov_time_seek(&ogg_file, value) == 0)
            return;
        std::cout << "Seek failed" << std::endl;
    }
};

inline ALuint read_le32(std::ifstream & file)
{
    ALubyte buffer[4];
    if(!file.read(reinterpret_cast<char*>(buffer), 4)) return 0;
    return buffer[0] | (buffer[1]<<8) | (buffer[2]<<16) | (buffer[3]<<24);
}

inline ALushort read_le16(std::ifstream & file)
{
    ALubyte buffer[2];
    if(!file.read(reinterpret_cast<char*>(buffer), 2)) return 0;
    return buffer[0] | (buffer[1]<<8);
}

class WavDecoder : public SoundDecoder 
{
private:
    std::ifstream file;
    int sample_size;
    int block_align;
    long data_start;
    long data_len;
    size_t rem_len;

public:
    WavDecoder(const std::string & filename)
    : data_start(0)
    {
        file.open(filename.c_str(), std::ios::binary);
        if (!file) {
            std::cerr << "Invalid file: " << filename << std::endl;
            return;
        }

        ALubyte buffer[25];
        ALuint length;
        if(!file.read(reinterpret_cast<char*>(buffer), 12) ||
           memcmp(buffer, "RIFF", 4) != 0 || memcmp(buffer+8, "WAVE", 4) != 0) {
            std::cerr << "Invalid header: " << filename << std::endl;
            return;
        }

        while(!data_start) {
            char tag[4];
            if(!file.read(tag, 4))
                break;

            length = read_le32(file);

            if(memcmp(tag, "fmt ", 4) == 0 && length >= 16) {
                // data type (should be 1 for PCM data, 3 for float PCM data
                int type = read_le16(file);
                if(type != 0x0001 && type != 0x0003) {
                    std::cerr << "Invalid type: " << filename << std::endl;
                    break;
                }

                channels = read_le16(file);
                sample_rate = read_le32(file);
                file.ignore(4);
                block_align = read_le16(file);
                if(block_align == 0) {
                    std::cerr << "Invalid blockalign: " << filename 
                        << std::endl;
                    break;
                }
                sample_size = read_le16(file);
                if (sample_size != 16) {
                    std::cerr << "Invalid sample size: " << filename 
                        << std::endl;
                    break;
                }
                length -= 16;

            }
            else if(memcmp(tag, "data", 4) == 0) {
                data_start = file.tellg();
                data_len = rem_len = length;
            }

            file.seekg(length, std::ios_base::cur);
        }

        if(data_start > 0) {
            samples = data_len / (sample_size / 8);
            file.seekg(data_start);
        }
    }

    ~WavDecoder()
    {
        file.close();
    }

    bool is_valid()
    {
        return (data_start > 0);
    }

    std::size_t read(signed short * data, std::size_t samples)
    {
        unsigned int bytes = samples * (sample_size / 8);
        std::streamsize rem = ((rem_len >= bytes) ? bytes : rem_len) / block_align;
        file.read(reinterpret_cast<char*>(data), rem*block_align);

        std::streamsize got = file.gcount();
        got -= got%block_align;
        rem_len -= got;

        // XXX big-endian

        return got / (sample_size / 8);
    }

    void seek(double time)
    {
        file.clear();
        std::size_t new_pos = time * sample_rate * (sample_size / 8) * channels;
        if (file.seekg(data_start + new_pos))
            rem_len = data_len - new_pos;
    }
};

SoundDecoder * create_decoder(const std::string & filename)
{
    std::string ext("wav");
    std::string::size_type pos = filename.find_last_of(".");
    if (pos != std::string::npos)
        ext = filename.substr(pos + 1);
    ext = to_lower(ext);
    if (ext == "wav")
        return new WavDecoder(filename);
    if (ext == "ogg")
        return new OggDecoder(filename);
    std::cout << "No decoder available for " << filename << std::endl;
    return NULL;
}

} // namespace ChowdrenAudio