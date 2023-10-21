if(NOT ENABLE_SDL_UI)
    return()
endif()

add_executable(projectM-Test-UI
        ConfigFile.cpp
        ConfigFile.h
        audioCapture.cpp
        audioCapture.hpp
        loopback.cpp
        loopback.hpp
        pmSDL.cpp
        pmSDL.hpp
        projectM_SDL_main.cpp
        setup.cpp
        setup.hpp
        )

target_link_libraries(projectM-Test-UI
        PRIVATE
        libprojectM::playlist
        GLM::GLM
        SDL2::SDL2
        SDL2::SDL2main
        ${CMAKE_DL_LIBS}
        )
