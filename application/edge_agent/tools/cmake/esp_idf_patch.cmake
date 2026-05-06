set(EDGE_AGENT_PROJECT_LOG_PREFIX "[edge_agent]")
set(EDGE_AGENT_ESP_IDF_PATCH "${CMAKE_SOURCE_DIR}/tools/esp-idf.patch")

if(NOT DEFINED ENV{IDF_PATH} OR "$ENV{IDF_PATH}" STREQUAL "")
    message(FATAL_ERROR "${EDGE_AGENT_PROJECT_LOG_PREFIX} IDF_PATH environment variable is not set")
endif()

if(EXISTS "${EDGE_AGENT_ESP_IDF_PATCH}")
    message(STATUS "${EDGE_AGENT_PROJECT_LOG_PREFIX} Skipping ESP-IDF patch (patch files already modified): ${EDGE_AGENT_ESP_IDF_PATCH}")
else()
    message(FATAL_ERROR "${EDGE_AGENT_PROJECT_LOG_PREFIX} ESP-IDF patch file not found: ${EDGE_AGENT_ESP_IDF_PATCH}")
endif()