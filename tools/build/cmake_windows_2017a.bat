cmake ^
-DCMAKE_BUILD_TYPE:STRING=Release ^
-DCMAKE_INSTALL_PREFIX:PATH=D:/local/DIPlib_2017a ^
-DDIP_ENABLE_FFTW:BOOL=ON ^
-DFFTW3_INCLUDE_DIR:PATH=D:/local/fftw ^
-DFFTW3_LIBRARY_FFTW3:FILEPATH=D:/local/fftw/libfftw3-3.lib ^
-DFFTW3_LIBRARY_FFTW3F:FILEPATH=D:/local/fftw/libfftw3f-3.lib ^
-DFREEGLUT_INCLUDE_DIR:PATH=D:/local/freeglut/include ^
-DFREEGLUT_LIBRARY:FILEPATH=D:/local/freeglut/lib/x64/freeglut.lib ^
-DGLEW_INCLUDE_DIR:PATH=D:/local/glew/include ^
-DGLEW_SHARED_LIBRARY_RELEASE:FILEPATH=D:/local/glew/lib/Release/x64/glew32s.lib ^
-DJava_JAR_EXECUTABLE:FILEPATH=D:/local/jdk-12.0.2/bin/jar.exe ^
-DJava_JAVAC_EXECUTABLE:FILEPATH=D:/local/jdk-12.0.2/bin/javac.exe ^
-DJava_JAVADOC_EXECUTABLE:FILEPATH=D:/local/jdk-12.0.2/bin/javadoc.exe ^
-DJava_JAVA_EXECUTABLE:FILEPATH=D:/local/jdk-12.0.2/bin/java.exe ^
-DBIOFORMATS_JAR:FILEPATH=D:/local/bioformats_package.jar ^
-DMatlab_ROOT_DIR:PATH="C:/Program Files/MATLAB/R2017a" ^
-DDIP_PYDIP_RELATIVE_LOCATION:BOOL=ON ^
-DPYDIP_INSTALL_PATH:PATH=D:/local/DIPlib_2017a/lib ^
-DPYTHON_EXECUTABLE:FILEPATH=C:/Users/sup-rligteringen/.conda/envs/python37/python.exe ^
d:/src/diplib ^
%*