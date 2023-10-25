
// using https://docs.python.org/3/extending/extending.html as template
// these lines must come first
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <iostream>
#include <algorithm>
#include <ostream>
#include <utility>
#include <vector>
#include <string>
#include <sstream>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

// #include <EGL/egl.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_hints.h>

#include <projectM-4/playlist.h>
#include <projectM-4/projectM.h>

#include <numpy/arrayobject.h>  // For the numpy C-API

// glfw
// #include <GLFW/glfw3.h>

// egl
#include <EGL/egl.h>

using namespace std;

static PyObject*
winamp_visual_systemcall(PyObject* self, PyObject* args) {
    const char *command;
    if (!PyArg_ParseTuple(args, "s", &command)) { return NULL; }
    return PyLong_FromLong(system(command));
}


projectm_handle _projectM{nullptr};
projectm_playlist_handle _playlist{nullptr};
void audioInputCallbackF32(void *userdata, unsigned char *stream, int len) {
    // projectm_handle *_projectM = (projectm_handle *) userdata;

    // TODO look into this
    projectm_pcm_add_float(_projectM, reinterpret_cast<float*>(stream), len/sizeof(float)/2, PROJECTM_STEREO);
    // projectm_pcm_add_float(_projectM, reinterpret_cast<float*>(stream), len/sizeof(float)/2, PROJECTM_MONO);

    // if (_audioChannelsCount == 1)
    //     projectm_pcm_add_float(_projectM, reinterpret_cast<float*>(stream), len/sizeof(float)/2, PROJECTM_MONO);
    // else if (_audioChannelsCount == 2)
    //     projectm_pcm_add_float(_projectM, reinterpret_cast<float*>(stream), len/sizeof(float)/2, PROJECTM_STEREO);
    // else {
    //     SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Multichannel audio not supported");
    //     SDL_Quit();
    // }
}



int initAudioInput(int selected_device) {
    SDL_AudioSpec want, have;

    // requested format
    // https://wiki.libsdl.org/SDL_AudioSpec#Remarks
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_F32;  // float
    want.channels = 2;  // mono might be better?
    want.samples = want.freq / 60;
    want.callback = audioInputCallbackF32;
    want.userdata = _projectM;

    // index -1 means "system deafult", which is used if we pass deviceName == NULL
    const char *deviceName = selected_device == -1 ? NULL : SDL_GetAudioDeviceName(selected_device, true);
    if (deviceName == NULL) {
        std::cout << "python/c++: WARNING Wasnt able to see the device id: " << selected_device << std::endl;
        return -1;
    }
    else {
        std::cout << "python/c++: Opening audio capture device ACTUALLY: " << deviceName << std::endl;
    }
    int audioDeviceId = SDL_OpenAudioDevice(deviceName, true, &want, &have, 0);

    if (audioDeviceId == 0) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to open audio capture device: %s", SDL_GetError());
        return -1;
    }

    // read characteristics of opened capture device
    if(deviceName == NULL)
        deviceName = "<System default capture device>";
    // SDL_Log("Opened audio capture device index=%i devId=%i: %s", selected_device, _audioDeviceId, deviceName);
    std::cout << "Opened audio capture device index=" << selected_device << " devId=" << audioDeviceId << ": " << deviceName << std::endl; 
    // std::string deviceToast = deviceName; // Example: Microphone rear
    // deviceToast += " selected";
    // SDL_Log("Samples: %i, frequency: %i, channels: %i, format: %i", have.samples, have.freq, have.channels, have.format);
    // _audioChannelsCount = have.channels;
    return audioDeviceId;
}

void openAudioInput() {
    const char* driver_name = SDL_GetCurrentAudioDriver();
    std::cout << "python/c++: Using audio driver: " << driver_name << SDL_GetError() << std::endl;


    unsigned int _numAudioDevices = SDL_GetNumAudioDevices(true);
    std::cout << "python/c++: Found " << _numAudioDevices << " audio capture devices" << std::endl;
    for (unsigned int i = 0; i < _numAudioDevices; i++) {
        std::cout << "python/c++: Found audio capture device: " << i << ", " << SDL_GetAudioDeviceName(i, true) << std::endl;
    }

    // We start with the system default capture device (index -1).
    // Note: this might work even if NumAudioDevices == 0 (example: if only a
    // monitor device exists, and SDL_HINT_AUDIO_INCLUDE_MONITORS is not set).
    // So we always try it, and revert to fakeAudio if the default fails _and_ NumAudioDevices == 0.
    
    if (_numAudioDevices == 0) {
        std::cout << "python/c++: No audio capture devices found" << std::endl;

    }

    int device_id_to_open = 3; // henry monitor on andrew arch linux
    int actual_audio_device_id_opened = initAudioInput(device_id_to_open);
    if (actual_audio_device_id_opened != -1) {
        std::cout << "python/c++: Opened audio capture device" << std::endl;
        SDL_PauseAudioDevice(actual_audio_device_id_opened, false);
    }
    else {
        std::cout << "python/c++: Failed to open audio capture device" << std::endl;
    }
}

// when it ends...
// void endAudioCapture() {
//     SDL_PauseAudioDevice(_audioDeviceId, true);
//     SDL_CloseAudioDevice(_audioDeviceId);
// }


static PyObject*
winamp_visual_setup_winamp(PyObject* self, PyObject* args) {


    SDL_version linked;
    SDL_GetVersion(&linked);
    // std::cout << "C++ - Python Extension: Using SDL version " << linked.major << "." << linked.minor << "." << linked.patch << "\n";
    SDL_Log("Using SDL version %d.%d.%d\n", linked.major, linked.minor, linked.patch);


    SDL_SetHint(SDL_HINT_AUDIO_INCLUDE_MONITORS, "1"); // this allows listening to speakers

    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        SDL_Log("Unable to initialize SDL AUDIO: %s", SDL_GetError());
    }
    openAudioInput();


    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL VIDEO: %s", SDL_GetError());
    }
    
    std::cout << "C++ - Python Extension: setting up winamp" << std::endl;
    _projectM = projectm_create();
    projectm_set_window_size(_projectM, 32, 20);


    std::cout << "C++ - Python Extension: setting up sdl window" << std::endl;


    // SDL
    SDL_Window* window = SDL_CreateWindow("", 0, 0, 32, 20, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    SDL_GL_CreateContext(window);


    // GLFW
    // int return_val = glfwInit();
    // if (return_val == GLFW_FALSE) {
    //     std::cout << "C++ - Python Extension: glfwInit() failed" << std::endl;
    // }
    // glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);  // Make window hidden
    // GLFWwindow* window = glfwCreateWindow(640, 480, "", NULL, NULL);
    // glfwMakeContextCurrent(window);


    // EGL
    // EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    // if (eglGetError() != EGL_SUCCESS) {
    //     cout << "C++ - Python Extension: eglGetDisplay() failed" << endl;
    // }
    
    // EGLint major, minor;
    // if (!eglInitialize(display, &major, &minor)) {
    //     cout << "C++ - Python Extension: eglInitialize() failed" << endl;
    //     exit(1);
    // }

    // EGLint configAttribs[] = {
    //     EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    //     EGL_BLUE_SIZE, 8,
    //     EGL_GREEN_SIZE, 8,
    //     EGL_RED_SIZE, 8,
    //     EGL_DEPTH_SIZE, 8,
    //     EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    //     EGL_NONE
    // };
    // EGLConfig config;
    // EGLint numConfigs;
    // eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);

    // EGLint pbufferAttribs[] = {
    //     EGL_WIDTH, 640,
    //     EGL_HEIGHT, 480,
    //     EGL_NONE,
    // };
    // EGLSurface surface = eglCreatePbufferSurface(display, config, pbufferAttribs);

    // EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);
    // eglMakeCurrent(display, surface, surface, context);

    cout << "C++ - Python Extension: AFTER SETUP" << endl;
    return Py_BuildValue("");
}


static PyObject*
winamp_visual_load_preset(PyObject* self, PyObject* args) {
    // std::cout << "C++ - Python Extension: loading preset" << std::endl;

    char* preset_path_c_str;
    if(!PyArg_ParseTuple(args, "s", &preset_path_c_str)) {
        return NULL;
    }
    // cout << "C++ - Python Extension: before loading preset: " << preset_path_c_str << endl;

    projectm_load_preset_file(_projectM, preset_path_c_str, false);
    return Py_BuildValue("");
}

static PyObject*
winamp_visual_render_frame(PyObject* self, PyObject* args) {
    projectm_opengl_render_frame(_projectM);
    return Py_BuildValue("");
}

static PyObject*
winamp_visual_print_to_terminal_higher_level(PyObject* self, PyObject* args) {
    int grab_width = projectm_get_grab_width(_projectM);
    int grab_height = projectm_get_grab_height(_projectM);
    GLubyte* andrew_pixels = projectm_get_andrew_pixels(_projectM);
    // cout << "grab_width: " << grab_width << ", grab_height" << projectm_get_grab_height << endl;

    std::stringstream ss;
    for (int y = 0; y < grab_height; y++) {
        for (int x = 0; x < grab_width; x++) {
            int index = (y * grab_width + x) * 4;
            int r = andrew_pixels[index];
            int g = andrew_pixels[index + 1];
            int b = andrew_pixels[index + 2];
            // int a = andrew_pixels[index + 3];
            ss << "\033[48;2;" << r << ";" << g << ";" << b << "m  \033[0m";
        }
        ss << std::endl;
    }

    ss << "Using grab_width / height:" << grab_width << ", " << grab_height << endl;
    std::cout << ss.str();

    std::cout << "\033[" << grab_height + 2 << "A" << std::endl;
    return Py_BuildValue("");
}


static PyObject*
winamp_visual_set_beat_sensitivity(PyObject* self, PyObject* args) {
    // load double from arguments 
    float beat_sensitivity;
    if(!PyArg_ParseTuple(args, "f", &beat_sensitivity)) {
        return NULL;
    }
    projectm_set_beat_sensitivity(_projectM, beat_sensitivity);
    return Py_BuildValue("");
}


static PyObject*
winamp_visual_get_beat_sensitivity(PyObject* self, PyObject* args) {
    float beat_sensitivity = projectm_get_beat_sensitivity(_projectM);
    return Py_BuildValue("f", beat_sensitivity);
}


// in python:
// grid = np.array(np.zeros((GRID_WIDTH, GRID_HEIGHT, 3)), np.double)

static PyObject*
winamp_visual_load_into_numpy_array(PyObject* self, PyObject* args) {
    int grab_width = projectm_get_grab_width(_projectM);
    int grab_height = projectm_get_grab_height(_projectM);
    GLubyte* andrew_pixels = projectm_get_andrew_pixels(_projectM);

    // !TODO consider making numpy array: NPY_UBYTE, idk why its a double

    // get numpy array from arguments
    PyArrayObject* numpy_array;
    if(!PyArg_ParseTuple(args, "O!", &PyArray_Type, &numpy_array)) {
        return NULL;
    }

    // numpy_array will be shape (20, 32, 3)
    // andrew_pixels is shape (32, 20, 4)
    // grab_width: 32, grab_height: 20

    // Check if the array shape matches the expected shape
    npy_intp* dims = PyArray_DIMS(numpy_array);
    if (dims[0] != grab_height || dims[1] != grab_width || dims[2] != 3) {
        PyErr_SetString(PyExc_ValueError, "numpy_array shape mismatch");
        return NULL;
    }

    // Ensure the numpy array is of the right type
    if (PyArray_TYPE(numpy_array) != NPY_DOUBLE) {
        PyErr_SetString(PyExc_ValueError, "numpy_array must be of type uint8");
        return NULL;
    }

    // load from andrew_pixels (RGBA values) into numpy_array (RGB)
    // remember that andrew_pixels is ubyte, and we need to load into numpy correctly
    for (int y = 0; y < grab_height; y++) {
        for (int x = 0; x < grab_width; x++) {
            int index = (y * grab_width + x) * 4;  // Index for andrew_pixels (RGBA)
            
            GLubyte* pixel = andrew_pixels + index;

            // Compute the numpy array's position
            double* numpy_pixel = (double*)PyArray_DATA(numpy_array) + (y * grab_width * 3 + x * 3);

            // Convert and normalize RGBA to RGB (normalized to the range [0, 100])
            numpy_pixel[0] = (double)pixel[0] / 2.55;  // R
            numpy_pixel[1] = (double)pixel[1] / 2.55;  // G
            numpy_pixel[2] = (double)pixel[2] / 2.55;  // B
        }
    }
    return Py_BuildValue("");
}

    // GLubyte* numpy_data = (GLubyte*) PyArray_DATA(numpy_array);
    // for (int y = 0; y < grab_height; y++) {
    //     for (int x = 0; x < grab_width; x++) {
    //         int andrew_pixels_index = (y * grab_width + x) * 4;
    //         int numpy_index = (y * grab_width + x) * 3;
    //         numpy_data[numpy_index] = andrew_pixels[andrew_pixels_index];
    //         numpy_data[numpy_index + 1] = andrew_pixels[andrew_pixels_index + 1];
    //         numpy_data[numpy_index + 2] = andrew_pixels[andrew_pixels_index + 2];
    //     }
    // }



static PyObject*
winamp_visual_print_to_terminal(PyObject* self, PyObject* args) {
    projectm_print_to_terminal(_projectM);

    return Py_BuildValue("");
}

static PyMethodDef winamp_visual_methods[] = {
    {"systemcall",  winamp_visual_systemcall, METH_VARARGS, ""},
    {"setup_winamp", winamp_visual_setup_winamp, METH_VARARGS, ""},
    {"load_preset", winamp_visual_load_preset, METH_VARARGS, ""},
    {"render_frame", winamp_visual_render_frame, METH_VARARGS, ""},
    {"set_beat_sensitivity", winamp_visual_set_beat_sensitivity, METH_VARARGS, ""},
    {"get_beat_sensitivity", winamp_visual_get_beat_sensitivity, METH_VARARGS, ""},
    {"print_to_terminal", winamp_visual_print_to_terminal, METH_VARARGS, ""},
    {"print_to_terminal_higher_level", winamp_visual_print_to_terminal_higher_level, METH_VARARGS, ""},
    {"load_into_numpy_array", winamp_visual_load_into_numpy_array, METH_VARARGS, ""},
    {NULL, NULL, 0, NULL},
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
    import_array();
    return PyModule_Create(&winamp_visualmodule);
}
