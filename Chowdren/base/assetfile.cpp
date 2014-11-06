#include "assetfile.h"
#include "chowconfig.h"
#include "datastream.h"
#include <iostream>

static bool assets_initialized = false;

static unsigned int image_offsets[IMAGE_ARRAY_SIZE];
static unsigned int sound_offsets[SOUND_ARRAY_SIZE];
static unsigned int font_offsets[FONT_ARRAY_SIZE];
static unsigned int shader_offsets[SHADER_ARRAY_SIZE];

static unsigned int * asset_offsets[] = {
    image_offsets,
    sound_offsets,
    font_offsets,
    shader_offsets
};

void read_offsets(FileStream & stream, int count, unsigned int * array)
{
    for (int i = 0; i < count; i++) {
        array[i] = stream.read_uint32();
    }
}

void init_assets(FSFile & fp)
{
    assets_initialized = true;

    FileStream stream(fp);

    // skip image preload
    fp.seek(IMAGE_COUNT * 2, SEEK_CUR);

    read_offsets(stream, IMAGE_COUNT, image_offsets);
    read_offsets(stream, SOUND_COUNT, sound_offsets);
    read_offsets(stream, FONT_COUNT, font_offsets);
    read_offsets(stream, SHADER_COUNT, shader_offsets);
}

// AssetFile

AssetFile::AssetFile()
: FSFile()
{
}

void AssetFile::open()
{
    FSFile::open("./Assets.dat", "r");
}

void AssetFile::set_item(int index, AssetType type)
{
    if (!assets_initialized) {
        init_assets(*this);
    }

    seek(asset_offsets[type][index]);
}
