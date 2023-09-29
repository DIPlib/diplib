# Compiling *DIP*image on Linux

Few things to keep in mind:

* No documentation
* No Python interface
* Build with BioFormats
* Build with Matlab R2018b
* Matlab R2021b install has problems with root rights in display, use `xhost` line
* Matlab R2018b requires g++-8
* FFTW has no recent apt-package and must be build
* compile FFTW with THREADS for DOUBLE *and* FLOAT

## Matlab install

```bash
cd Downloads
mkdir matlab_R2018b
mv matlab_R2018b_glnxa64.zip matlab_R2018b
cd matlab_R2018b/
unzip matlab_R2018b_glnxa64.zip
xhost +SI:localuser:root
sudo ./install
matlab
```

## *DIP*image compilation

```bash
sudo apt install build-essential cmake git
suda apt install gcc-8 g++-8
sudo apt install cmake-curses-gui
sudo apt install openjdk-8-jdk
sudo apt install freeglut3-dev
sudo apt install libfreetype-dev
export CC=gcc-8
export CXX=g++-8
mkdir ~/diplib
cd ~/diplib
wget https://downloads.openmicroscopy.org/bio-formats/7.0.0/artifacts/bioformats_package.jar
mkdir download source repository
cd download
wget http://www.fftw.org/fftw-3.3.10.tar.gz
cd ../source
tar xf ../download/fftw-3.3.10.tar.gz 
cd fftw-3.3.10/
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=~/diplib/fftw \
    -DENABLE_AVX=On \
    -DENABLE_AVX2=On \
    -DENABLE_SSE=On \
    -DENABLE_SSE2=On \
    -DENABLE_THREADS=On
make -j
make install
cmake .. -DCMAKE_INSTALL_PREFIX=~/diplib/fftw \
    -DENABLE_AVX=On \
    -DENABLE_AVX2=On \
    -DENABLE_SSE=On \
    -DENABLE_SSE2=On \
    -DENABLE_THREADS=On \
    -DENABLE_FLOAT=On
make -j
make install
export LD_LIBRARY_PATH=~/diplib/fftw/lib:$LD_LIBRARY_PATH
cd ~/diplib/source
git clone https://github.com/DIPlib/diplib.git
cd diplib
cmake .. -DCMAKE_INSTALL_PREFIX=~/diplib/diplib \
    -DDIP_BUILD_PYDIP=Off \
    -DDIP_ENABLE_FFTW=On \
    -DDIP_ENABLE_FREETYPE=On \
    -DFFTW3_LIBRARY_FFTW3=~/diplib/fftw/lib/libfftw3.so \
    -DFFTW3_LIBRARY_FFTW3F=~/diplib/fftw/lib/libfftw3f.so \
    -DFFTW3_LIBRARY_FFTW3_THREADS=~/diplib/fftw/lib/libfftw3_threads.so \
    -DFFTW3_LIBRARY_FFTW3F_THREADS=~/diplib/fftw/lib/libfftw3f_threads.so \
    -DFFTW3_INCLUDE_DIR=~/diplib/fftw/include \
    -DBIOFORMATS_JAR=~/diplib/bioformats_package.jar
make -j
make -j check
make install
cp ~/diplib/bioformats_package.jar ~/diplib/diplib/share/DIPimage/private
matlab
```
