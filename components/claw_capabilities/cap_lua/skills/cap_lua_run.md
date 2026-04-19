# Lua Script Execution

Use this skill when the user wants to see existing Lua scripts, run one, or inspect async execution jobs.

## Speed

- **`lua_list_scripts`** once per task is usually enough; avoid repeated listing while iterating on the same `temp/*.lua`.
- When the agent invokes `lua_run_script_async` from an IM chat, **`args.channel` / `args.chat_id` are often injected** — you do **not** need to duplicate them in the tool JSON unless you intentionally target another chat. Omitting redundant `args` keeps the tool call smaller and faster to validate.
- Prefer **`lua_run_script_async`** only for long-running scripts; use **`lua_run_script`** for quick one-shot checks to get output in the same turn.
- To **read** an existing `.lua` source with `read_file`, use **`scripts/`** + the same relative path `lua_list_scripts` shows (e.g. `scripts/button_demo.lua`), not the bare filename.

## Rules
- Call the direct capability entrypoints, not `cap_cli`.
- Use `lua_list_scripts` to inspect files. `prefix` is optional and must also be a relative path under the Lua base directory.
- Use `lua_run_script` for short tasks that should return output immediately.
- Use `lua_run_script_async` for loops, animations, watchers, or other long-running behavior.
- Use `lua_list_async_jobs` and `lua_get_async_job` to inspect async execution state.
- `path` must be a relative `.lua` path under the configured Lua base directory.
- Newly authored scripts that are still being validated must run from `temp/*.lua`.
- `args` may be an object or array. The runtime exposes it to Lua as the global `args`.
- When the tool is invoked from an IM chat session, the firmware may merge `channel`, `chat_id`, and `session_id` into `args` if those keys are absent (so Lua can send messages back to the same chat).
- `timeout_ms` is optional, but when present it must be a positive integer.

## Minimal Examples
```json
{"path":"hello.lua"}
```

```json
{"path":"blink.lua","args":{"pin":2},"timeout_ms":3000}
```

## Guidance
- If the target script does not exist yet, switch to the Lua authoring flow first.
- Prefer re-running the same relative path while iterating on a script instead of creating many near-duplicate files.
- After running a temporary script, keep using the same `temp/*.lua` path for revisions until the user confirms it should be kept.
- When saving the confirmed version, move or rewrite it under `user/` rather than keeping it in `temp/`.
