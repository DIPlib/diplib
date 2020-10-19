# Run this on MacOS with Xcode from the diplib diretory
# set $PYPI_TOKEN to the PyPI token for the diplib project

# Setup
export BUILD_THREADS=2
export DELOCATE=`pwd`/tools/travis/delocate

mkdir build
cd build
#wget https://downloads.openmicroscopy.org/bio-formats/6.5.0/artifacts/bioformats_package.jar
touch bioformats_package.jar

python3 -m pip install delocate wheel twine==1.15.0

# Basic configuration
cmake .. -DDIP_PYDIP_WHEEL_INCLUDE_LIBS=On -DBIOFORMATS_JAR=`pwd`/bioformats_package.jar -DDIP_BUILD_DIPIMAGE=Off

# Python 3.7
export PYTHON=/usr/local/opt/python@3.7/bin/python3
export PYTHON_VERSION=3.7
cmake .. -DPYBIND11_PYTHON_VERSION=$PYTHON_VERSION -DPYTHON_EXECUTABLE=$PYTHON
make -j $BUILD_THREADS bdist_wheel
python3 $DELOCATE -w wheelhouse/ -v pydip/staging/dist/*.whl

# Python 3.8
export PYTHON=/usr/local/opt/python@3.8/bin/python3
export PYTHON_VERSION=3.8
cmake .. -DPYBIND11_PYTHON_VERSION=$PYTHON_VERSION -DPYTHON_EXECUTABLE=$PYTHON
make -j $BUILD_THREADS bdist_wheel
python3 $DELOCATE -w wheelhouse/ -v pydip/staging/dist/*.whl

# Upload to pypi.org
python3 -m twine upload -u __token__ -p $PYPI_TOKEN wheelhouse/*.whl
