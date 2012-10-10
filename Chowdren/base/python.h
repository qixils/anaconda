#include <Python.h>

class PythonInterpreter : public FrameObject
{
public:
    PythonInterpreter(std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id)
    {
        static bool initialized = false;
        if (!initialized) {
            initialized = true;
            Py_Initialize();
        }
        
    }
};