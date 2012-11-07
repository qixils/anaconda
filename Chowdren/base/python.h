// I don't know why these are defined
#ifdef _MSC_VER
#undef HAVE_UNISTD_H
#undef HAVE_STDINT_H
#endif

#define Py_PYMATH_H
#include <Python.h>

// macros for event handling

#define GET_PYTHON_ARG(i) PyTuple_GetItem(python_args, i)

class PythonInterpreter : public FrameObject
{
public:
    static PyObject * main_module;
    static PyObject * interface_module;
    static PyObject * functions_module;
    PyObject * parameters;
    PyObject * returns;

    static void initialize()
    {
        static bool initialized = false;
        if (initialized)
            return;
        initialized = true;
        Py_NoSiteFlag = 1;
        Py_SetPythonHome(".");
        Py_Initialize();
        main_module = PyImport_AddModule("__main__");
        interface_module = Py_InitModule("interface", NULL);
        functions_module = PyModule_New("functions");
        PyModule_AddObject(interface_module, "functions", functions_module);
    }

    static void add_function(const char * name, void * fp)
    {
        initialize();
        PyCFunction f = (PyCFunction)fp;
        PyMethodDef * methd = new PyMethodDef;
        methd->ml_name = name;
        methd->ml_meth = f;
        methd->ml_flags = METH_VARARGS;
        methd->ml_doc = NULL;
        PyObject * py_f = PyCFunction_New(methd, NULL);
        PyModule_AddObject(functions_module, name, py_f);
        print_errors();
    }

    PythonInterpreter(std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id), parameters(NULL), returns(NULL)
    {
        initialize();
    }

    static void print_errors()
    {
        if (PyErr_Occurred() != NULL) {
            std::cout << "Error!" << std::endl;
            PyErr_Print();
        }
    }

    void run_string(std::string str)
    {
        str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
        PyObject * globals = PyModule_GetDict(main_module);
        PyObject * result = PyRun_String(str.c_str(), Py_file_input, 
            globals, globals);
        print_errors();
    }

    void add_parameter(PyObject * v)
    {
        if (parameters == NULL)
            parameters = PyList_New(0);
        PyList_Append(parameters, v);
    }

    void add_parameter(double v)
    {
        add_parameter(to_object(v));
    }

    static inline PyObject * to_object(double value)
    {
        return *((PyObject**)&value);
    }

    static inline PyObject * to_object(PyObject * value)
    {
        return value;
    }

    static double to_double(PyObject * value)
    {
        return *((double*)&value);
    }

    static PyObject * create_object(const std::string & v)
    {
        return PyString_FromStringAndSize(v.data(), v.size());
    }

    static PyObject * create_object(int value)
    {
        return PyInt_FromLong(value);
    }

    static PyObject * eval(std::string str)
    {
        str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
        PyObject * globals = PyModule_GetDict(main_module);
        PyObject * result = PyRun_String(str.c_str(), Py_eval_input, 
            globals, globals);
        print_errors();
        return result;
    }

    static double list_append(double listd, PyObject * value)
    {
        PyObject * list = to_object(listd);
        PyList_Append(list, value);
        return listd;
    }

    static double create_list()
    {
        return to_double(PyList_New(0));
    }

    void call_global(const std::string & name)
    {
/*        std::cout << "Calling " << name << " from application" << std::endl;*/
        print_errors();
        PyObject * globals = PyModule_GetDict(main_module);
        const char * function_name = name.c_str();
        PyObject * function = PyRun_String(function_name, Py_eval_input, 
            globals, globals);
        if (function == NULL) {
            PyErr_Format(PyExc_AttributeError,
                "no global exists with the name '%s'", function_name);
            print_errors();
            return;
        }

        PyObject * parameters;
        if (this->parameters == NULL) {
            parameters = PyTuple_New(0);
        } else {
            parameters = PyList_AsTuple(this->parameters);
            Py_DECREF(this->parameters);
        }
        this->parameters = NULL;
        Py_XDECREF(returns);
        PyObject * result = PyObject_Call(function, parameters, NULL);

        if (result == NULL) {
            Py_INCREF(Py_None);
            returns = Py_None;
            print_errors();
        } else {
            if (!PyTuple_Check(result)) {
                returns = Py_BuildValue("(O)", result);
            } else {
                returns = result;
            }
        }

/*        std::cout << "Call for " << name << " returning to application" << std::endl;*/
        Py_DECREF(parameters);
    }

    std::string as_string(PyObject * v)
    {
        char * buf;
        Py_ssize_t len;
        PyString_AsStringAndSize(PyObject_Str(v), &buf, &len);
        return std::string(buf, len);
    }

    int as_number(PyObject * v)
    {
        return PyInt_AsLong(v);
    }

    std::string as_string(double v)
    {
        return as_string(to_object(v));
    }
};

PyObject * PythonInterpreter::main_module = NULL;
PyObject * PythonInterpreter::interface_module = NULL;
PyObject * PythonInterpreter::functions_module = NULL;