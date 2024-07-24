# NXP Matter support repo

This repository contains NXP Matter support files required to support the Matter stack on NXP platforms.

# Directory Structure

| File/Folder       | Content                                                                                                                                                             |
| ----------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| examples/platform | Common NXP platform example files required to support a matter application in an NXP platform  such linker file, board files...                                     |
| gn_build          | Contains GN build files allowing to build NXP SDK files required to support a Matter application build using GN                                                     |
| scripts           | Contains NXP scripts. For example it contains a script allowing to help to intialize NXP SDKs                                                                       |
| github_sdk        | Contains west manifest configuration indicating recommended SDK modules to be used with Matter. It is also the default location where NXP github SDK will be cloned |