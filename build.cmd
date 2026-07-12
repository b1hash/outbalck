@echo off
:: SimpleRemoter Quick Build Script
:: Usage: build.cmd [release|debug] [x64|x86|all] [server|clean|publish]

setlocal enabledelayedexpansion

set CONFIG=Release
set PLATFORM=x64
set EXTRA_ARGS=

:parse_args
if "%~1"=="" goto :run
if /i "%~1"=="release" (set CONFIG=Release& shift& goto :parse_args)
if /i "%~1"=="debug" (set CONFIG=Debug& shift& goto :parse_args)
if /i "%~1"=="x64" (set PLATFORM=x64& shift& goto :parse_args)
if /i "%~1"=="x86" (set PLATFORM=x86& shift& goto :parse_args)
if /i "%~1"=="all" (set PLATFORM=all& shift& goto :parse_args)
if /i "%~1"=="server" (set EXTRA_ARGS=!EXTRA_ARGS! -ServerOnly& shift& goto :parse_args)
if /i "%~1"=="clean" (set EXTRA_ARGS=!EXTRA_ARGS! -Clean& shift& goto :parse_args)
if /i "%~1"=="publish" (set EXTRA_ARGS=!EXTRA_ARGS! -Publish& shift& goto :parse_args)
echo Unknown argument: %~1
shift
goto :parse_args

:run
echo.
echo Building SimpleRemoter: %CONFIG% ^| %PLATFORM%
echo.

powershell -ExecutionPolicy Bypass -File "%~dp0build.ps1" -Config %CONFIG% -Platform %PLATFORM% %EXTRA_ARGS%

if %errorlevel% neq 0 (
    echo.
    echo Build FAILED!
    pause
    exit /b 1
)

echo.
pause
