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
IF "%~1"=="" (
    goto default_sdk_path
) ELSE IF "%~1"=="--platform" (
    REM Case to ignore case where script is called with .\third_party\nxp\nxp_matter_support\scripts\bootstrap.bat --platform nxp
    REM Goal is to keep backward compatibility with old boostrap scrip version
    REM In such case set SDK_PATH to the default value
    goto default_sdk_path
) ELSE (
    SET "SDK_PATH=%~1"
    goto run_sub_bootstrap
)

:default_sdk_path
SET "SDK_PATH=third_party\nxp\nxp_matter_support\github_sdk\sdk_next\repo\mcuxsdk"

:run_sub_bootstrap
echo SDK PATH = "%SDK_PATH%

call third_party\nxp\nxp_matter_support\scripts\setup\sub_bootstrap.bat %~n0

REM Export again environment variables to keep them in the parent process
IF NOT EXIST temp_env.bat (
    exit /b 1
) ELSE (
    call temp_env.bat
    del temp_env.bat
)
