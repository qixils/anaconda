#include "assets.h"
#include <iostream>

static std::string assets_folder = std::string(".");

const std::string & get_assets_folder()
{
    return assets_folder;
}

void set_assets_folder(const std::string & value)
{
    std::cout << "Setting assets folder to: " << value << std::endl;
    assets_folder = value;
}