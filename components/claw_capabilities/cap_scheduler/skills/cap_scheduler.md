# Scheduler Management

Use this skill when the user needs to inspect or control timer-based schedule rules.

## When to use
- The user asks to list current schedules.
- The user asks to add a new once/interval/cron schedule.
- The user asks to pause, resume, enable, disable, trigger, or reload a schedule.

## Available capabilities
- `scheduler_list`: list all scheduler entries and runtime state.
- `scheduler_get`: get one scheduler entry by id.
- `scheduler_add`: add one scheduler entry from a JSON definition.
- `scheduler_enable`: enable one scheduler entry by id.
- `scheduler_disable`: disable one scheduler entry by id.
- `scheduler_pause`: pause one scheduler entry by id.
- `scheduler_resume`: resume one scheduler entry by id.
- `scheduler_trigger_now`: trigger one scheduler entry immediately.
- `scheduler_reload`: reload scheduler definitions from disk.

## Calling rules
- Use direct scheduler capabilities. Do not route through `run_cli_command` unless explicitly requested.
- Always include `id` for single-entry operations.
- For `scheduler_add`, include at least:
- `id`
- `kind` in `once`, `interval`, or `cron`
- `once` requires `start_at_ms`.
- `interval` requires `interval_ms`.
- `cron` requires `cron_expr`.
- Keep `payload_json` as a JSON string, for example `"{\"message\":\"tick\"}"`.

## Recommended workflow
1. Use `scheduler_list` or `scheduler_get` to confirm current state.
2. Apply one mutation (`scheduler_add`, `scheduler_enable`, `scheduler_pause`, etc.).
3. Re-check with `scheduler_get` to confirm the expected runtime state.

## Examples

Add an interval schedule:
```json
{
  "id": "daily_health_ping",
  "kind": "interval",
  "enabled": true,
  "interval_ms": 60000,
  "timezone": "CST-8",
  "text": "health ping",
  "payload_json": "{\"message\":\"health ping\"}"
}
```

Add a cron schedule:
```json
{
  "id": "hourly_digest",
  "kind": "cron",
  "enabled": true,
  "cron_expr": "0 * * * *",
  "timezone": "CST-8",
  "text": "hourly digest"
}
```

Pause a schedule:
```json
{
  "id": "hourly_digest"
}
```
