/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "cap_lua.h"
#include "esp_err.h"

#define CAP_LUA_DEFAULT_BASE_DIR        "/spiffs/lua"
#define CAP_LUA_MAX_SCRIPT_SIZE         (16 * 1024)
#define CAP_LUA_OUTPUT_SIZE             (4 * 1024)
#define CAP_LUA_SYNC_DEFAULT_TIMEOUT_MS 60000
/*
 * Async jobs default to "until cancelled" so long-running clocks, watchers,
 * and games can run indefinitely. Use lua_stop_async_job to terminate them.
 */
#define CAP_LUA_ASYNC_DEFAULT_TIMEOUT_MS 0
#define CAP_LUA_ASYNC_MAX_JOBS          16
#define CAP_LUA_ASYNC_MAX_CONCURRENT    4
/*
 * Stack lives in internal DRAM (FATFS / display drivers expect it). 12 KB is
 * the empirically-tuned minimum that still fits typical Lua scripts with a
 * comfortable margin; reducing further increases the risk of stack overflow
 * in deeper Lua call chains, while raising it makes allocation more likely to
 * fail once the heap is fragmented after boot.
 */
#define CAP_LUA_ASYNC_STACK             (12 * 1024)
#define CAP_LUA_ASYNC_PRIO              4
#define CAP_LUA_MAX_MODULES             16

#define CAP_LUA_JOB_NAME_MAX            32
#define CAP_LUA_JOB_EXCLUSIVE_MAX       16
#define CAP_LUA_JOB_PATH_MAX            192
#define CAP_LUA_JOB_ID_LEN              9      /* 8 hex + NUL */
#define CAP_LUA_STOP_WAIT_DEFAULT_MS    2000   /* sync wait window for stop */

typedef struct {
    char path[CAP_LUA_JOB_PATH_MAX];
    char name[CAP_LUA_JOB_NAME_MAX];
    char exclusive[CAP_LUA_JOB_EXCLUSIVE_MAX];
    char *args_json;
    uint32_t timeout_ms;
    bool replace;
    time_t created_at;
} cap_lua_async_job_t;

typedef enum {
    CAP_LUA_JOB_QUEUED = 0,
    CAP_LUA_JOB_RUNNING,
    CAP_LUA_JOB_DONE,
    CAP_LUA_JOB_FAILED,
    CAP_LUA_JOB_TIMEOUT,
    CAP_LUA_JOB_STOPPED,
} cap_lua_job_status_t;

typedef struct {
    char job_id[CAP_LUA_JOB_ID_LEN];
    char name[CAP_LUA_JOB_NAME_MAX];
    char exclusive[CAP_LUA_JOB_EXCLUSIVE_MAX];
    char path[CAP_LUA_JOB_PATH_MAX];
    cap_lua_job_status_t status;
    time_t created_at;
    time_t started_at;
    time_t finished_at;
} cap_lua_async_job_snapshot_t;

const char *cap_lua_get_base_dir(void);
bool cap_lua_path_is_valid(const char *path);
esp_err_t cap_lua_resolve_path(const char *path, char *resolved, size_t resolved_size);
esp_err_t cap_lua_ensure_base_dir(void);

esp_err_t cap_lua_runtime_init(void);

/*
 * Execute a Lua script synchronously.
 *
 * stop_requested may be NULL; when non-NULL the runtime hook will raise
 * a Lua error tagged "stopped by user" once the flag becomes true.
 *
 * timeout_ms == 0 means "no wall-clock deadline".
 */
esp_err_t cap_lua_runtime_execute_file(const char *path,
                                       const char *args_json,
                                       uint32_t timeout_ms,
                                       volatile bool *stop_requested,
                                       char *output,
                                       size_t output_size);
esp_err_t cap_lua_register_builtin_modules(void);
size_t cap_lua_get_module_count(void);
const cap_lua_module_t *cap_lua_get_module(size_t index);
size_t cap_lua_get_runtime_cleanup_count(void);
cap_lua_runtime_cleanup_fn_t cap_lua_get_runtime_cleanup(size_t index);

esp_err_t cap_lua_async_init(void);
esp_err_t cap_lua_async_start(void);
esp_err_t cap_lua_async_submit(const cap_lua_async_job_t *job,
                               char *job_id_out,
                               size_t job_id_out_size,
                               char *err_out,
                               size_t err_out_size);
esp_err_t cap_lua_async_list_jobs(const char *status_filter,
                                  char *output,
                                  size_t output_size);
esp_err_t cap_lua_async_get_job(const char *id_or_name,
                                char *output,
                                size_t output_size);
esp_err_t cap_lua_async_stop_job(const char *id_or_name,
                                 uint32_t wait_ms,
                                 char *output,
                                 size_t output_size);
esp_err_t cap_lua_async_stop_all_jobs(const char *exclusive_filter,
                                      uint32_t wait_ms,
                                      char *output,
                                      size_t output_size);
size_t cap_lua_async_collect_active_snapshots(cap_lua_async_job_snapshot_t *out,
                                              size_t max);
const char *cap_lua_job_status_name(cap_lua_job_status_t status);
/* Lightweight single-job status probe. Returns ESP_ERR_NOT_FOUND if the slot
 * has already been recycled. summary_out may be NULL. */
esp_err_t cap_lua_async_get_status(const char *job_id,
                                   cap_lua_job_status_t *out_status,
                                   char *summary_out,
                                   size_t summary_out_size);
/* Block up to timeout_ms waiting for the job to reach a terminal state. If
 * the job is already terminal, returns immediately. Returns the latest status
 * snapshot regardless of whether the wait timed out. */
esp_err_t cap_lua_async_wait_settle(const char *job_id,
                                    uint32_t timeout_ms,
                                    cap_lua_job_status_t *out_status,
                                    char *summary_out,
                                    size_t summary_out_size);
