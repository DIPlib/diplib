#!/bin/bash
# Run this in a manylinux2014 docker container with /io mounted to some local directory

# Setup
yum -y install wget freeglut-devel java-1.8.0-openjdk-devel.x86_64
/opt/python/cp39-cp39/bin/python -m pip install cmake auditwheel
CMAKE=/opt/python/cp39-cp39/lib/python3.9/site-packages/cmake/data/bin/cmake
BUILD_THREADS=4
PYTHON_VERSIONS=(3.8 3.9 3.10 3.11 3.12)
EXCLUDES=(libjvm.so libOpenGL.so.0 libGLX.so.0 libGLdispatch.so.0)

# Clone diplib repository
git clone https://github.com/diplib/diplib
cd diplib
mkdir build
cd build
wget -nv https://downloads.openmicroscopy.org/bio-formats/7.0.0/artifacts/bioformats_package.jar

# Basic configuration
$CMAKE .. -DDIP_PYDIP_WHEEL_INCLUDE_LIBS=On -DBIOFORMATS_JAR=`pwd`/bioformats_package.jar 

# Build wheels
for v in ${PYTHON_VERSIONS[@]}; do
   $CMAKE .. -DPYBIND11_PYTHON_VERSION=$v -DPYTHON_EXECUTABLE=/opt/python/cp${v/./}-cp${v/./}/bin/python
   make -j $BUILD_THREADS bdist_wheel
   auditwheel repair ${EXCLUDES[@]/#/--exclude } pydip/staging/dist/*.whl
done;

mv wheelhouse /io
