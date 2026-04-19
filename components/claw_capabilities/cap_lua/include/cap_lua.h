/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "claw_core.h"
#include "esp_err.h"
#include "lua.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *name;
    lua_CFunction open_fn;
} cap_lua_module_t;

typedef void (*cap_lua_runtime_cleanup_fn_t)(void);

const char *cap_lua_get_base_dir(void);
esp_err_t cap_lua_register_group(void);
esp_err_t cap_lua_set_base_dir(const char *base_dir);
esp_err_t cap_lua_register_module(const char *name, lua_CFunction open_fn);
esp_err_t cap_lua_register_modules(const cap_lua_module_t *modules, size_t count);
esp_err_t cap_lua_register_runtime_cleanup(cap_lua_runtime_cleanup_fn_t cleanup_fn);
esp_err_t cap_lua_list_scripts(const char *prefix, char *output, size_t output_size);
esp_err_t cap_lua_write_script(const char *path,
                               const char *content,
                               bool overwrite,
                               char *output,
                               size_t output_size);
esp_err_t cap_lua_run_script(const char *path,
                             const char *args_json,
                             uint32_t timeout_ms,
                             char *output,
                             size_t output_size);

/*
 * Submit a managed Lua script to run asynchronously.
 *
 * - timeout_ms == 0  : no wall-clock deadline (cancel-only).
 * - name             : optional handle (defaults to script basename).
 * - exclusive        : optional mutex group (e.g. "display"); same group can
 *                      have at most one running job.
 * - replace          : if true, conflicting jobs (same name OR exclusive) are
 *                      stopped before this one starts.
 */
esp_err_t cap_lua_run_script_async(const char *path,
                                   const char *args_json,
                                   uint32_t timeout_ms,
                                   const char *name,
                                   const char *exclusive,
                                   bool replace,
                                   char *output,
                                   size_t output_size);
esp_err_t cap_lua_list_jobs(const char *status, char *output, size_t output_size);
esp_err_t cap_lua_get_job(const char *id_or_name, char *output, size_t output_size);
esp_err_t cap_lua_stop_job(const char *id_or_name,
                           uint32_t wait_ms,
                           char *output,
                           size_t output_size);
esp_err_t cap_lua_stop_all_jobs(const char *exclusive_filter,
                                uint32_t wait_ms,
                                char *output,
                                size_t output_size);

/* Context provider that injects a snapshot of active async jobs into LLM context. */
extern const claw_core_context_provider_t cap_lua_async_jobs_provider;

/*
 * Diagnostic completion observer (matches claw_core_completion_observer_fn):
 * detects assistant replies that claim to have stopped / cancelled / cleared
 * an async script while the active-jobs context was injected and no matching
 * stop tool was invoked this turn. Emits ESP_LOGW only — no behaviour change.
 *
 * Wire this once during boot via claw_core_add_completion_observer().
 */
void cap_lua_honesty_observe_completion(const claw_core_completion_summary_t *summary,
                                        void *user_ctx);

#ifdef __cplusplus
}
#endif
