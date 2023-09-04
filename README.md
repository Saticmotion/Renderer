**This is a simple software wireframe renderer.**

It's made for Win32, so that's all it runs on for now.

1. Before building, run misc\shell.bat. This runs the vcvarsall command (in x64 mode, so you might want to change it for 32 bit OS) to setup the command line for building with cl.exe
2. These settings are lost when you close the commandline, so be sure to keep it open while editing.
3. To build the project run build.bat.
4. To run the built exe, run run.bat.

---
The renderer is in [main.cpp](https://github.com/Saticmotion/Renderer/blob/master/main.cpp)

A small math library in in [math_r.cpp](https://github.com/Saticmotion/Renderer/blob/master/math_r.cpp)
