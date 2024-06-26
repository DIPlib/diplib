#!/bin/bash
# Run this in a manylinux2014 docker container with /io mounted to some local directory

# Setup
yum -y install freeglut-devel java-1.8.0-openjdk-devel.x86_64
/opt/python/cp39-cp39/bin/python -m pip install cmake auditwheel
CMAKE=/opt/python/cp39-cp39/lib/python3.9/site-packages/cmake/data/bin/cmake
BUILD_THREADS=4
PYTHON_VERSIONS=(3.9 3.10 3.11 3.12)
EXCLUDES=(libjvm.so libOpenGL.so.0 libGLX.so.0 libGLdispatch.so.0)

# /io is diplib repo
cd /io
mkdir build
cd build

# Basic configuration
$CMAKE .. -DDIP_PYDIP_WHEEL_INCLUDE_LIBS=On

# Build wheels
for v in ${PYTHON_VERSIONS[@]}; do
   $CMAKE .. -DPython_EXECUTABLE=/opt/python/cp${v/./}-cp${v/./}/bin/python
   make -j $BUILD_THREADS bdist_wheel
   auditwheel repair ${EXCLUDES[@]/#/--exclude } pydip/staging/dist/*.whl
done;

mv wheelhouse /io
