:<<"::WINDOWS_ONLY"
@echo off
:: Copyright 2024,2025 NXP
:: All rights reserved.
::
:: SPDX-License-Identifier: BSD-3-Clause
::
:<<"::WINDOWS_ONLY"

REM Usage: ".\third_party\nxp\nxp_matter_support\scripts\bootstrap.bat [sdk_path]"
REM Usage: sdk_path is optional if not given the default SDK path would be assumed
SET "SDK_PATH="
SET "CHIP_ROOT_PATH="
IF "%~1"=="" (
    goto default_sdk_path
) ELSE IF "%~1"=="--platform" (
    REM Case to ignore case where script is called with .\third_party\nxp\nxp_matter_support\scripts\bootstrap.bat --platform nxp
    REM Goal is to keep backward compatibility with old boostrap script version
    REM In such case set SDK_PATH to the default value
    goto default_sdk_path
) ELSE (
    SET "SDK_PATH=%~1"
    IF EXIST "%SDK_PATH%\middleware\matter" (
        SET "CHIP_ROOT_PATH=%SDK_PATH%\middleware\matter"
    ) ELSE IF EXIST "scripts\setup\zap.version" (
        SET "CHIP_ROOT_PATH=%CD%"
    ) ELSE (
        ECHO ERROR: Matter repository not found.
        ECHO Please ensure your SDK supports Matter or run this script from the Matter repository root.
        exit /B 1
    )
    goto run_sub_bootstrap
)

:default_sdk_path
IF EXIST "%CD%\middleware\matter" (
    @REM Default SDK path is assumed to be the current directory
    SET "SDK_PATH=%CD%"
    SET "CHIP_ROOT_PATH=%CD%\middleware\matter"
) ELSE IF EXIST "%CD%\scripts\setup\zap.version" (
    @REM Backward compatibility for old architecture
    @REM This is the case when the script is run from the root of the Matter repository
    SET "SDK_PATH=%CD%\sdk\repo\mcuxsdk"
    SET "CHIP_ROOT_PATH=%CD%"
) ELSE (
    ECHO ERROR: SDK_PATH not provided and no default SDK found.
    ECHO Please provide the SDK path as an argument with 'bootstrap.bat <sdk_path>'
    EXIT /B 1
)

:run_sub_bootstrap
echo SDK PATH = "%SDK_PATH%"
echo CHIP_ROOT_PATH = "%CHIP_ROOT_PATH%"

call "%~dp0\setup\sub_bootstrap.bat" %~n0

REM Export again environment variables to keep them in the parent process
IF NOT EXIST temp_env.bat (
    exit /b 1
) ELSE (
    call temp_env.bat
    del temp_env.bat
)
