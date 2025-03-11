@echo off
setlocal
set app=server
set APPPATH=build\Debug\%app%.exe
set EXECUTABLE=%app%.exe

if not exist %APPPATH% (
    echo Executable not found! Please build the project first.
    exit /b 1
)

echo Running %app%...

cd build\Debug\

%EXECUTABLE% server

if %ERRORLEVEL% NEQ 0 (
    echo Program exited with error code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

@REM echo Program ran successfully!
endlocal