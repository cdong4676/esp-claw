/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdbool.h>

#include "llm/claw_llm_types.h"

esp_err_t claw_llm_http_post_json(const claw_llm_http_json_request_t *request,
                                  claw_llm_http_response_t *out_response,
                                  char **out_error_message);
void claw_llm_http_response_free(claw_llm_http_response_t *response);

/*
 * Cooperative abort for the calling task's in-flight HTTP request.
 *
 * `arm` records (flag, current task handle) in a file-static slot. While armed,
 * the HTTP event handler polls *flag on every callback (HEADER/DATA/...) and
 * returns ESP_FAIL when the flag is true, causing esp_http_client_perform to
 * exit with an error and claw_llm_http_post_json to return ESP_FAIL.
 *
 * The arm/disarm pair MUST be called from the same task that will run
 * claw_llm_http_post_json. The flag must outlive the HTTP call. Only one
 * task may have an armed flag at a time; arming again from a different task
 * silently overwrites the previous slot, which is fine because the LLM
 * runtime serializes requests in claw_core_task.
 */
void claw_llm_http_arm_abort(volatile bool *flag);
void claw_llm_http_disarm_abort(void);
