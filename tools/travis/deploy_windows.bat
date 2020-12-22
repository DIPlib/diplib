REM Run this on Windows 10 with VS C++ build tools installed

REM Setup
mkdir build
cd build
mkdir wheelhouse

python -m wget https://downloads.openmicroscopy.org/bio-formats/6.5.0/artifacts/bioformats_package.jar
python -m wget https://sourceforge.net/projects/freeglut/files/freeglut/3.0.0/freeglut-3.0.0.tar.gz/download
python -c "import tarfile; tar = tarfile.open('download'); tar.extractall()"
cd freeglut-3.0.0
mkdir build
cd build
cmake .. -A x64
cmake --build . --config Release
cd ..
cd ..

REM Basic configuration
cmake .. -A x64 -DBIOFORMATS_JAR=%CD%\bioformats_package.jar -DFREEGLUT_INCLUDE_DIR=%CD%\freeglut-3.0.0\include -DFREEGLUT_LIBRARY=%CD%\freeglut-3.0.0\build\lib\Release\freeglut_static.lib -DFREEGLUT_STATIC=On -DDIP_BUILD_DIPIMAGE=Off -DDIP_PYDIP_WHEEL_INCLUDE_LIBS=On

REM Python 3.7
cmake .. -A x64 -DPYTHON_EXECUTABLE=C:/Python37/python.exe
cmake --build . --target bdist_wheel --config Release
copy pydip\Release\staging\dist\*.whl wheelhouse

REM Python 3.8
cmake .. -A x64 -DPYTHON_EXECUTABLE=C:/Python38/python.exe
cmake --build . --target bdist_wheel --config Release
copy pydip\Release\staging\dist\*.whl wheelhouse

REM Python 3.9
cmake .. -A x64 -DPYTHON_EXECUTABLE=C:/Python39/python.exe
cmake --build . --target bdist_wheel --config Release
copy pydip\Release\staging\dist\*.whl wheelhouse

REM Upload to pypi.org
cd wheelhouse
@python -m twine upload *.whl -u __token__ -p %PYPI_TOKEN%
