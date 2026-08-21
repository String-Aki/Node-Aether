cmake_minimum_required(VERSION 3.16)

# Add microlink-v2 component path
set(EXTRA_COMPONENT_DIRS
    "/home/deepak/Stash/DEV/Projects/ESP32-S3/microlink/components"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Node-Aether_Socket_Controller)
