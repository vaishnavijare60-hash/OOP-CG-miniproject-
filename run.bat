@echo off
cd /d "%~dp0"

if not exist banking_system.exe (
    echo Executable banking_system.exe not found!
    echo Running build.bat first...
    call build.bat
)

if exist banking_system.exe (
    banking_system.exe
) else (
    echo [ERROR] banking_system.exe could not be found or built.
    pause
)
