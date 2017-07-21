#undef DIP__ENABLE_DOCTEST

#include <diplib/linear.h>

#include <diplib/viewer/glut.h>
#include <diplib/viewer/glfw.h>

#include <diplib/viewer/image.h>
#include <diplib/viewer/slice.h>

int main()
{
#ifdef DIP__HAS_GLFW
  GLFWManager manager;
#else
  GLUTManager manager;
#endif
  
  dip::Image image { dip::UnsignedArray{ 500, 400, 50 }, 2, dip::DT_DFLOAT };  
  image[0] = 0;
  image[1] = 0;
  double *p = ((double*)image.Pointer(dip::UnsignedArray{ 250, 200, 25 }));
  
  p[0] = 1.0;
  p[1] = -0.5;
  
  image = Gauss(image, {10}) * 10000;
  
  manager.createWindow(WindowPtr(new SliceViewer(image)));
  
  dip::Image image2 { dip::UnsignedArray{ 50, 40 }, 3, dip::DT_UINT8 };  
  image2 = 128;
  
  manager.createWindow(WindowPtr(new ImageViewer(image2)));
  
  while (manager.activeWindows())
  {
    // Only necessary for GLFW
    manager.processEvents();
    usleep(10);
  }

  return 0;  
}
