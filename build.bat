@echo off
if not defined DevEnvDir (
	call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

IF NOT EXIST build mkdir build
IF NOT EXIST bin mkdir bin
pushd build
cl -nologo -Oi -GR- -EHa- -Zi -FC -diagnostics:column /std:c++20 ..\src\main.cpp /DLL /link /out:..\bin\test.dll /INCREMENTAL:NO /NOEXP /NOIMPLIB /DLL
popd