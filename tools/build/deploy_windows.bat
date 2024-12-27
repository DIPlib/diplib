REM Run this on Windows 10 with VS C++ build tools installed

REM Get Python versions

FOR /F "tokens=*" %%g IN ('dir C:\hostedtoolcache\windows\Python\3.10.* /B') do (SET PYTHON310=%%g)
FOR /F "tokens=*" %%g IN ('dir C:\hostedtoolcache\windows\Python\3.11.* /B') do (SET PYTHON311=%%g)
FOR /F "tokens=*" %%g IN ('dir C:\hostedtoolcache\windows\Python\3.12.* /B') do (SET PYTHON312=%%g)
FOR /F "tokens=*" %%g IN ('dir C:\hostedtoolcache\windows\Python\3.13.* /B') do (SET PYTHON313=%%g)

REM Setup
mkdir build
cd build
mkdir wheelhouse

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
cmake .. -A x64 -DFREEGLUT_INCLUDE_DIR=%CD%\freeglut-3.0.0\include -DFREEGLUT_LIBRARY=%CD%\freeglut-3.0.0\build\lib\Release\freeglut_static.lib -DFREEGLUT_STATIC=On -DDIP_BUILD_DIPIMAGE=Off -DDIP_PYDIP_WHEEL_INCLUDE_LIBS=On -DDIP_ENABLE_UNICODE=Off

REM Python 3.10
C:\hostedtoolcache\windows\Python\%PYTHON310%\x64\python.exe -m pip install setuptools wheel build
cmake .. -A x64 -DPython_EXECUTABLE=C:\hostedtoolcache\windows\Python\%PYTHON310%\x64\python.exe
cmake --build . --target bdist_wheel --config Release
copy pydip\Release\staging\dist\*.whl wheelhouse

REM Python 3.11
C:\hostedtoolcache\windows\Python\%PYTHON311%\x64\python.exe -m pip install setuptools wheel build
cmake .. -A x64 -DPython_EXECUTABLE=C:\hostedtoolcache\windows\Python\%PYTHON311%\x64\python.exe
cmake --build . --target bdist_wheel --config Release
copy pydip\Release\staging\dist\*.whl wheelhouse

REM Python 3.12
C:\hostedtoolcache\windows\Python\%PYTHON312%\x64\python.exe -m pip install setuptools wheel build
cmake .. -A x64 -DPython_EXECUTABLE=C:\hostedtoolcache\windows\Python\%PYTHON312%\x64\python.exe
cmake --build . --target bdist_wheel --config Release
copy pydip\Release\staging\dist\*.whl wheelhouse

REM Python 3.13
C:\hostedtoolcache\windows\Python\%PYTHON313%\x64\python.exe -m pip install setuptools wheel build
cmake .. -A x64 -DPython_EXECUTABLE=C:\hostedtoolcache\windows\Python\%PYTHON313%\x64\python.exe
cmake --build . --target bdist_wheel --config Release
copy pydip\Release\staging\dist\*.whl wheelhouse

REM Upload to pypi.org
cd wheelhouse
@python -m twine upload *.whl -u __token__ -p %PYPI_TOKEN%
