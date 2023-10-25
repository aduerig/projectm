if(NOT ENABLE_SDL_UI)
    return()
endif()

add_executable(projectM-Test-UI
        audioCapture.cpp
        audioCapture.hpp
        pmSDL.cpp
        pmSDL.hpp
        projectM_SDL_main.cpp
        setup.cpp
        setup.hpp
        )


execute_process(COMMAND hostname OUTPUT_VARIABLE HOSTNAME OUTPUT_STRIP_TRAILING_WHITESPACE)
if("${HOSTNAME}" STREQUAL "doorbell")
        message(STATUS "HOSTNAME IS DOORBELL!")
        target_include_directories(projectM-Test-UI PRIVATE BEFORE "/home/pi/random/sdl_install/SDL-release-2.28.4/")
        target_link_libraries(projectM-Test-UI
            PRIVATE
            /home/pi/random/sdl_install/SDL-release-2.28.4/build/.libs/libSDL2.so
            ${CMAKE_DL_LIBS}
        )

else()
        message(STATUS "HOSTNAME IS NOTTTTT DOORBELL!")
        target_link_libraries(projectM-Test-UI
                PRIVATE
                libprojectM::playlist
                GLM::GLM
                SDL2::SDL2
                SDL2::SDL2main
                ${CMAKE_DL_LIBS}
        )
endif()

