<img align="right" width="120" height="120" src="https://github.com/user-attachments/assets/44954271-aef7-4081-bbb4-4f3d120085e5">

### AeroMap
Simple desktop drone mapping application.

Originally, AeroMap was a port of OpenDroneMap. I've always been fascinated by the seemingly magic process of turning a pile of photographs into accurate 2D and 3D models. The way ODM is structured clearly lays out how that process works. Unfortunately - for me - ODM is written mostly in python and I'm a C/C++ developer. So I ported it to my "native language" as a learning tool. After I saw that it was successfully processing image datasets, I published it on GitHub in hopes others may find it useful.

Now that I'm hooked, I continue to develop AeroMap. Here's the near-tearm roadmap:

  - Thermal Datasets
  - Multispectral Datasets
  - Analysis tools built into app

### Build Toolchain

  - Visual Studio Community 2019
  - Qt 5.15.2
  - OpenCV 4.10.0
  
  Set up the environment variables:

      OPENCV_PATH is the location of the OpenCV install
      PROJ_LIB is the location of AeroMap's proj data folder (eg, AeroMap\Lib\proj-7.1.1\build_vs14_x64\data)
      QT_DIR should have been set by the Qt installer
