#ifndef setup_hpp
#define setup_hpp

#include "pmSDL.hpp"

#include <string>

class projectMSDL;

void debugGL(GLenum source,
               GLenum type,
               GLuint id,
               GLenum severity,
               GLsizei length,
               const GLchar* message,
             const void* userParam);

void seedRand();
void initGL();
void dumpOpenGLInfo();
void enableGLDebugOutput();
projectMSDL *setupSDLApp();

#endif /* setup_hpp */
