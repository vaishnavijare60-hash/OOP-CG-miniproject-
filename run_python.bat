@echo off
echo ======================================================================
echo    Launching Banking System (OOP & Computer Graphics Mini-Project)
echo ======================================================================
echo Select Execution Mode:
echo   1. Modern Graphical UI (Tkinter + 2D Graphics Canvas)
echo   2. Interactive Terminal Console (CLI)
echo.
set /p mode="Enter choice (1 or 2, default is 1): "

if "%mode%"=="2" (
    python banking_system.py --cli
) else (
    python banking_system.py
)
pause
