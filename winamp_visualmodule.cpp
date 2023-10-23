// using https://docs.python.org/3/extending/extending.html as template
// these lines must come first
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <algorithm>
#include <ostream>
#include <iostream>
#include <utility>

#include "my_sdl.cpp"

using namespace std;

static PyObject*
winamp_visual_systemcall(PyObject* self, PyObject* args) {
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command)) {
        return NULL;
    }

    sts = system(command);
    return PyLong_FromLong(sts);
}

static PyObject*
winamp_visual_print_from_c(PyObject* self, PyObject* args) {
    cout << "C++ - Python Extension: this is from C" << endl;
    return Py_BuildValue(""); // this is None in Python
}

projectm_handle _projectM{nullptr};
projectm_playlist_handle _playlist{nullptr};
static PyObject*
winamp_visual_setup_winamp(PyObject* self, PyObject* args) {
    std::cout << "C++ - Python Extension: setting up winamp" << std::endl;
    _projectM = projectm_create();
    std::cout << "C++ - Python Extension: after up winamp" << std::endl;
    return Py_BuildValue("");
}


static PyObject*
winamp_visual_load_preset(PyObject* self, PyObject* args) {
    std::cout << "C++ - Python Extension: loading preset" << std::endl;

    // get std::string
    char* preset_path_c_str;
    if(!PyArg_ParseTuple(args, "s", &preset_path_c_str)) {
        return NULL;
    }
    // this crashes???
    // std::string preset_path(preset_path_c_str);

    projectm_load_preset_file(_projectM, preset_path_c_str, false);
    // auto projectMInstance = reinterpret_cast<projectMWrapper*>(_projectM);
    // auto projectMInstance = handle_to_instance(_projectM);
    // projectMInstance->LoadPresetFile(preset_path, false);
    return Py_BuildValue("");
}

static PyObject*
winamp_visual_render_frame(PyObject* self, PyObject* args) {
    projectm_opengl_render_frame(_projectM);
    return Py_BuildValue("");
}

static PyMethodDef winamp_visual_methods[] = {
    {"systemcall",  winamp_visual_systemcall, METH_VARARGS, ""},
    {"print_from_c",  winamp_visual_print_from_c, METH_VARARGS, ""},
    {"setup_winamp", winamp_visual_setup_winamp, METH_VARARGS, ""},
    {"load_preset", winamp_visual_load_preset, METH_VARARGS, ""},
    {"render_frame", winamp_visual_render_frame, METH_VARARGS, ""},
};

static struct PyModuleDef winamp_visualmodule = {
    PyModuleDef_HEAD_INIT,
    "winamp_visual",   /* name of module */
    NULL, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    winamp_visual_methods
};

PyMODINIT_FUNC
PyInit_winamp_visual(void) {
    return PyModule_Create(&winamp_visualmodule);
}