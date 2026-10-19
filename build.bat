@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

echo ======================================================================
echo    Compiling Banking System (OOP and CG Mini-Project)
echo ======================================================================

:: Check if g++ is in PATH
where g++ >nul 2>nul
if %ERRORLEVEL% equ 0 (
    set CXX=g++
) else (
    if exist "C:\MinGW\bin\g++.exe" (
        set "CXX=C:\MinGW\bin\g++.exe"
        set "PATH=C:\MinGW\bin;%PATH%"
    ) else (
        echo [ERROR] g++ compiler not found in PATH or C:\MinGW\bin.
        echo Please ensure MinGW GCC is installed.
        pause
        exit /b 1
    )
)

echo Using compiler: !CXX!
if not exist data mkdir data

!CXX! -std=c++14 -O2 -Iinclude src\Date.cpp src\Transaction.cpp src\Account.cpp src\SavingsAccount.cpp src\CurrentAccount.cpp src\Bank.cpp src\Visualizer.cpp src\ConsoleUI.cpp src\main.cpp -o banking_system.exe -lgdi32 -luser32

if %ERRORLEVEL% equ 0 (
    echo.
    echo [BUILD SUCCESSFUL] Created banking_system.exe
    echo Run using: run.bat
) else (
    echo.
    echo [BUILD FAILED] An error occurred during compilation.
)

pause
