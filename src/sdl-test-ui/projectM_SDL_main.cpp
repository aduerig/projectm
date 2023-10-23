#include "pmSDL.hpp"

static int mainLoop(void *userData) {
    projectMSDL **appRef = (projectMSDL **)userData;
    auto app = *appRef;
        
    int fps = app->fps(); // frame rate limiter
    if (fps <= 0)
        fps = 24;
    const Uint32 frame_delay = 1000/fps;
    Uint32 last_time = SDL_GetTicks();
    
    while (! app->done) {
        app->renderFrame();
        app->pollEvent();
        Uint32 elapsed = SDL_GetTicks() - last_time;
        if (elapsed < frame_delay)
            SDL_Delay(frame_delay - elapsed);
        last_time = SDL_GetTicks();
    }
    return 0;
}

int main(int argc, char *argv[]) {
    projectMSDL *app = setupSDLApp();
    int status = mainLoop(&app);

    SDL_GL_DeleteContext(app->_openGlContext);
#if !FAKE_AUDIO
    app->endAudioCapture();
#endif
    delete app;
    return status;
}