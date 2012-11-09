#include "assets.h"
#include <iostream>
#include "path.h"

static std::string assets_folder = std::string("./images");

const std::string & get_assets_folder()
{
    return assets_folder;
}

void set_assets_folder(const std::string & value)
{
    assets_folder = convert_path(value);
    std::cout << "Setting assets folder to: " << assets_folder << std::endl;
}