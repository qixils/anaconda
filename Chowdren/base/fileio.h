#ifndef CHOWDREN_FILEIO_H
#define CHOWDREN_FILEIO_H

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
