#pragma once

// Include gl.h properly on supported platforms
#ifdef __APPLE__
  #include <OpenGL/gl.h>
#else
  #ifdef _WIN32
    #define NOMINMAX // windows.h must not define min() and max(), which are conflicting with std::min() and std::max()
    #include <windows.h>
    #include <gl/glew.h>  // Needed for OpenGL extensions
  #endif
  #include <GL/gl.h>
<<<<<<< HEAD
#endif
=======
#endif
>>>>>>> Fixed OpenGL compilation on Windows
