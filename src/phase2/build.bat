@echo off
REM Build script for phase2 - YM2151 tone generator (Windows)

echo Building Nuked-OPM tone generator...

cd /d "%~dp0"

REM Check if zig is available
where zig >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Using zig cc as compiler...
    zig cc -o opm_generator.exe main.c opm.c -lm
    if %ERRORLEVEL% NEQ 0 (
        echo Zig compilation failed, trying with cl.exe or gcc...
        goto TryFallback
    )
    goto BuildSuccess
)

:TryFallback
REM Try cl.exe (Visual Studio)
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Using cl.exe (Visual Studio) as fallback...
    cl /Fe:opm_generator.exe main.c opm.c
    if %ERRORLEVEL% EQU 0 goto BuildSuccess
)

REM Try gcc as last resort (MinGW/MSYS2)
where gcc >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Using gcc as fallback...
    gcc -o opm_generator.exe main.c opm.c -lm
    if %ERRORLEVEL% EQU 0 goto BuildSuccess
)

echo Error: No suitable C compiler found.
echo Please install one of: zig, Visual Studio (cl.exe), or gcc
echo Recommended: zig from https://ziglang.org/download/
exit /b 1

:BuildSuccess
echo Build complete! Executable: %~dp0opm_generator.exe
