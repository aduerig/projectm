#include "setup.hpp"

#include <SDL2/SDL_hints.h>

#include <chrono>
#include <cmath>

#if OGL_DEBUG
void debugGL(GLenum source,
             GLenum type,
             GLuint id,
             GLenum severity,
             GLsizei length,
             const GLchar* message,
             const void* userParam) {

    /*if (type != GL_DEBUG_TYPE_OTHER)*/
    {
        std::cerr << " -- \n" << "Type: " <<
        type << "; Source: " <<
        source <<"; ID: " << id << "; Severity: " <<
        severity << "\n" << message << "\n";
    }
}
#endif

void initGL() {
#if USE_GLES
    // use GLES 2.0 (this may need adjusting)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    // Disabling compatibility profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
}

void dumpOpenGLInfo() {
    SDL_Log("- GL_VERSION: %s", glGetString(GL_VERSION));
    SDL_Log("- GL_SHADING_LANGUAGE_VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    SDL_Log("- GL_VENDOR: %s", glGetString(GL_VENDOR));
}


void enableGLDebugOutput() {
#if OGL_DEBUG && !USE_GLES
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugGL, NULL);
#endif
}

// initialize SDL, openGL, config
projectMSDL *setupSDLApp() {
    projectMSDL *app;
        
#ifdef SDL_HINT_AUDIO_INCLUDE_MONITORS
    SDL_SetHint(SDL_HINT_AUDIO_INCLUDE_MONITORS, "1");
#endif

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    // default window size to usable bounds (e.g. minus menubar and dock)
    SDL_Rect initialWindowBounds;

    // int width = initialWindowBounds.w;
    // int height = initialWindowBounds.h;
    int width = 32;
    int height = 20;

    initGL();

    SDL_Window *win = SDL_CreateWindow("projectM", 0, 0, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_GL_GetDrawableSize(win,&width,&height);

    SDL_GLContext glCtx = SDL_GL_CreateContext(win);

    dumpOpenGLInfo();

    SDL_SetWindowTitle(win, "projectM");

    SDL_GL_MakeCurrent(win, glCtx);  // associate GL context with main window
    int avsync = SDL_GL_SetSwapInterval(-1); // try to enable adaptive vsync
    if (avsync == -1) { // adaptive vsync not supported
        SDL_GL_SetSwapInterval(1); // enable updates synchronized with vertical retrace
    }

    std::string base_path = DATADIR_PATH;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Using data directory: %s\n", base_path.c_str());

    // load configuration file
    std::string presetURL = base_path + "/presets";

    app = new projectMSDL(glCtx, presetURL);

    // center window and full desktop screen
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
        SDL_SetWindowPosition(win, dm.w / 2, dm.h / 2);
    } else {
        SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
        SDL_SetWindowPosition(win, initialWindowBounds.x, initialWindowBounds.y);
    }
    SDL_SetWindowSize(win, width, height);
    app->resize(width, height);

    std::string modKey = "CTRL";
    app->init(win);

    enableGLDebugOutput();

    // INFO: Found audio capture device 0: Monitor of GA102 High Definition Audio Controller Digital Stereo (HDMI)
    // INFO: Found audio capture device 1: Monitor of FIFINE K678 Microphone Analog Stereo
    // INFO: Found audio capture device 2: FIFINE K678 Microphone Analog Stereo
    // INFO: Found audio capture device 3: Monitor of Starship/Matisse HD Audio Controller Digital Stereo (IEC958)
    // INFO: Found audio capture device 4: Starship/Matisse HD Audio Controller Analog Stereo
    // INFO: Found audio capture device 5: Monitor of henry
    // SDL_AudioInit("Monitor of henry");
    SDL_version linked;
    SDL_GetVersion(&linked);
    SDL_Log("Using SDL version %d.%d.%d\n", linked.major, linked.minor, linked.patch);

#if !FAKE_AUDIO
    // get an audio input device
    if (app->openAudioInput())
        app->beginAudioCapture();
#endif

    return app;
}


    //     // projectm_set_mesh_size(projectMHandle, config.read<size_t>("Mesh X", 32), config.read<size_t>("Mesh Y", 24));
    //     // SDL_SetWindowSize(win, config.read<size_t>("Window Width", 1024), config.read<size_t>("Window Height", 768));
    //     // projectm_set_soft_cut_duration(projectMHandle, config.read<double>("Smooth Preset Duration", config.read<int>("Smooth Transition Duration", 3)));
    //     // projectm_set_preset_duration(projectMHandle, config.read<double>("Preset Duration", 30));
    //     // projectm_set_easter_egg(projectMHandle, config.read<float>("Easter Egg Parameter", 0.0));
    //     // projectm_set_hard_cut_enabled(projectMHandle,  config.read<bool>("Hard Cuts Enabled", false));
    //     // projectm_set_hard_cut_duration(projectMHandle, config.read<double>("Hard Cut Duration", 60));
    //     // projectm_set_hard_cut_sensitivity(projectMHandle, config.read<float>("Hard Cut Sensitivity", 1.0));
    //     // projectm_set_beat_sensitivity(projectMHandle, config.read<float>("Beat Sensitivity", 1.0));
    //     // projectm_set_aspect_correction(projectMHandle, config.read<bool>("Aspect Correction", true));
    //     // projectm_set_fps(projectMHandle, config.read<int32_t>("FPS", 60));

    //     // app->setFps(config.read<size_t>("FPS", 60));