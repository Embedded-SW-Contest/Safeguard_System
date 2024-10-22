# 안전지킴이 Safeguard_System
## LG Embedded 자동차 모빌리티 부문

# :oncoming_automobile: UWB 기반 골목길 교통사고 예방 임베디드 시스템 : Safeguard_System

<img src="https://img.shields.io/badge/C++-00599C?style=flat&logo=cplusplus&logoColor=white"/> <img src="https://img.shields.io/badge/C-A8B9CC?style=flat&logo=c&logoColor=white"/> <img src="https://img.shields.io/badge/CMake-064F8C?style=flat&logo=cmake&logoColor=white"/>

----

## 폴더구조
```
safeguard-system
├── CMakeLists.txt
├── build
│   ├── CMakeCache.txt
│   ├── CMakeFiles
│   │   ├── 3.25.1
│   │   │   ├── CMakeCCompiler.cmake
│   │   │   ├── CMakeCXXCompiler.cmake
│   │   │   ├── CMakeDetermineCompilerABI_C.bin
│   │   │   ├── CMakeDetermineCompilerABI_CXX.bin
│   │   │   ├── CMakeSystem.cmake
│   │   │   ├── CompilerIdC
│   │   │   │   ├── CMakeCCompilerId.c
│   │   │   │   ├── a.out
│   │   │   │   └── tmp
│   │   │   └── CompilerIdCXX
│   │   │       ├── CMakeCXXCompilerId.cpp
│   │   │       ├── a.out
│   │   │       └── tmp
│   │   ├── CMakeDirectoryInformation.cmake
│   │   ├── CMakeOutput.log
│   │   ├── Makefile.cmake
│   │   ├── Makefile2
│   │   ├── SSD1306.dir
│   │   │   ├── DependInfo.cmake
│   │   │   ├── build.make
│   │   │   ├── cmake_clean.cmake
│   │   │   ├── cmake_clean_target.cmake
│   │   │   ├── compiler_depend.internal
│   │   │   ├── compiler_depend.make
│   │   │   ├── compiler_depend.ts
│   │   │   ├── depend.make
│   │   │   ├── flags.make
│   │   │   ├── lib
│   │   │   ├── link.txt
│   │   │   └── progress.make
│   │   ├── TargetDirectories.txt
│   │   ├── cmake.check_cache
│   │   ├── pkgRedirects
│   │   ├── progress.marks
│   │   ├── safeguard.dir
│   │   │   ├── DependInfo.cmake
│   │   │   ├── build.make
│   │   │   ├── cmake_clean.cmake
│   │   │   ├── compiler_depend.internal
│   │   │   ├── compiler_depend.make
│   │   │   ├── compiler_depend.ts
│   │   │   ├── depend.make
│   │   │   ├── flags.make
│   │   │   ├── gps.cpp.o
│   │   │   ├── gps.cpp.o.d
│   │   │   ├── lidar.cpp.o
│   │   │   ├── lidar.cpp.o.d
│   │   │   ├── link.txt
│   │   │   ├── progress.make
│   │   │   ├── safeguard.cpp.o
│   │   │   └── safeguard.cpp.o.d
│   │   └── test.dir
│   │       ├── DependInfo.cmake
│   │       ├── build.make
│   │       ├── cmake_clean.cmake
│   │       ├── compiler_depend.internal
│   │       ├── compiler_depend.make
│   │       ├── compiler_depend.ts
│   │       ├── depend.make
│   │       ├── flags.make
│   │       ├── gpsTest.cpp.o
│   │       ├── gpsTest.cpp.o.d
│   │       ├── lidar.cpp.o
│   │       ├── lidar.cpp.o.d
│   │       ├── link.txt
│   │       ├── progress.make
│   │       ├── test.cpp.o
│   │       └── test.cpp.o.d
│   ├── Makefile
│   ├── bin
│   │   ├── safeguard
│   ├── cmake_install.cmake
│   └── lib
│       └── libSSD1306.a
├── config.h
├── gps.cpp
├── gps.h
├── httplib.h
├── lib
├── lidar.cpp
├── lidar.h
├── output.txt
├── safeguard.cpp
└── user.h
```

## 순서도
 ![image](https://github.com/user-attachments/assets/5c9458cf-c645-4666-ad0d-46f701e99b2d)


 
