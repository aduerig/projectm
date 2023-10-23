#include <algorithm>
#include <ostream>
#include <iostream>
#include <utility>
#include <vector>
#include <string>


#include <SDL2/SDL.h>
#include <SDL2/SDL_hints.h>

using namespace std;

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


    int _numAudioDevices = SDL_GetNumAudioDevices(true);  // capture, please
    std::cout << "python/c++: Found " << _numAudioDevices << " audio capture devices" << std::endl;
    for (int i = 0; i < _numAudioDevices; i++) {
        std::cout << "python/c++: Found audio capture device: " << i << ", " << SDL_GetAudioDeviceName(i, true) << std::endl;
    }

    // We start with the system default capture device (index -1).
    // Note: this might work even if NumAudioDevices == 0 (example: if only a
    // monitor device exists, and SDL_HINT_AUDIO_INCLUDE_MONITORS is not set).
    // So we always try it, and revert to fakeAudio if the default fails _and_ NumAudioDevices == 0.
    
    if (_numAudioDevices == 0) {
        std::cout << "python/c++: No audio capture devices found" << std::endl;

    }

    int actual_device_opened = initAudioInput(4);
    if (actual_device_opened != -1) {
        std::cout << "python/c++: Opened audio capture device" << std::endl;
        SDL_PauseAudioDevice(actual_device_opened, false);
    }
    else {
        std::cout << "python/c++: Failed to open audio capture device" << std::endl;
    }
}

int main() {


    // if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    //     SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    // }
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
    }

    // real is:     2.28.4
    // ours is also 2.28.4
    SDL_version linked;
    SDL_GetVersion(&linked);
    // std::cout << "C++ - Python Extension: Using SDL version " << linked.major << "." << linked.minor << "." << linked.patch << "\n";
    SDL_Log("Using SDL version %d.%d.%d\n", linked.major, linked.minor, linked.patch);

    std::cout << "C++ - Python Extension: after up winamp" << std::endl;
    std::cout << "C++ - Python Extension: setting up sdl OR glfw window" << std::endl;

    SDL_Window* window = SDL_CreateWindow("", 0, 0, 32, 20, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    SDL_GL_CreateContext(window);

    openAudioInput();
}
