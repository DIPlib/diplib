mkdir build.pydip
cd build.pydip
mkdir wheelhouse

cmake .. -DBIOFORMATS_JAR=D:/Workspace/diplib/extra64/bioformats_package.jar -DFREEGLUT_INCLUDE_DIR=D:/Workspace/diplib/extra64 -DFREEGLUT_LIBRARY=D:/Workspace/diplib/extra64/freeglut_static.lib -DFREEGLUT_STATIC=On -DDIP_BUILD_DIPIMAGE=Off -DDIP_PYDIP_WHEEL_INCLUDE_LIBS=On

cmake .. -DPYTHON_EXECUTABLE=%USERPROFILE%/AppData/Local/Programs/Python/Python37/python.exe
cmake --build . --target bdist_wheel --config Release
copy pydip\Release\staging\dist\*.whl wheelhouse

cmake .. -DPYTHON_EXECUTABLE=%USERPROFILE%/AppData/Local/Programs/Python/Python38/python.exe
cmake --build . --target bdist_wheel --config Release
copy pydip\Release\staging\dist\*.whl wheelhouse

cd wheelhouse
%USERPROFILE%/AppData\Local\Programs\Python\Python37\python.exe -m twine upload *.whl
