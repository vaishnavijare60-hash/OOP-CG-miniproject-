@echo off
cd /d "%~dp0"
echo ======================================================================
echo    Launching Banking System (OOP and Computer Graphics Mini-Project)
echo ======================================================================
echo Select Execution Mode:
echo   1. Web Browser UI (Opens in Chrome/Edge at http://localhost:5000)
echo   2. Desktop Graphical UI (Tkinter + 2D Graphics Canvas)
echo   3. Interactive Terminal Console (CLI)
echo.
set /p mode="Enter choice (1, 2, or 3, default is 1): "

if "%mode%"=="3" (
    python banking_system.py --cli
) else if "%mode%"=="2" (
    python banking_system.py
) else (
    call run_web.bat
)
pause
