@echo off

g++ main.cpp WSSD.cpp -o main.exe

if %ERRORLEVEL% neq 0 (
    echo compilation error
    exit /b 1
)

main.exe

pause