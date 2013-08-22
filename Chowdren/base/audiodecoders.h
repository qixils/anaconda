#include <vorbis/vorbisfile.h>
#include <string>
#include "stringcommon.h"
#include "filecommon.h"
#include "platform.h"
#include "path.h"
#include <string.h>

#ifndef CHOWDREN_IS_DESKTOP
#define IS_BIG_ENDIAN
#endif

namespace ChowdrenAudio {

template <typename T>
void swap(T &val1, T &val2)
{
    val1 ^= val2;
    val2 ^= val1;
    val1 ^= val2;
}

class SoundDecoder
{
public:
    std::size_t samples;
    int channels;
    int sample_rate;

    virtual bool is_valid() = 0;
    virtual std::size_t read(signed short * data, std::size_t samples) = 0;
    virtual void seek(double value) = 0;
    virtual ~SoundDecoder() {};
};

size_t read_func(void * ptr, size_t size, size_t nmemb, void *datasource)
{
    FSFile * fp = (FSFile*)datasource;
    return fp->read(ptr, size*nmemb);
}

int seek_func(void *datasource, ogg_int64_t offset, int whence)
{
    FSFile * fp = (FSFile*)datasource;
    fp->seek(offset, whence);
    return 0;
}

int close_func(void *datasource)
{
    FSFile * fp = (FSFile*)datasource;
    fp->close();
    return 0;
}

long tell_func(void *datasource)
{
    FSFile * fp = (FSFile*)datasource;
    return fp->tell();
}

ov_callbacks callbacks = {
    read_func,
    seek_func,
    close_func,
    tell_func
};

class OggDecoder : public SoundDecoder 
{
private:
    OggVorbis_File ogg_file;
    vorbis_info * ogg_info;
    int ogg_bitstream;
    FSFile fp;

public:
    double total_time;

    OggDecoder(const std::string & filename)
    : ogg_info(NULL), ogg_bitstream(0)
    {
        fp.open(filename.c_str(), "r");

        if (ov_open_callbacks((void*)&fp, &ogg_file, NULL, 0, callbacks) != 0)
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
        fp.close();
    }

    bool is_valid()
    {
        return ogg_info != NULL;
    }
    
    std::size_t read(signed short * sdata, std::size_t samples)
    {
        if (!(sdata && samples))
            return 0;
        unsigned int got = 0;
        int bytes = samples * 2;
        char * data = (char*)sdata;
        while(bytes > 0) {
#ifdef IS_BIG_ENDIAN
            int big_endian = 1;
#else
            int big_endian = 0;
#endif 
            int res = ov_read(&ogg_file, &data[got], bytes, 
                              big_endian, 2, 1, &ogg_bitstream);
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
        int ret = ov_time_seek(&ogg_file, value);
        if(ret == 0)
            return;
        std::cout << "Seek failed: " << ret << std::endl;
    }
};

inline unsigned int read_le32(FSFile & file)
{
    unsigned char buffer[4];
    if(!file.read(reinterpret_cast<char*>(buffer), 4)) return 0;
    return buffer[0] | (buffer[1]<<8) | (buffer[2]<<16) | (buffer[3]<<24);
}

inline unsigned short read_le16(FSFile & file)
{
    unsigned char buffer[2];
    if(!file.read(reinterpret_cast<char*>(buffer), 2)) return 0;
    return buffer[0] | (buffer[1]<<8);
}

class WavDecoder : public SoundDecoder 
{
private:
    FSFile file;
    int sample_size;
    int block_align;
    long data_start;
    long data_len;
    size_t rem_len;

public:
    WavDecoder(const std::string & filename)
    : data_start(0)
    {
        file.open(filename.c_str(), "r");
        if (!file.is_open()) {
            std::cerr << "Invalid file: " << filename << std::endl;
            return;
        }

        unsigned char buffer[25];
        unsigned int length;
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
                file.seek(4, SEEK_CUR);
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
                data_start = file.tell();
                data_len = rem_len = length;
            }

            file.seek(length, SEEK_CUR);
        }

        if(data_start > 0) {
            samples = data_len / (sample_size / 8);
            file.seek(data_start);
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
        size_t got = file.read((char*)data, rem*block_align);
        got -= got%block_align;
        rem_len -= got;

#ifdef IS_BIG_ENDIAN
        unsigned char * datac = (unsigned char *)data;
        if (sample_size == 16) {
            for(std::streamsize i = 0; i < got; i+=2)
                swap(datac[i], datac[i+1]);
        } else if (sample_size == 32) {
            for(std::streamsize i = 0; i < got; i+=4) {
                swap(datac[i+0], datac[i+3]);
                swap(datac[i+1], datac[i+2]);
            }
        } else if (sample_size == 64) {
            for(std::streamsize i = 0; i < got; i+=8) {
                swap(datac[i+0], datac[i+7]);
                swap(datac[i+1], datac[i+6]);
                swap(datac[i+2], datac[i+5]);
                swap(datac[i+3], datac[i+4]);
            }
        }
#endif

        return got / (sample_size / 8);
    }

    void seek(double time)
    {
        std::size_t new_pos = time * sample_rate * (sample_size / 8) * channels;
        if (file.seek(data_start + new_pos))
            rem_len = data_len - new_pos;
    }
};

SoundDecoder * create_decoder(const std::string & filename)
{
    std::string ext = get_path_ext(filename);
    SoundDecoder * decoder;
    if (ext == "wav")
        decoder = new WavDecoder(filename);
    else if (ext == "ogg")
        decoder = new OggDecoder(filename);
    else {
        std::cout << "No decoder available for " << filename << std::endl;
        return NULL;
    }
    if (decoder->is_valid())
        return decoder;
    std::cout << "Could not load sound '" << filename << "'" << std::endl;
    return NULL;
}

} // namespace ChowdrenAudio
