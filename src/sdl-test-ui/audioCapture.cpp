#include "audioCapture.hpp"
#include "pmSDL.hpp"


int projectMSDL::initAudioInput() {
    // params for audio input
    SDL_AudioSpec want, have;

    // requested format
    // https://wiki.libsdl.org/SDL_AudioSpec#Remarks
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_F32;  // float
    want.channels = 2;  // mono might be better?
    want.samples = want.freq / 60;
    want.callback = projectMSDL::audioInputCallbackF32;
    want.userdata = this;

    // index -1 means "system deafult", which is used if we pass deviceName == NULL
    const char *deviceName = _selectedAudioDevice == -1 ? NULL : SDL_GetAudioDeviceName(_selectedAudioDevice, true);
    _audioDeviceId = SDL_OpenAudioDevice(deviceName, true, &want, &have, 0);

    if (_audioDeviceId == 0) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to open audio capture device: %s", SDL_GetError());
        return 0;
    }

    // read characteristics of opened capture device
    if(deviceName == NULL)
        deviceName = "<System default capture device>";
    SDL_Log("Opened audio capture device index=%i devId=%i: %s", _selectedAudioDevice, _audioDeviceId, deviceName);
    std::string deviceToast = deviceName; // Example: Microphone rear
    deviceToast += " selected";
#ifdef DEBUG
    SDL_Log("Samples: %i, frequency: %i, channels: %i, format: %i", have.samples, have.freq, have.channels, have.format);
#endif
    _audioChannelsCount = have.channels;

    return 1;
}

void projectMSDL::audioInputCallbackF32(void *userdata, unsigned char *stream, int len) {
    // std::cout << "audioInputCallbackF32" << std::endl;
    projectMSDL *app = (projectMSDL *) userdata;
//    printf("\nLEN: %i\n", len);
//    for (int i = 0; i < 64; i++)
//        printf("%X ", stream[i]);
    // stream is (i think) samples*channels floats (native byte order) of len BYTES
    if (app->_audioChannelsCount == 1)
        projectm_pcm_add_float(app->_projectM, reinterpret_cast<float*>(stream), len/sizeof(float)/2, PROJECTM_MONO);
    else if (app->_audioChannelsCount == 2)
        projectm_pcm_add_float(app->_projectM, reinterpret_cast<float*>(stream), len/sizeof(float)/2, PROJECTM_STEREO);
    else {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Multichannel audio not supported");
        SDL_Quit();
    }
}

int projectMSDL::toggleAudioInput() {
    // trigger a toggle with CMD-I or CTRL-I
    this->endAudioCapture(); // end current audio capture.
    _curAudioDevice++; // iterate device index
    if (_curAudioDevice >= (int) _numAudioDevices) { // We reached outside the boundaries of available audio devices.
        _curAudioDevice = -1; // Return to the default audio device.
        if (_numAudioDevices == 0) // If WASAPI_LOOPBACK was not enabled and there is only the default audio device, it's pointless to toggle anything.
        {
            SDL_Log("Only the default audio capture device is available. There is nothing to toggle at this time.");
            return 1;
        }
        _selectedAudioDevice = _curAudioDevice;
        initAudioInput();
        this->beginAudioCapture();
    }
    else {
        // This is a normal scenario where we move forward in the audio device index.
        _selectedAudioDevice = _curAudioDevice;
        initAudioInput();
        this->beginAudioCapture();
    }
    return 1;
}

int projectMSDL::openAudioInput() {
    fakeAudio = false; // if we are opening an audio input then there is no need for fake audio.
    // get audio driver name (static)
    const char* driver_name = SDL_GetCurrentAudioDriver();
    SDL_Log("Using audio driver: %s\n", driver_name);

    // get audio input device
    _numAudioDevices = SDL_GetNumAudioDevices(true);  // capture, please
    std::cout << "python/c++: Found " << _numAudioDevices << " audio capture devices" << std::endl;

    for (unsigned int i = 0; i < _numAudioDevices; i++) {
        SDL_Log("Found audio capture device %d: %s", i, SDL_GetAudioDeviceName(i, true));
    }

    // We start with the system default capture device (index -1).
    // Note: this might work even if NumAudioDevices == 0 (example: if only a
    // monitor device exists, and SDL_HINT_AUDIO_INCLUDE_MONITORS is not set).
    // So we always try it, and revert to fakeAudio if the default fails _and_ NumAudioDevices == 0.
    
    // arch linux main computer andrew
    _curAudioDevice = 3;
    _selectedAudioDevice = 3;
    if(!initAudioInput() && _numAudioDevices == 0) {
        // the default device doesn't work, and there's no other device to try
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "No audio capture devices found");
        fakeAudio = true;
        return 0;
    }

    return 1;
}

void projectMSDL::beginAudioCapture() {
    // allocate a buffer to store PCM data for feeding in
    SDL_PauseAudioDevice(_audioDeviceId, false);
}

void projectMSDL::endAudioCapture() {
    SDL_PauseAudioDevice(_audioDeviceId, true);
    SDL_CloseAudioDevice(_audioDeviceId);
}

