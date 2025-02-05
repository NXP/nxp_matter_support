#!/usr/bin/env python3
#
# Copyright (c) 2024 Project CHIP Authors
# Copyright 2024 NXP
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import argparse
import logging
import os
import shutil
import subprocess
import sys
import yaml
from dataclasses import dataclass

NXP_MATTER_SUPPORT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))


@dataclass(init=False)
class NxpSdk:
    sdk_name: str
    sdk_target_location_abspath: str
    sdk_target_location_relativepath_from_root: str
    sdk_manifest_path: str
    sdk_manifest_name: str
    sdk_storage_location_abspath: str
    sdk_storage_location_relativepath_from_root: str

    def __init__(self, name, sdk_target_relative_path):
        self.sdk_name = name
        self.sdk_target_location_abspath = os.path.join(NXP_MATTER_SUPPORT_ROOT, sdk_target_relative_path)
        self.sdk_target_location_relativepath_from_root = sdk_target_relative_path
        self.sdk_manifest_path = os.path.abspath(os.path.join(self.sdk_target_location_abspath , 'manifest'))
        self.sdk_manifest_name = 'west.yml'
        self.get_sdk_storage_location()

    def get_sdk_storage_location(self):
        westFilePath = os.path.abspath(os.path.join(self.sdk_manifest_path, self.sdk_manifest_name))
        if (os.path.exists(westFilePath)):
            westStream = open(westFilePath, 'r')
            west = yaml.load(westStream, Loader=yaml.SafeLoader)
            westStream.close()
            try:
                path_prefix = west["manifest"]["projects"][0]["import"]["path-prefix"]
                #In case of sdk_next SDK, all SDK modules would be stored in mcuxsdk folder therefore add it to path_prefix
                if ("sdk_next" in self.sdk_target_location_relativepath_from_root):
                    path_prefix = os.path.join(path_prefix, 'mcuxsdk')
                self.sdk_storage_location_relativepath_from_root = os.path.join(NXP_MATTER_SUPPORT_ROOT, self.sdk_target_location_relativepath_from_root , path_prefix)
                self.sdk_storage_location_abspath = os.path.abspath(self.sdk_storage_location_relativepath_from_root)
            except (KeyError) as exception:
                logging.error("Wrong west file format %s", exception)
                sys.exit(1)
        else:
            logging.error("SDK west config file : %s does not exist", westFilePath)
            sys.exit(1)

def NxpSdk_k32w0():
    sdk = NxpSdk('k32w0', 'github_sdk/k32w0')
    return sdk

def NxpSdk_common():
    sdk = NxpSdk('common', 'github_sdk/sdk_next')
    return sdk

ALL_PLATFORM_SDK = [
    NxpSdk_k32w0(),
    NxpSdk_common(),
]

ALL_PLATFORM_NAME = [p.sdk_name for p in ALL_PLATFORM_SDK]


def clean_sdk_local_changes(sdk_location):
    logging.warning("SDK will be cleaned all local modification(s) will be lost")
    # Cleaning all local modifications
    git_clean_command = "git reset --hard && git clean -xdf"
    command = ['west', 'forall', '-c', git_clean_command, '-a']
    subprocess.run(command, cwd=sdk_location, check=True)


def init_nxp_sdk_version(nxp_sdk, force):
    print("Init SDK in: " + nxp_sdk.sdk_target_location_abspath)
    west_path = os.path.join(nxp_sdk.sdk_target_location_abspath, '.west')
    if os.path.exists(west_path):
        if not force:
            logging.error("SDK is already initialized, use --force to force init")
            sys.exit(1)
        shutil.rmtree(west_path)

    command = ['west', 'init', '-l', nxp_sdk.sdk_manifest_path, '--mf', nxp_sdk.sdk_manifest_name]
    subprocess.run(command, check=True)
    # In case of SDK next architecture a dedicated update process is required
    if ("sdk_next" not in nxp_sdk.sdk_target_location_relativepath_from_root):
        update_nxp_sdk_version(nxp_sdk, force)
    else:
        update_nxp_sdk_next_version(nxp_sdk, force)

def update_nxp_sdk_next_version(nxp_sdk, force):
    print("Update SDK in " + nxp_sdk.sdk_target_location_abspath)
    if not os.path.exists(os.path.join(nxp_sdk.sdk_target_location_abspath, '.west')):
        logging.error("--update-only error SDK is not initialized")
        sys.exit(1)
    try:
        # check if we are in the internal development environment
        subprocess.run(['git', 'config', '--global', '--get-all', 'url.ssh://git@bitbucket.sw.nxp.com/mcucore/bifrost.git.insteadof'], check=True)
        # in case of development enrironment update URL remote location
        subprocess.run(['west', 'update', 'bifrost'], cwd=nxp_sdk.sdk_target_location_abspath, check=True)
        url_wrapper_dest = "includeif.gitdir:" + str(os.path.abspath(os.path.join(nxp_sdk.sdk_target_location_abspath,'.path')).replace('\\','/'))
        url_wrapper_src = os.path.abspath(os.path.join(nxp_sdk.sdk_target_location_abspath, 'repo/bifrost/.gitconfig').replace('\\','/'))
        subprocess.run(['git', 'config', '--global', url_wrapper_dest, url_wrapper_src], cwd=nxp_sdk.sdk_target_location_abspath, check=True)
    except (subprocess.CalledProcessError) as exception:
        #Ignore error as we are not in internal development environment
        pass
    try:
        subprocess.run(['west', 'config', 'commands.allow_extensions', 'true'], cwd=nxp_sdk.sdk_target_location_abspath, check=True)
        subprocess.run(['west', 'update', '--fetch', 'smart'], cwd=nxp_sdk.sdk_target_location_abspath, check=True)
        subprocess.run(['west', 'mcuxsdk-export'], cwd=nxp_sdk.sdk_target_location_abspath, check=True)
    except (RuntimeError, subprocess.CalledProcessError) as exception:
        logging.exception(
            'Error SDK cannot be updated, local changes may need to be cleaned manually %s', exception)


def update_nxp_sdk_version(nxp_sdk, force):
    print("Update SDK in " + nxp_sdk.sdk_target_location_abspath)
    if not os.path.exists(os.path.join(nxp_sdk.sdk_target_location_abspath, '.west')):
        logging.error("--update-only error SDK is not initialized")
        sys.exit(1)
    command = ['west', 'update', '--fetch', 'smart']
    try:
        subprocess.run(command, cwd=nxp_sdk.sdk_target_location_abspath, check=True)
    except (RuntimeError, subprocess.CalledProcessError) as exception:
        if force:
            # In case of force update option and in case of update failure:
            # 1. try to clean all local modications if any
            # 2. Retry the west update command. It should be successfull now as all local modifications have been cleaned
            clean_sdk_local_changes(nxp_sdk.sdk_target_location_abspath)
            subprocess.run(command, cwd=nxp_sdk.sdk_target_location_abspath, check=True)
        else:
            logging.exception(
                'Error SDK cannot be updated, local changes should be cleaned manually or use --force to force update %s', exception)


def main():

    parser = argparse.ArgumentParser(description='Checkout or update relevant NXP SDK')
    parser.add_argument(
        "--update-only", help="Update NXP SDK to the correct version. Would fail if the SDK does not exist", action="store_true")
    parser.add_argument('--platform', nargs='+', choices=ALL_PLATFORM_NAME, default=ALL_PLATFORM_NAME,
                        help='Allows to select which SDK for a particular NXP platform to initialize')
    parser.add_argument('--force', action='store_true',
                        help='Force SDK initialization, hard clean will be done in case of failure - WARNING -- All local SDK modification(s) will be lost')

    args = parser.parse_args()

    for current_plat in args.platform:
        nxp_sdk = [p for p in ALL_PLATFORM_SDK if p.sdk_name == current_plat][0]
        if args.update_only:
            update_nxp_sdk_version(nxp_sdk, args.force)
        else:
            init_nxp_sdk_version(nxp_sdk, args.force)


if __name__ == '__main__':
    main()