#include <string>
#include <GL/glfw.h>

class Image
{
public:
    int handle;
    int hotspot_x, hotspot_y, action_x, action_y;
    GLuint tex;
    int width, height;

    Image(int handle, int hot_x, int hot_y, int act_x, int act_y);
    Image(const std::string & filename, int hot_x, int hot_y, 
          int act_x, int act_y);
    void load();
    void load_filename(const std::string & filename);
    void draw(double x, double y, double angle = 0.0);
};