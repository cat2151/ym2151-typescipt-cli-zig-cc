@echo off
REM Build script for phase2 - YM2151 tone generator (Windows)

echo Building Nuked-OPM tone generator...

cd /d "%~dp0"

REM Check if zig is available
where zig >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Using zig cc as compiler...
    zig cc -o opm_generator.exe main.c opm.c -lm
) else (
    echo Error: zig not found. Please install zig to build this project.
    echo Download from: https://ziglang.org/download/
    exit /b 1
)

echo Build complete! Executable: %~dp0opm_generator.exe
