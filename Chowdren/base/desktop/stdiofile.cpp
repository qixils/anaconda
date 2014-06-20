#ifdef CHOWDREN_ENABLE_STEAM
#define HANDLE_BASE FileHandle

class FileHandle
{
public:
    virtual ~FileHandle()
    {
    }

    virtual size_t write(void * data, size_t size)
    {
        return 0;
    }

    virtual bool seek(size_t v, int origin)
    {
        return false;
    }

    virtual size_t tell()
    {
        return 0;
    }

    virtual int getc()
    {
        return EOF;
    }

    virtual size_t read(void * data, size_t size)
    {
        return 0;
    }

    virtual void close()
    {
    }

    virtual bool at_end()
    {
        return true;
    }
};

class SteamWriteFile : public FileHandle
{
public:
    std::string filename;
    std::ostringstream stream;

    SteamWriteFile(const std::string & filename)
    {
        std::cout << "Opening Steam write: " << filename << std::endl;
        this->filename = filename;
    }

    size_t write(void * data, size_t size)
    {
        stream.write((const char*)data, size);
        return size;
    }

    bool seek(size_t v, int origin)
    {
        if (origin == SEEK_CUR)
            stream.seekp(v, std::ios_base::cur);
        else if (origin == SEEK_END)
            stream.seekp(v, std::ios_base::end);
        else
            stream.seekp(v, std::ios_base::beg);
        return !stream.fail();
    }

    size_t tell()
    {
        return stream.tellp();
    }

    void close()
    {
        stream.seekp(0, std::ios_base::end);
        std::string v = stream.str();
        SteamRemoteStorage()->FileWrite(filename.c_str(), &v[0], v.size());
    }

    bool at_end()
    {
        return false;
    }
};

class SteamReadFile : public FileHandle
{
public:
    std::istringstream stream;

    SteamReadFile(const char * filename)
    {
        int32 size = SteamRemoteStorage()->GetFileSize(filename);
        std::string v;
        v.resize(size, 0);
        SteamRemoteStorage()->FileRead(filename, &v[0], size);
        stream.str(v);
    }

    size_t read(void * data, size_t size)
    {
        stream.read((char*)data, size);
        return stream.gcount();
    }

    bool seek(size_t v, int origin)
    {
        if (origin == SEEK_CUR)
            stream.seekg(v, std::ios_base::cur);
        else if (origin == SEEK_END)
            stream.seekg(v, std::ios_base::end);
        else
            stream.seekg(v, std::ios_base::beg);
        return !stream.fail();
    }

    size_t tell()
    {
        return stream.tellg();
    }

    void close()
    {
    }

    int getc()
    {
        return stream.get();
    }

    bool at_end()
    {
        int c = getc();
        stream.unget();
        return c == EOF;
    }
};

class StandardFile : public FileHandle
#else
#define HANDLE_BASE StandardFile

class StandardFile
#endif
{
public:
    FILE * fp;

    StandardFile(FSFile * parent, const char * filename, bool is_read)
    {
        const char * real_mode;
        if (is_read)
            real_mode = "rb";
        else
            real_mode = "wb";
#ifdef FSFILE_CONVERT_PATH
        std::string file_string = convert_path(filename);
        const char * file_string_c = file_string.c_str();
        fp = fopen(file_string_c, real_mode);
#else
        fp = fopen(filename, real_mode);
#endif
        parent->closed = fp == NULL;
    }

    bool seek(size_t v, int origin)
    {
        return fseek(fp, v, origin) == 0;
    }

    size_t tell()
    {
        return ftell(fp);
    }

    int getc()
    {
        return fgetc(fp);
    }

    size_t read(void * data, size_t size)
    {
        return fread(data, 1, size, fp);
    }

    size_t write(void * data, size_t size)
    {
        return fwrite(data, 1, size, fp);
    }

    void close()
    {
        fclose(fp);
    }

    bool at_end()
    {
        int c = getc();
        ungetc(c, fp);
        return c == EOF;
    }
};

void FSFile::open(const char * filename, const char * mode)
{
    bool is_read;
    switch (*mode) {
        case 'r':
            is_read = true;
            break;
        case 'w':
            is_read = false;
            break;
    }
#ifdef CHOWDREN_ENABLE_STEAM
    HANDLE_BASE * new_handle = NULL;
    std::string base = get_path_filename(filename);
    if (is_read) {
        const char * base_c = base.c_str();
        if (SteamRemoteStorage()->FileExists(base_c))
            new_handle = new SteamReadFile(base_c);
    } else {
        new_handle = new SteamWriteFile(base);
    }
    if (new_handle == NULL) {
        new_handle = new StandardFile(this, filename, is_read);
        if (closed)
            return;
    }
#else
    HANDLE_BASE * new_handle = new StandardFile(this, filename, is_read);
    if (closed)
        return;
#endif
    handle = (void*)new_handle;
    closed = false;
}

bool FSFile::seek(size_t v, int origin)
{
    return ((HANDLE_BASE*)handle)->seek(v, origin);
}

size_t FSFile::tell()
{
    return ((HANDLE_BASE*)handle)->tell();
}

int FSFile::getc()
{
    return ((HANDLE_BASE*)handle)->getc();
}

size_t FSFile::read(void * data, size_t size)
{
    return ((HANDLE_BASE*)handle)->read(data, size);
}

size_t FSFile::write(void * data, size_t size)
{
    return ((HANDLE_BASE*)handle)->write(data, size);
}

bool FSFile::at_end()
{
    return ((HANDLE_BASE*)handle)->at_end();
}

void FSFile::close()
{
    if (closed)
        return;
    HANDLE_BASE * h = (HANDLE_BASE*)handle;
    h->close();
    delete h;
    closed = true;
}
