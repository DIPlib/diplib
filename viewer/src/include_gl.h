#pragma once

// Include gl.h properly on supported platforms
#ifdef __APPLE__
  #include <OpenGL/gl.h> // IWYU pragma: export
#else
  #ifdef _WIN32
    #define NOMINMAX // windows.h must not define min() and max(), which are conflicting with std::min() and std::max()
    #include <windows.h> // IWYU pragma: export
  #endif
  #include <GL/gl.h> // IWYU pragma: export
#endif
