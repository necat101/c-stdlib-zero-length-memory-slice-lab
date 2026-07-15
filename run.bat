@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

REM Resolve Zig
if defined ZIG_BIN if exist "%ZIG_BIN%" goto :have_zig
where zig >nul 2>&1
if %ERRORLEVEL%==0 (
  for /f "delims=" %%i in ('where zig') do set "ZIG_BIN=%%i" & goto :have_zig
)
if exist "%USERPROFILE%\.local\bin\zig.exe" set "ZIG_BIN=%USERPROFILE%\.local\bin\zig.exe" & goto :have_zig
if exist "%USERPROFILE%\bin\zig.exe" set "ZIG_BIN=%USERPROFILE%\bin\zig.exe" & goto :have_zig
:have_zig

REM Resolve Python
if defined PYTHON_BIN if exist "%PYTHON_BIN%" goto :have_py
where python >nul 2>&1
if %ERRORLEVEL%==0 set "PYTHON_BIN=python" & goto :have_py
where python3 >nul 2>&1
if %ERRORLEVEL%==0 set "PYTHON_BIN=python3" & goto :have_py
set "PYTHON_BIN=python"
:have_py

if not defined ZIG_BIN echo warning: zig not found - c-dependent rows will be toolchain_skip

echo ==^> run_lab.py
"%PYTHON_BIN%" run_lab.py
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
echo.
echo ==^> test_lab.py
"%PYTHON_BIN%" -m unittest -v
