@echo off
cd /d "%~dp0"
title Apex Banking System - Web Application
echo ======================================================================
echo    Starting Banking System Web Application
echo ======================================================================
echo Opening http://localhost:5000 in your browser...
start "" http://localhost:5000
python web_app.py
if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERROR] Web app exited with error code %ERRORLEVEL%
    pause
)
