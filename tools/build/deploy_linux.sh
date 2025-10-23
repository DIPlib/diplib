#!/bin/bash
# Run this on a Linux machine with docker, from the diplib directory
# set $PYPI_TOKEN to the PyPI token for the diplib project

docker run -v `pwd`:/io quay.io/pypa/manylinux2014_x86_64 /bin/bash /io/tools/build/manylinux.sh

python3 -m pip install -U twine
python3 -m twine upload -u __token__ -p $PYPI_TOKEN wheelhouse/*.whl
