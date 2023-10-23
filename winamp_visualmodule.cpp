// using https://docs.python.org/3/extending/extending.html as template
// these lines must come first
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <algorithm>
#include <ostream>
#include <iostream>
#include <utility>
#include <vector>
#include <string>

// #include <EGL/egl.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_hints.h>

#include <projectM-4/playlist.h>
#include <projectM-4/projectM.h>
// #include "my_sdl.cpp"

using namespace std;

static PyObject*
winamp_visual_systemcall(PyObject* self, PyObject* args) {
    const char *command;
    if (!PyArg_ParseTuple(args, "s", &command)) { return NULL; }
    return PyLong_FromLong(system(command));
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
    // want.callback = projectMSDL::audioInputCallbackF32;
    // want.userdata = this;

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
    int actual_device_opened = initAudioInput(device_id_to_open);
    if (actual_device_opened != -1) {
        std::cout << "python/c++: Opened audio capture device" << std::endl;
        SDL_PauseAudioDevice(actual_device_opened, false);
    }
    else {
        std::cout << "python/c++: Failed to open audio capture device" << std::endl;
    }
}


projectm_handle _projectM{nullptr};
projectm_playlist_handle _playlist{nullptr};
static PyObject*
winamp_visual_setup_winamp(PyObject* self, PyObject* args) {

    std::cout << "C++ - Python Extension: setting up winamp" << std::endl;
    _projectM = projectm_create();
    projectm_set_window_size(_projectM, 32, 20);



    // SDL 
    SDL_SetHint(SDL_HINT_AUDIO_INCLUDE_MONITORS, "1"); // this allows listening to speakers

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    }

    std::cout << "C++ - Python Extension: setting up sdl OR glfw window" << std::endl;


    SDL_Window* window = SDL_CreateWindow("", 0, 0, 32, 20, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    SDL_GL_CreateContext(window);

    openAudioInput();

    // GLFW
    // glfwInit();
    // glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);  // Make window hidden
    // GLFWwindow* window = glfwCreateWindow(640, 480, "", NULL, NULL);
    // glfwMakeContextCurrent(window);

    // EGL
    // EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    // eglInitialize(display, NULL, NULL);

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

    return Py_BuildValue("");
}


static PyObject*
winamp_visual_load_preset(PyObject* self, PyObject* args) {
    std::cout << "C++ - Python Extension: loading preset" << std::endl;

    char* preset_path_c_str;
    if(!PyArg_ParseTuple(args, "s", &preset_path_c_str)) {
        return NULL;
    }
    // this crashes???
    // std::string my_preset_path;
    // my_preset_path = preset_path_c_str;

    cout << "C++ - Python Extension: before loading preset: " << preset_path_c_str << endl;

    projectm_load_preset_file(_projectM, preset_path_c_str, false);
    return Py_BuildValue("");
}

static PyObject*
winamp_visual_render_frame(PyObject* self, PyObject* args) {
    projectm_opengl_render_frame(_projectM);
    return Py_BuildValue("");
}

static PyMethodDef winamp_visual_methods[] = {
    {"systemcall",  winamp_visual_systemcall, METH_VARARGS, ""},
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


// SDL_version linked;
// SDL_GetVersion(&linked);
// // std::cout << "C++ - Python Extension: Using SDL version " << linked.major << "." << linked.minor << "." << linked.patch << "\n";
// SDL_Log("Using SDL version %d.%d.%d\n", linked.major, linked.minor, linked.patch);
