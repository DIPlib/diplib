# Run this on MacOS with Xcode from the diplib directory
# set $PYPI_TOKEN to the PyPI token for the diplib project

# Setup
export PYTHON_VERSIONS=(3.8 3.9 3.10 3.11 3.12)
brew install python@3.8
brew install python@3.9
brew install python@3.10
brew install python@3.11
brew install python@3.12
# The install above might have changed the default version of `python3`, so we need to reinstall packages:
python3 -m pip install -U twine delocate

mkdir build
cd build
wget -nv https://downloads.openmicroscopy.org/bio-formats/7.0.0/artifacts/bioformats_package.jar

# Basic configuration
cmake .. -DDIP_PYDIP_WHEEL_INCLUDE_LIBS=On -DBIOFORMATS_JAR=`pwd`/bioformats_package.jar -DDIP_BUILD_DIPIMAGE=Off

# Build all wheels
for v in ${PYTHON_VERSIONS[@]}; do
   export PYTHON=$HOMEBREW_DIR/opt/python@$v/bin/python$v
   $PYTHON -m pip install build
   cmake .. -DPYBIND11_PYTHON_VERSION=$v -DPYTHON_EXECUTABLE=$PYTHON
   make -j $BUILD_THREADS bdist_wheel
   delocate-wheel -e libjvm -w wheelhouse/ -v pydip/staging/dist/*.whl
done;

# Upload to pypi.org
python3 -m twine upload -u __token__ -p $PYPI_TOKEN wheelhouse/*.whl
