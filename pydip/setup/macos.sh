# Run this on a MacOS with Xcode and Homebrew

# Setup
brew install cmake git wget libomp glfw python3 python@3.8
python3 -m pip install twine delocate
export BUILD_THREADS=4

# Clone diplib repository
git clone https://github.com/diplib/diplib
cd diplib
git checkout pydip-experimental
mkdir build
cd build
#wget https://downloads.openmicroscopy.org/bio-formats/6.5.0/artifacts/bioformats_package.jar

# Basic configuration
#cmake .. -DDIP_PYDIP_WHEEL_INCLUDE_LIBS=On -DBIOFORMATS_JAR=`pwd`/bioformats_package.jar -DDIP_BUILD_DIPIMAGE=Off
# -- we're temporarily excluding DIPjavaio because delocate wants to put the JVM into the wheel...
cmake .. -DDIP_PYDIP_WHEEL_INCLUDE_LIBS=On -DDIP_BUILD_JAVAIO=Off -DDIP_BUILD_DIPIMAGE=Off

# Python 3.7
export PYTHON=/usr/local/opt/python@3/bin/python3
export PYTHON_VERSION=3.7
cmake .. -DPYBIND11_PYTHON_VERSION=$PYTHON_VERSION -DPYTHON_EXECUTABLE=$PYTHON
make -j $BUILD_THREADS bdist_wheel
delocate-wheel -w pydip-fixed/ -v pydip/staging/dist/*.whl

# Python 3.8
export PYTHON=/usr/local/opt/python@3.8/bin/python3
export PYTHON_VERSION=3.8
cmake .. -DPYBIND11_PYTHON_VERSION=$PYTHON_VERSION -DPYTHON_EXECUTABLE=$PYTHON
make -j $BUILD_THREADS bdist_wheel
delocate-wheel -w pydip-fixed/ -v pydip/staging/dist/*.whl

# Upload to pypi.org
python3 -m twine upload pydip-fixed/*.whl
