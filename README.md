Recon
=====

3D Reconstruction

## Build (OS X)

    brew install qt5
    brew install --with-qt5 --without-python --without-legacy --c++11 vtk

## Build (Windows Powershell)

    mkdir build
    cd build
    cmake -DQTDIR=C:\Qt\5.4\msvc2013_64 ..
    Invoke-Item ReconWorkspace.sln

## Reference

- [VisualSFM Doc](http://ccwu.me/vsfm/doc.html)
- [VTK Source Code](https://github.com/Kitware/VTK)
