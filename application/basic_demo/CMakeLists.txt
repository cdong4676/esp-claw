cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS
            "../../components/claw_modules"
            "../../components/lua_modules"
            "../../components/claw_capabilities"
            )
if(NOT DEFINED IDF_TARGET)
    set(IDF_TARGET "esp32s3" CACHE STRING "ESP-IDF target")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(basic_demo)

include("${CMAKE_SOURCE_DIR}/tools/cmake/skills_sync.cmake")

fatfs_create_spiflash_image(storage fatfs_image FLASH_IN_PROJECT)
basic_demo_enable_component_skills_sync(
    TARGET fatfs_storage_bin
    DEMO_SKILLS_DIR "${CMAKE_SOURCE_DIR}/fatfs_image/skills"
    MANIFEST_PATH "${CMAKE_BINARY_DIR}/component_skills_manifest.json"
)
