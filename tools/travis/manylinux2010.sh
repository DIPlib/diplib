#!/bin/bash
# Run this in a manylinux2010 docker container with /io mounted to some local directory

# Setup
yum -y install wget freeglut-devel java-1.8.0-openjdk-devel.x86_64
/opt/python/cp39-cp39/bin/python -m pip install cmake auditwheel
export CMAKE=/opt/python/cp39-cp39/lib/python3.9/site-packages/cmake/data/bin/cmake
export BUILD_THREADS=2
export AUDITWHEEL=`pwd`/diplib/tools/travis/auditwheel

# Clone diplib repository
git clone https://github.com/diplib/diplib
cd diplib
mkdir build
cd build
wget https://downloads.openmicroscopy.org/bio-formats/6.5.0/artifacts/bioformats_package.jar

# Basic configuration
$CMAKE .. -DDIP_PYDIP_WHEEL_INCLUDE_LIBS=On -DBIOFORMATS_JAR=`pwd`/bioformats_package.jar 

# Python 3.8
export PYTHON=cp38-cp38
export PYTHON_VERSION=3.8
$CMAKE .. -DPYBIND11_PYTHON_VERSION=$PYTHON_VERSION -DPYTHON_EXECUTABLE=/opt/python/$PYTHON/bin/python
make -j $BUILD_THREADS bdist_wheel
/opt/python/cp39-cp39/bin/python $AUDITWHEEL repair pydip/staging/dist/*.whl

# Python 3.9
export PYTHON=cp39-cp39
export PYTHON_VERSION=3.9
$CMAKE .. -DPYBIND11_PYTHON_VERSION=$PYTHON_VERSION -DPYTHON_EXECUTABLE=/opt/python/$PYTHON/bin/python
make -j $BUILD_THREADS bdist_wheel
/opt/python/cp39-cp39/bin/python $AUDITWHEEL repair pydip/staging/dist/*.whl

# Python 3.10
export PYTHON=cp310-cp310
export PYTHON_VERSION=3.10
$CMAKE .. -DPYBIND11_PYTHON_VERSION=$PYTHON_VERSION -DPYTHON_EXECUTABLE=/opt/python/$PYTHON/bin/python
make -j $BUILD_THREADS bdist_wheel
/opt/python/cp39-cp39/bin/python $AUDITWHEEL repair pydip/staging/dist/*.whl

# Python 3.11
export PYTHON=cp311-cp311
export PYTHON_VERSION=3.11
$CMAKE .. -DPYBIND11_PYTHON_VERSION=$PYTHON_VERSION -DPYTHON_EXECUTABLE=/opt/python/$PYTHON/bin/python
make -j $BUILD_THREADS bdist_wheel
/opt/python/cp39-cp39/bin/python $AUDITWHEEL repair pydip/staging/dist/*.whl

mv wheelhouse /io
