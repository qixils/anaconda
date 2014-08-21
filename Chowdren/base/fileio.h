#ifndef CHOWDREN_FILEIO_H
#define CHOWDREN_FILEIO_H

class FSFile
{
public:
    void * handle;
    bool closed;

    FSFile();
    FSFile(const char * filename, const char * mode);
    ~FSFile();
    void open(const char * filename, const char * mode);
    bool seek(size_t v, int origin = SEEK_SET);
    size_t tell();
    bool is_open();
    void read_line(std::string & line);
    void read_delim(std::string & line, char delim);
    size_t read(void * data, size_t size);
    size_t write(const void * data, size_t size);
    void close();
    bool at_end();
    int getc();
};

bool read_file(const char * filename, char ** data, size_t * ret_size,
                      bool binary = true);
bool read_file(const char * filename, std::string & dst,
               bool binary = true);
bool read_file_c(const char * filename, char ** data, size_t * ret_size,
                 bool binary = true);

#ifdef __cplusplus
extern "C" {
#endif

void * fsfile_fopen(const char * filename, const char * mode);
int fsfile_fclose(void * fp);
int fsfile_fseek(void * fp, long int offset, int origin);
size_t fsfile_fread(void * ptr, size_t size, size_t count, void * fp);
long int fsfile_ftell(void * fp);

#ifdef __cplusplus
}
#endif

#endif // CHOWDREN_FILEIO_H
