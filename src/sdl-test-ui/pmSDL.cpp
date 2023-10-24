#include "pmSDL.hpp"

#include <vector>

using namespace std;

projectMSDL::projectMSDL(SDL_GLContext glCtx, const string& presetPath)
    : _openGlContext(glCtx)
    , _projectM(projectm_create())
    , _playlist(projectm_playlist_create(_projectM)) {
    projectm_get_window_size(_projectM, &_width, &_height);
    projectm_playlist_set_preset_switched_event_callback(_playlist, &projectMSDL::presetSwitchedEvent, static_cast<void*>(this));
    projectm_playlist_add_path(_playlist, presetPath.c_str(), true, false);
}

projectMSDL::~projectMSDL() {
    projectm_playlist_destroy(_playlist);
    _playlist = nullptr;
    projectm_destroy(_projectM);
    _projectM = nullptr;
}

void projectMSDL::scrollHandler(SDL_Event* sdl_evt) {
    if (sdl_evt->wheel.y > 0) { // handle mouse scroll wheel - up++
        projectm_playlist_play_previous(_playlist, true);
    }

    if (sdl_evt->wheel.y < 0) { // handle mouse scroll wheel - down--
        projectm_playlist_play_next(_playlist, true);
    }
}

void projectMSDL::keyHandler(SDL_Event* sdl_evt) {
    SDL_Keymod sdl_mod = (SDL_Keymod) sdl_evt->key.keysym.mod;
    SDL_Keycode sdl_keycode = sdl_evt->key.keysym.sym;

    if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL) // Left or Right Gui or Left Ctrl
        keymod = true;

    // handle keyboard input (for our app first, then projectM)
    switch (sdl_keycode)
    {
        // cout << sdl_keycode << endl;
        case SDLK_q:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                done = 1;
                return;
            }
            break;

        case SDLK_i:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                toggleAudioInput();
                return; // handled
            }
            break;

        case SDLK_LEFT:
            cout << "pmSDL: play previous (left key)" << endl;
            projectm_playlist_play_previous(_playlist, true);
            break;

        case SDLK_RIGHT:
            projectm_playlist_play_next(_playlist, true);
            break;

        case SDLK_UP:
            projectm_set_beat_sensitivity(_projectM, projectm_get_beat_sensitivity(_projectM) + 0.01f);
            cout << "pmSDL: increase beat sensitivity" << projectm_get_beat_sensitivity(_projectM) << endl << endl;
            break;

        case SDLK_DOWN:
            projectm_set_beat_sensitivity(_projectM, projectm_get_beat_sensitivity(_projectM) - 0.01f);
            cout << "pmSDL: decrease beat sensitivity" << projectm_get_beat_sensitivity(_projectM) << endl << endl;
            break;

    }
}

void projectMSDL::resize(unsigned int width_, unsigned int height_) {
    _width = width_;
    _height = height_;

    // Hide cursor if window size equals desktop size
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
        SDL_ShowCursor(SDL_DISABLE);
    }

    projectm_set_window_size(_projectM, _width, _height);
}

void projectMSDL::pollEvent() {
    SDL_Event evt;

    int mousex = 0;
    float mousexscale = 0;
    int mousey = 0;
    float mouseyscale = 0;
    int mousepressure = 0;
    while (SDL_PollEvent(&evt)) {
        switch (evt.type) {
            case SDL_WINDOWEVENT:
                int h, w;
                SDL_GL_GetDrawableSize(_sdlWindow, &w, &h);
                switch (evt.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                        resize(w, h);
                        break;
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        resize(w, h);
                        break;
                }
                break;
            case SDL_MOUSEWHEEL:
                scrollHandler(&evt);

            case SDL_KEYDOWN:
                keyHandler(&evt);
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (evt.button.button == SDL_BUTTON_LEFT) {
                    if (!mouseDown) {
                        SDL_GetMouseState(&mousex, &mousey);
                        // Scale those coordinates. libProjectM supports a scale of 0.1 instead of absolute pixel coordinates.
                        mousexscale = (mousex / (float) _width);
                        mouseyscale = ((_height - mousey) / (float) _height);
                        touch(mousexscale, mouseyscale, mousepressure); // Touch. By not supplying a touch type, we will default to random.
                        mouseDown = true;
                    }
                }
                else if (evt.button.button == SDL_BUTTON_RIGHT) {
                    mouseDown = false;
                    if (keymod) { // Keymod = Left or Right Gui or Left Ctrl. This is a shortcut to remove all waveforms.
                        touchDestroyAll();
                        keymod = false;
                        break;
                    }

                    SDL_GetMouseState(&mousex, &mousey); // Right Click
                    mousexscale = (mousex / (float) _width);
                    mouseyscale = ((_height - mousey) / (float) _height);
                    touchDestroy(mousexscale, mouseyscale);
                }
                break;

            case SDL_MOUSEBUTTONUP:
                mouseDown = false;
                break;

            case SDL_QUIT:
                done = true;
                break;
        }
    }

    if (mouseDown)
    {
        SDL_GetMouseState(&mousex, &mousey);
        mousexscale = (mousex / (float) _width);
        mouseyscale = ((_height - mousey) / (float) _height);
        touchDrag(mousexscale, mouseyscale, mousepressure);
    }
}

// This touches the screen to generate a waveform at X / Y.
void projectMSDL::touch(float x, float y, int pressure, int touchtype) {
#ifdef PROJECTM_TOUCH_ENABLED
    projectm_touch(_projectM, x, y, pressure, static_cast<projectm_touch_type>(touchtype));
#endif
}

// This moves the X Y of your existing waveform that was generated by a touch (only if you held down your click and dragged your mouse around).
void projectMSDL::touchDrag(float x, float y, int pressure) {
    projectm_touch_drag(_projectM, x, y, pressure);
}

// Remove waveform at X Y
void projectMSDL::touchDestroy(float x, float y) {
    projectm_touch_destroy(_projectM, x, y);
}

// Remove all waveforms
void projectMSDL::touchDestroyAll() {
    projectm_touch_destroy_all(_projectM);
}

void projectMSDL::renderFrame() {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    projectm_opengl_render_frame(_projectM);
    projectm_print_to_terminal(_projectM);

    SDL_GL_SwapWindow(_sdlWindow);
}

void projectMSDL::init(SDL_Window* window, const bool _renderToTexture) {
    _sdlWindow = window;
    projectm_set_window_size(_projectM, _width, _height);
}

string projectMSDL::getActivePresetName() {
    unsigned int index = projectm_playlist_get_position(_playlist);
    if (index)
    {
        auto presetName = projectm_playlist_item(_playlist, index);
        string presetNameString(presetName);
        projectm_playlist_free_string(presetName);
        return presetNameString;
    }
    return {};
}

void projectMSDL::presetSwitchedEvent(bool isHardCut, unsigned int index, void* context) {
    auto app = reinterpret_cast<projectMSDL*>(context);
    auto presetName = projectm_playlist_item(app->_playlist, index);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Displaying preset: %s\n\n", presetName);

    app->_presetName = presetName;
    projectm_playlist_free_string(presetName);

    app->UpdateWindowTitle();
}

projectm_handle projectMSDL::projectM() {
    return _projectM;
}

void projectMSDL::setFps(size_t fps) {
    _fps = fps;
}

size_t projectMSDL::fps() const {
    return _fps;
}

void projectMSDL::UpdateWindowTitle() {
    string title = "projectM ➫ " + _presetName;
    if (projectm_get_preset_locked(_projectM))
    {
        title.append(" [locked]");
    }
    SDL_SetWindowTitle(_sdlWindow, title.c_str());
}
