@echo off
cd /d "%~dp0"
echo Starting Apex Banking System Graphical User Interface...
python banking_system.py
if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERROR] Application exited with code %ERRORLEVEL%
    pause
)
