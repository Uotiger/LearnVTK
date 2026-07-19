@echo off
set CMAKE="C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if "%1"=="" (
    %CMAKE% --build out/build/debug --config Debug
) else (
    %CMAKE% --build out/build/debug --target %1 --config Debug
)
