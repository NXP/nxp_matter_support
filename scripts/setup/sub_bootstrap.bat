:<<"::WINDOWS_ONLY"
@echo off
:: Copyright 2024,2025 NXP
:: All rights reserved.
::
:: SPDX-License-Identifier: BSD-3-Clause
::
:<<"::WINDOWS_ONLY"

setlocal EnableDelayedExpansion

if "%~1" == "activate" (
    goto python_env
)

:: Ensure developer mode is enabled
:check_developer_mode
echo Checking if Developer Mode is enabled...
reg query HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\AppModelUnlock /v AllowDevelopmentWithoutDevLicense | find "0x1" >NUL 2>&1
if %ERRORLEVEL% EQU 0 goto python_env
set /p ENABLE_DEVELOPER_MODE=This step will enable Windows Developer Mode. Are you sure? (y/n)
if /i "%ENABLE_DEVELOPER_MODE%" NEQ "y" (
    echo Developer Mode is a prerequisite for pigweed virtual environment. Cancelling bootstrap.
    exit /b 1
)

:python_env
echo Python installation ...
:: Detect python installation.
:: In order to avoid opening Microsoft Store app, bypass the entries that
:: contain WindowsApps string in their path. This ensures that a proper
:: Python3 executable is used. If no Python is detected, log a message.
for /f %%p in ('where python') do (
    echo.%%p | findstr WindowsApps >NUL 2>&1
    if !ERRORLEVEL! NEQ 0 (
        %%p --version >python_version.tmp 2>&1
        findstr /C:"Python 3" python_version.tmp >NUL 2>&1
        if !ERRORLEVEL! EQU 0 (
            endlocal
            echo Python is found at: %%p
            set "python=%%p"
            goto env_setup
        )
    )
)

echo.
echo Error: no system Python3 present
echo.
echo   Please install Python3 on your system, add it to your PATH
echo   and re-try running bootstrap.
exit /b 1

goto env_setup

:: Env setup
:env_setup

REM Check if the script is being run from the <matter_repo>
IF NOT EXIST "scripts\setup\zap.version" (
    echo ERROR: This script must be run from the <matter_repo> directory.
    exit /b 1
)

REM Check if pigweed_environment.gni exists
IF NOT EXIST "build_overrides\pigweed_environment.gni" (
    echo ERROR: pigweed_environment.gni not found in build_overrides.
    exit /b 1
)

REM Check if gn is available in the PATH
where gn >nul 2>nul
IF ERRORLEVEL 1 (
    echo ERROR: gn is not available in the PATH.
    exit /b 1
)

REM Check if cmake is available in the PATH
where cmake >nul 2>nul
IF ERRORLEVEL 1 (
    echo ERROR: cmake is not available in the PATH.
    exit /b 1
)

set "VENV_PATH=%CD%\venv"

if "%~1" == "activate" (
    goto activate
) else (
    goto boostrap
)

:: Activate
:activate
REM Check if already in a virtual env
IF DEFINED VIRTUAL_ENV (
    echo ERROR: already in a virtual env
    exit /b 1 
)
REM Activate the virtual environnemnt
IF EXIST "venv" (
    call "venv\Scripts\activate"
) else (
    echo ERROR: venv not found would need to run bootstrap script
    exit /b 1 
)
REM Check now that the virtual env is active
IF DEFINED VIRTUAL_ENV (
    echo Virtual environment is active.
    echo VIRTUAL_ENV = %VIRTUAL_ENV%
) else (
    echo Virtual environment is NOT active.
)
goto end

:: boostrap
:boostrap

REM Check if requirements.txt exists in the SDK path
IF NOT EXIST "%SDK_PATH%\scripts\requirements.txt" (
    echo ERROR: requirements.txt not found in %SDK_PATH%\scripts.
    exit /b 1
)

REM Check if py_matter_idl file exist
IF NOT EXIST "scripts\py_matter_idl" (
    echo ERROR: y_matter_idl not found in <matter_repo>\scripts.
    exit /b 1
)


REM Run mcux-env.cmd
IF NOT EXIST "%SDK_PATH%\mcux-env.cmd" (
    echo ERROR: mcux-env.cmd not found in %SDK_PATH%.
    exit /b 1
)

REM Check if requirements-matter.txt exists in the SDK path
IF EXIST "%SDK_PATH%\scripts\requirements-matter.txt" (
    SET "REQUIREMENTS_FILE=%SDK_PATH%\scripts\requirements-matter.txt"
) ELSE (
    SET "REQUIREMENTS_FILE="
)

REM Create Python virtual environment if it doesn't exist
IF NOT EXIST "venv" (
    echo Creating Python virtual environment...
    call "%python%" -m venv venv
)

REM Activate the virtual environment if not already active
IF NOT DEFINED VIRTUAL_ENV (
    call "venv\Scripts\activate"
)

REM Check now that the virtual env is active
IF NOT DEFINED VIRTUAL_ENV (
    echo ERROR: venv was not correclty activated.
    exit /b 1
)

REM Install requirements from SDK requirements.txt
pip install -r "%SDK_PATH%\scripts\requirements.txt"
REM Install Matter requirement from py_matter_idl
pip install -e "scripts\py_matter_idl"

REM Install additional packages if requirements-matter.txt is missing
IF "%REQUIREMENTS_FILE%"=="" (
    echo Installing additional packages...
    pip install crc>=7.0.0 jsonschema>=4.17.0
) 

:end

REM Read zap version from file
FOR /F "usebackq delims=" %%F IN ("scripts\setup\zap.version") DO SET zap_version=%%F

REM Build the full path
SET "ZAP_INSTALL_PATH=%CD%\.zap\zap-%zap_version%"

REM Use pushd to resolve to absolute path
IF EXIST "%ZAP_INSTALL_PATH%\" (
	echo ZAP already exist...
    pushd "%ZAP_INSTALL_PATH%"
    SET "ZAP_INSTALL_PATH=%CD%\.zap\zap-%zap_version%"
    popd
) ELSE (
    REM Download ZAP tool if not found
    echo ZAP not found, downloading...
    call python scripts\tools\zap\zap_download.py --sdk-root . --extract-root .zap
    pushd "%CD%\.zap\zap-%zap_version%"
    SET "ZAP_INSTALL_PATH=%CD%\.zap\zap-%zap_version%"
    popd
)

echo ZAP_INSTALL_PATH is %ZAP_INSTALL_PATH%

setlocal EnableDelayedExpansion

call "%SDK_PATH%\mcux-env.cmd"
echo set "ZEPHYR_BASE=%ZEPHYR_BASE%" > temp_env.bat

REM Add virtual env path and Zap path to PATH only if not already added
set "foundEnvPath=false"
set "foundEnvZapPath=false"

FOR %%A IN ("%PATH:;=" "%") DO (
    IF /I "%%~A"=="%VENV_PATH%\Scripts" (
        set "foundEnvPath=true"
    ) ELSE IF /I "%%~A"=="%ZAP_INSTALL_PATH%" (
        set "foundEnvZapPath=true"
    )
)

IF "!foundEnvPath!"=="false" (
    echo set "PATH=%VENV_PATH%\Scripts;%%PATH%%" >> temp_env.bat
)

IF "!foundEnvZapPath!"=="false" (
    echo set "PATH=%ZAP_INSTALL_PATH%;%%PATH%%" >> temp_env.bat
)

west mcuxsdk-export