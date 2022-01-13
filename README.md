

# Ray Caster

A reimpementation of the renderer of Wolfenstein3D.

## Requirements
This project requires an installation of OpenGL and GLUT. The following command can be used to install the dependences on a debian based system:
```bash
$ sudo apt install libglu1-mesa-dev freeglut3-dev mesa-common-dev
```

## Usage

1. Configure the project and generate the native build system:
```bash
$ cmake -B build
```

2. Call the build system to compile and link the project
```bash
$ cmake --build build
```
The compiled executables will be placed in build/bin.
