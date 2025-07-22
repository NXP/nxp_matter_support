#!/bin/bash

# Copyright 2024,2025 NXP
# SPDX-License-Identifier: BSD-3-Clause

# Check if the script is being sourced
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "❌#!/bin/bash ERROR: This script must be sourced, not executed."
    echo "✅ Please run it using: source ${BASH_SOURCE[0]}"
    exit 1
fi

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macos"
else
    echo "Unsupported platform: $OSTYPE"
    return 1
fi

# Usage: source bootstrap.sh [sdk_path]
# If sdk_path is not provided, default path is used
if [[ "$1" == "" || "$1" == "--platform" ]]; then
  if [[ -d "$(pwd)/middleware/matter" ]]; then
    SDK_PATH="$(pwd)"
    CHIP_ROOT_PATH="$SDK_PATH/middleware/matter"
  elif [[ -d "$(pwd)/third_party/nxp/nxp_matter_support/github_sdk/sdk_next/repo/mcuxsdk" ]]; then
    SDK_PATH="$(pwd)/third_party/nxp/nxp_matter_support/github_sdk/sdk_next/repo/mcuxsdk"
    CHIP_ROOT_PATH=$(pwd)
  else
    echo "❌ ERROR: SDK_PATH not provided and no default SDK found."
    echo "👉 Please provide the SDK path as an argument with 'source bootstrap.sh <sdk_path>'"
    return 1
  fi
else
  SDK_PATH="$1"
  if [[ -f "scripts/setup/zap.version" ]]; then
    CHIP_ROOT_PATH="$(pwd)"
  elif [[ -d "$SDK_PATH/middleware/matter" ]]; then
    CHIP_ROOT_PATH="$SDK_PATH/middleware/matter"
  else
    echo "❌ ERROR: Matter repository not found."
    echo "👉 Please ensure your SDK supports Matter or run this script from the Matter repository root."
    return 1
  fi
fi

echo "📁 SDK PATH = $SDK_PATH"
echo "📁 CHIP_ROOT_PATH = $CHIP_ROOT_PATH"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SCRIPT_NAME=$(basename "${BASH_SOURCE[0]}")

VENV_PATH="$(pwd)/venv"

if [[ ! -f "$CHIP_ROOT_PATH/build_overrides/pigweed_environment.gni" ]]; then
  echo "ERROR: pigweed_environment.gni not found in build_overrides."
  return 1
fi

command -v gn >/dev/null 2>&1 || { echo >&2 "ERROR: gn is not available in the PATH."; return 1; }
command -v cmake >/dev/null 2>&1 || { echo >&2 "ERROR: cmake is not available in the PATH."; return 1; }

# Check for Python 3
if ! command -v python3 >/dev/null 2>&1; then
  echo "ERROR: Python3 is not installed or not in PATH."
  return 1
fi

PYTHON=python3

# Read zap version
ZAP_VERSION=$(<$CHIP_ROOT_PATH/scripts/setup/zap.version)
ZAP_INSTALL_PATH="$(pwd)/.zap/zap-$ZAP_VERSION"

activate() { 
  if [[ -n "$VIRTUAL_ENV" ]]; then
    echo "❌ ERROR: Virtual environment is already active."
    echo "👉 Please deactivate it first with 'deactivate' command."
    return 1
  fi
  if [[ -d "$VENV_PATH" ]]; then
    source "$VENV_PATH/bin/activate" || return 1
  else
    echo "❌ ERROR: Virtual environment not found at $VENV_PATH. Please run bootstrap.sh to create it."
    return 1
  fi
  
  if [[ -n "$VIRTUAL_ENV" ]]; then
      echo "✅ Virtual environment is active."
      echo "📦 VIRTUAL_ENV = $VIRTUAL_ENV"
  else
      echo "❌ Virtual environment is NOT active."
  fi
}

bootstrap() {
  # Create virtual environment if it does not exist
  if [[ ! -d "$VENV_PATH" ]]; then
    echo "Creating Python virtual environment..."
    $PYTHON -m venv "$VENV_PATH" || return 1
  fi

  # Activate virtual environment
  source "$VENV_PATH/bin/activate" || return 1

  # Install requirements
  if [[ ! -f "$SDK_PATH/scripts/requirements.txt" ]]; then
    echo "ERROR: requirements.txt not found in $SDK_PATH/scripts."
    return 1
  fi

  if [[ ! -d "$CHIP_ROOT_PATH/scripts/py_matter_idl" ]]; then
    echo "ERROR: py_matter_idl not found in scripts."
    return 1
  fi

  pip install -r "$SDK_PATH/scripts/requirements.txt"
  pip install -e "$CHIP_ROOT_PATH/scripts/py_matter_idl"

  if [[ -f "$SDK_PATH/scripts/requirements-matter.txt" ]]; then
    pip install -r "$SDK_PATH/scripts/requirements-matter.txt"
  else
    pip install 'crc>=7.0.0' 'jsonschema>=4.17.0'
  fi

  # Download ZAP if it does not exist
  if [[ -d "$ZAP_INSTALL_PATH" ]]; then
    echo "ZAP already exists at $ZAP_INSTALL_PATH. Skipping download."
  else
    echo "ZAP not found, downloading..."
    python $CHIP_ROOT_PATH/scripts/tools/zap/zap_download.py --sdk-root $CHIP_ROOT_PATH --extract-root $(pwd)/.zap || return 1
  fi
}

# Main logic based on script name
if [[ "$SCRIPT_NAME" == "activate.sh" ]]; then
    activate || return 1
else
    bootstrap || return 1
fi

echo "💡 ZAP_INSTALL_PATH is $ZAP_INSTALL_PATH"

# Call mcux-env.sh if it exists
if [[ -f "$SDK_PATH/mcux-env.sh" ]]; then
  source "$SDK_PATH/mcux-env.sh" || return 1
else
  echo "ERROR: mcux-env.sh not found in $SDK_PATH."
  return 1
fi

# Export environment variables
export ZEPHYR_BASE="$ZEPHYR_BASE"
export PATH="$VENV_PATH/bin:$ZAP_INSTALL_PATH:$PATH"

echo "📌 export ZEPHYR_BASE="$ZEPHYR_BASE""
echo "📌 export PATH="$VENV_PATH/bin:$ZAP_INSTALL_PATH:\$PATH""

# Final step
echo "✅ Environment setup complete."