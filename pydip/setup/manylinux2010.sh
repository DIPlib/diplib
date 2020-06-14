# sudo docker run -i -t quay.io/pypa/manylinux2010_x86_64 /bin/bash

# Setup
yum -y install freeglut-devel
/opt/python/cp37-cp37m/bin/python -m pip install cmake twine
export CMAKE=/opt/python/cp37-cp37m/lib/python3.7/site-packages/cmake/data/bin/cmake

# Clone diplib repository
git clone https://github.com/diplib/diplib
cd diplib
mkdir build
cd build

# Python 3.7
export PYTHON=cp37-cp37m
export PYTHON_VERSION=3.7
$CMAKE .. -DDIP_PYDIP_WHEEL_INCLUDE_LIBS=On -DPYBIND11_PYTHON_VERSION=$PYTHON_VERSION -DPYTHON_EXECUTABLE=/opt/python/$PYTHON/bin/python
make bdist_wheel
/opt/python/cp37-cp37m/bin/python auditwheel repair pydip/staging/dist/*.whl

# Python 3.8
export PYTHON=cp38-cp38
export PYTHON_VERSION=3.8
$CMAKE .. -DDIP_PYDIP_WHEEL_INCLUDE_LIBS=On -DPYBIND11_PYTHON_VERSION=$PYTHON_VERSION -DPYTHON_EXECUTABLE=/opt/python/$PYTHON/bin/python
make bdist_wheel
/opt/python/cp37-cp37m/bin/python auditwheel repair pydip/staging/dist/*.whl

# Upload to test.pypi.org
/opt/python/cp37-cp37m/bin/python -m twine upload --repository testpypi wheelhouse/*.whl
