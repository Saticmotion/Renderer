@echo off

REM call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat x64"

set DEBUGVARS=/Od /Zi
set RELEASEVARS=/O2 /Oi

set files=../main.cpp ../host.cpp

set link=user32.lib gdi32.lib

set options=%DEBUGVARS%  %files% %link%

mkdir .\build
pushd .\build

cl %options%

popd