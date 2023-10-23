git submodule update --init --recursive
    for eval



other frontend
    https://github.com/kblaschke/frontend-sdl2


cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_SDL_UI=ON
cmake --build . -- -j

src/sdl-test-ui/projectM-Test-UI



rm CMakeCache.txt; cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_SDL_UI=ON && cmake --build . -- -j && src/sdl-test-ui/projectM-Test-UI

static:
    rm CMakeCache.txt; cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=OFF -DENABLE_SDL_UI=ON && cmake --build . -- -j && src/sdl-test-ui/projectM-Test-UI

INFO: Displaying preset: ./presets/presets-cream-of-the-crop/Waveform/Wire Tunnel/stahlregen + geiss + martin - the origin of galaxies.milk


milkdroppreset.cpp
lBlitFramebuffer(0, 0, renderContext.viewportSizeX, renderContext.vie

    int width = initialWindowBounds.w;
    int height = initialWindowBounds.h;

SDL_Log("Found audio capture device %d: %s", i, SDL_GetAudioDeviceName(i, true));


winners;
INFO: Displaying preset: ./presets/presets-cream-of-the-crop/Reaction/Liquid Ripples/suksma - satanic teleprompter - nothing has will, stop pretending.milk

INFO: Displaying preset: ./presets/presets-cream-of-the-crop/Supernova/Stars/324.milk

INFO: Displaying preset: ./presets/presets-cream-of-the-crop/Supernova/Radiate/$$$ Royal - Mashup (355).milk




benchmarking
perf record -g src/sdl-test-ui/projectM-Test-UI
perf report -g 'graph,0.5,caller'