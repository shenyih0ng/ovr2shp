@echo off

SET buildVersion=%1

IF [%buildVersion%]==[] (
	echo Missing build version arg
	echo Specify a build version e.g "setup.bat x64"
	exit /B
)

echo Build Version: %buildVersion%

IF "%buildVersion%"=="x86" (
	:: 32 bit build
	vcvarsall.bat x86 && .\build_dep.bat && nmake /F .\makefile.vc build
)

IF "%buildVersion%"=="x64" (
	:: 64 bit build
	vcvarsall.bat x64 && .\build_dep.bat && nmake /F .\makefile.vc build
)
