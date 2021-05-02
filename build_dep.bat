@echo off

set GDAL_INCLUDE=E:\GDAL\include

cd /d .\hfa

for %%f in (*.cpp) do (
    echo %%~nf
    cl.exe /c -I%GDAL_INCLUDE% %%f
)

cd /d ..\