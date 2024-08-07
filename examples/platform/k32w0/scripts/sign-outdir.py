# Copyright 2022 NXP
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

import argparse
import os
import platform
import subprocess

def sign_cmd(tool, image, identifer):
    cmd = f"python {tool} -i={identifer} {image}"
    if args.simple_hash:
        cmd = cmd + " -SimpleHashVerification"
    cmd += f" && arm-none-eabi-objcopy -I srec -O binary {image}.srec {image}.bin"

    return cmd

def windows_sign_cmd(tool_path):
    # sign_images is a Linux wrapper over dk6_image_tool.py.
    # On Windows, use the Python script directly.
    tool = f"{tool_path}/../dk6_image_tool.py"

    image = [f for f in os.listdir(os.getcwd()) if f.startswith('chip-') and f.endswith('-example')][0]
    cmd = sign_cmd(tool, image, 1)

    if args.ota_enabled:
        # Also sign the SSBL with the correct image identifier.
        cmd += " && " + sign_cmd(tool, 'chip-k32w0x-ssbl', 0)

    return cmd

def linux_sign_cmd(tool_path):
    # Convert script to unix format if needed
    subprocess.call("(file " + tool_path + " | grep CRLF > /dev/null) && (dos2unix " + tool_path + ")", shell=True)
    cmd = tool_path + " " + os.getcwd()
    if args.simple_hash:
        cmd = cmd + " -SimpleHashVerification"

    return cmd

def main(args):
    if "NXP_K32W0_SDK_ROOT" in os.environ and os.environ["NXP_K32W0_SDK_ROOT"] != "":
        path_prefix = ""
        # Treat particular case of CI
        if "third_party/nxp/nxp_matter_support/github_sdk/k32w0/repo" in os.environ["NXP_K32W0_SDK_ROOT"]:
            path_prefix = "/core"
        tool_path = os.environ["NXP_K32W0_SDK_ROOT"] + path_prefix + "/tools/imagetool/sign_images.sh"
    else:
        tool_path = os.path.abspath(
            __file__ + "/../../../../../github_sdk/k32w0/repo/core/tools/imagetool/sign_images.sh")

    # Give execute permission if needed
    if os.access(tool_path, os.X_OK) is False:
        os.chmod(tool_path, 0o766)

    if platform.system() == "Windows":
        cmd = windows_sign_cmd(tool_path)
    else:
        cmd = linux_sign_cmd(tool_path)

    subprocess.call(cmd, shell=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--simple-hash",
        help="When enabled, adds a hash of the whole image at the end of the binary.",
        action="store_true"
    )
    parser.add_argument(
        "--ota-enabled",
        help="When enabled, OTA is enabled so image identifier should be 1 for app and 0 for SSBL.",
        action="store_true"
    )
    args = parser.parse_args()

    main(args)
