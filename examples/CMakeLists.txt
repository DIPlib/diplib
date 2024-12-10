# (c)2017-2022, Cris Luengo
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


set(CMAKE_BUILD_WITH_INSTALL_RPATH 0) # Allow running these in the build directory -- they're not installed anyway

# An example program that demonstrates different ways of doing per-pixel arithmetic
add_executable(image_arithmetic cpp/image_arithmetic.cpp)
target_link_libraries(image_arithmetic DIP)

# An example program that demonstrates doing statistics across a series of images
add_executable(image_series_statistics cpp/image_series_statistics.cpp)
target_link_libraries(image_series_statistics DIP)

# An example program that demonstrates how to register a new measurement feature in dip::MeasurementTool
add_executable(register_measurement_feature cpp/register_measurement_feature.cpp)
target_link_libraries(register_measurement_feature DIP)

# A program that times dilations with different structuring element shapes and sizes
add_executable(time_dilations cpp/time_dilations.cpp)
target_link_libraries(time_dilations DIP)

# A program that times the FFT for different image sizes
add_executable(time_fft cpp/time_fft.cpp)
target_link_libraries(time_fft DIP)

# A program that shows the difference between dip::VarianceAccumulator and dip::FastVarianceAccumulator
add_executable(variance cpp/variance.cpp)
target_link_libraries(variance DIP)

# A program to compute the fractal dimension of an input image
add_executable(fractal_dimension cpp/fractal_dimension.cpp)
target_link_libraries(fractal_dimension DIP)

# This is a simple program that replicates the dip::RadialMean function
add_executable(radial_mean cpp/radial_mean.cpp)
target_compile_definitions(radial_mean PRIVATE DIP_EXAMPLES_DIR="${CMAKE_CURRENT_LIST_DIR}")
target_link_libraries(radial_mean DIP)

# This is a program that demonstrates how to generate various shapes
add_executable(generate_shapes cpp/generate_shapes.cpp)
target_link_libraries(generate_shapes DIP)

# A program to time functions with and without multithreading, useful to fine-tune the cost value that determines
# for what size image it is worth using multithreading.
add_executable(time_openmp cpp/time_openmp.cpp)
target_link_libraries(time_openmp DIP)

# A program that tests the binary morphology functions
add_executable(test_binary cpp/test_binary.cpp)
target_compile_definitions(test_binary PRIVATE DIP_EXAMPLES_DIR="${CMAKE_CURRENT_LIST_DIR}")
target_link_libraries(test_binary DIP)

# The `examples` target builds them all
add_custom_target(examples DEPENDS
                  image_arithmetic
                  register_measurement_feature
                  time_dilations
                  variance
                  fractal_dimension
                  radial_mean
                  time_openmp
                  test_binary
                  )

## A set of examples that need DIPviewer, compile only if DIPviewer is available

if(DIP_BUILD_DIPVIEWER)

   # A test program that demonstrates some of the low-level functionality of DIPviewer
   add_executable(viewer cpp/viewer.cpp)
   target_compile_definitions(viewer PRIVATE DIP_EXAMPLES_DIR="${CMAKE_CURRENT_LIST_DIR}")
   target_link_libraries(viewer DIP DIPviewer Threads::Threads)

   # An example program that does simple image manipulation and uses dip::viewer::ShowSimple
   add_executable(display cpp/display.cpp)
   target_compile_definitions(display PRIVATE DIP_EXAMPLES_DIR="${CMAKE_CURRENT_LIST_DIR}")
   target_link_libraries(display DIP DIPviewer)

   # A simple tool that shows an image file using dip::viewer::Show
   add_executable(fileviewer cpp/fileviewer.cpp)
   target_link_libraries(fileviewer DIP DIPviewer)
   if(DIP_BUILD_JAVAIO)
      target_link_libraries(fileviewer DIPjavaio)
   endif()

   # An example program that shows how to delete small objects from a grey-value image, and use dip::viewer::ShowSimple
   add_executable(select_objects cpp/select_objects.cpp)
   target_compile_definitions(select_objects PRIVATE DIP_EXAMPLES_DIR="${CMAKE_CURRENT_LIST_DIR}")
   target_link_libraries(select_objects DIP DIPviewer)

   # An example program that shows how to quantize colors in an RGB image, and use dip::viewer::ShowSimple
   add_executable(color_quantization cpp/color_quantization.cpp)
   target_compile_definitions(color_quantization PRIVATE DIP_EXAMPLES_DIR="${CMAKE_CURRENT_LIST_DIR}")
   target_link_libraries(color_quantization DIP DIPviewer)

   # An example program that shows how to apply a convolution with a custom kernel to an image
   add_executable(filter_image cpp/filter_image.cpp)
   target_compile_definitions(filter_image PRIVATE DIP_EXAMPLES_DIR="${CMAKE_CURRENT_LIST_DIR}")
   target_link_libraries(filter_image DIP DIPviewer)

   # An example program that shows how to use various geometric transformations
   add_executable(geometric_transformation cpp/geometric_transformation.cpp)
   target_compile_definitions(geometric_transformation PRIVATE DIP_EXAMPLES_DIR="${CMAKE_CURRENT_LIST_DIR}")
   target_link_libraries(geometric_transformation DIP DIPviewer)

   # An example program that uses superpixel segmentation and graph manipulation, it displays some images
   # using dip::viewer::Show, and links those displays together
   add_executable(oversegmentation cpp/oversegmentation.cpp)
   target_compile_definitions(oversegmentation PRIVATE DIP_EXAMPLES_DIR="${CMAKE_CURRENT_LIST_DIR}")
   target_link_libraries(oversegmentation DIP DIPviewer)

   # An example program that demonstrates using the graph cut segmentation algorithm, it displays some images
   # using dip::viewer::Show, and links those displays together
   add_executable(graph_cut_segmentation cpp/graph_cut_segmentation.cpp)
   target_link_libraries(graph_cut_segmentation DIP DIPviewer)

   # An example program that shows how to use dip::MeasurementTool and work with its output dip::Measurement data
   add_executable(using_measurement_tool cpp/using_measurement_tool.cpp)
   target_compile_definitions(using_measurement_tool PRIVATE DIP_EXAMPLES_DIR="${CMAKE_CURRENT_LIST_DIR}")
   target_link_libraries(using_measurement_tool DIP DIPviewer)

   # An example program that shows how to blend images
   add_executable(blend_images cpp/blend_images.cpp)
   target_compile_definitions(blend_images PRIVATE DIP_EXAMPLES_DIR="${CMAKE_CURRENT_LIST_DIR}")
   target_link_libraries(blend_images DIP DIPviewer)

   add_dependencies(examples
                    viewer
                    display
                    fileviewer
                    select_objects
                    color_quantization
                    filter_image
                    geometric_transformation
                    oversegmentation
                    using_measurement_tool
                    )

   ## An example that needs OpenCV, compile only if OpenCV is available

   find_package(OpenCV QUIET COMPONENTS core imgproc highgui)
   if(OpenCV_FOUND)

      # An example program that uses both DIPlib and OpenCV
      add_executable(opencv_with_dip external_interfaces/opencv_with_dip.cpp)
      target_compile_definitions(opencv_with_dip PRIVATE DIP_EXAMPLES_DIR="${CMAKE_CURRENT_LIST_DIR}")
      target_link_libraries(opencv_with_dip DIP DIPviewer ${OpenCV_LIBS})

      add_dependencies(examples
                       opencv_with_dip
                       )

   endif()

   ## An example that needs Vigra, compile only if Vigra is available

   find_package(Vigra QUIET)
   if(Vigra_FOUND)

      # An example program that uses both DIPlib and Vigra
      add_executable(vigra_with_dip external_interfaces/vigra_with_dip.cpp)
      target_compile_definitions(vigra_with_dip PRIVATE DIP_EXAMPLES_DIR="${CMAKE_CURRENT_LIST_DIR}")
      target_include_directories(vigra_with_dip PRIVATE ${Vigra_INCLUDE_DIRS})
      target_link_libraries(vigra_with_dip DIP DIPviewer vigraimpex)
      if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
         target_compile_options(vigra_with_dip PRIVATE -Wno-conversion -Wno-sign-conversion)
      elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
         target_compile_options(vigra_with_dip PRIVATE -Wno-conversion -Wno-sign-conversion -Wno-deprecated-copy -Wno-parentheses)
      endif()

      add_dependencies(examples
                       vigra_with_dip
                       )

   endif()

endif(DIP_BUILD_DIPVIEWER)


### ---

# This is more a utility for the developers than an example...
# This program generates data in the file src/generation/draw_text_builtin.cpp
# We don't build this with the other examples
add_executable(generate_embedded_font cpp/generate_embedded_font.cpp)
target_link_libraries(generate_embedded_font DIP)
