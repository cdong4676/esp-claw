# Scheduler Management

Use this skill when the user needs to inspect or control timer-based schedule rules.

## When to use
- The user asks to list current schedules.
- The user asks to add a new once/interval/cron schedule.
- The user asks to pause, resume, enable, disable, trigger, or reload a schedule.
- The user asks for a timer-based reminder, automation, periodic check, or timed
  agent wake-up.

## Available capabilities
- `scheduler_list`: list all scheduler entries and runtime state.
- `scheduler_get`: get one scheduler entry by id.
- `scheduler_add`: add a new scheduler entry from `schedule_json` string.
- `scheduler_enable`: enable one scheduler entry by id.
- `scheduler_disable`: disable one scheduler entry by id.
- `scheduler_pause`: pause one scheduler entry by id.
- `scheduler_resume`: resume one scheduler entry by id.
- `scheduler_trigger_now`: trigger one scheduler entry immediately.
- `scheduler_reload`: reload scheduler definitions from disk.

## `scheduler_add` input format
- `schedule_json` is a JSON string, not an object.
- Use a stable, unique `id`. `scheduler_add` fails if the id already exists.

```json
{
  "schedule_json": "<JSON string of one scheduler entry object>"
}
```

## `schedule_json` object fields
- `id`: schedule unique id.
- `enabled`: whether schedule is active. Defaults to `true`.
- `kind`: `once`, `interval`, or `cron`.
- `start_at_ms`: absolute Unix timestamp in milliseconds. Required for `once`;
  optional anchor for `interval`. Use `0` when no anchor is needed.
- `end_at_ms`: optional absolute stop time in milliseconds. `0` means no stop
  time.
- `interval_ms`: interval period for `interval`.
- `cron_expr`: 5-field cron expression for `cron` (`minute hour mday month wday`),
  without seconds.
- `event_type`: event type published to the router. Defaults to `schedule`; keep
  this value for normal scheduler automations.
- `event_key`: logical key for router matching and tracing. Defaults to `id`.
- `source_channel`: event source channel (for example `time`, `qq`, `telegram`).
- `chat_id`: target chat id when the router action needs one.
- `content_type`: event content type. Defaults to `trigger`.
- `session_policy`: session policy (`trigger`, `chat`, `global`, `ephemeral`,
  `nosave`). Defaults to `trigger`.
- `text`: event text payload. For agent wake-up rules, write this as the prompt
  the agent should reason about.
- `payload_json`: user structured payload. Defaults to `{}`. At trigger time the
  scheduler wraps it under `user_payload` and also adds schedule metadata.
- `max_runs`: max trigger count. `0` means unlimited.

## Time semantics
- Choose `interval` for relative periodic execution, such as "every 10 seconds",
  "every 5 minutes", "once per hour after startup", or general periodic polling, or when there are offline running needs.
  If `start_at_ms` is `0`, the next fire time is `now + interval_ms`. If
  `start_at_ms` is set, it is used as the interval anchor.
- Choose `cron` for calendar-based repeated execution, such as "every day at
  08:00", "every Monday", "on the 1st day of each month", or "every 3 minutes
  aligned to wall-clock time". `cron` uses the device time zone stored in NVS
  and depends on valid wall-clock time. Supported field forms are
  `*`, `*/N`, or one explicit number in range; for example, `*/3 * * * *`.
- Choose `once` for one-time execution at a specific absolute time, such as a
  deadline. `start_at_ms` is required and valid time sync is required.

## MUST work with Event Router
- Scheduler entries only publish events; they do not define post-trigger behavior.
- For normal scheduled work, set `event_type` to `schedule` and add a matching
  router rule using `match.event_type` + `match.event_key`.
- Router rule operations and formats are documented in `cap_router_mgr.md`
  (`add_router_rule` / `update_router_rule`).
- Always choose one router action strategy before adding the schedule:
  - Send message to user directly: default strategy. Use it when the scheduled
    action is deterministic and can be handled by router actions, scripts, or
    fixed capability calls.
  - Send message to wake up agent: expensive strategy. Use it when the scheduled
    task requires the agent to reason from current context, decide what to do,
    choose skills/tools, summarize/check a changing situation, or handle
    complicated issues.

## Router action strategy

### Strategy 1: send message to user directly
- Use this for fixed reminders, fixed outbound messages, voice reminders,
  periodic local automation, polling, cleanup, logging, and other deterministic
  work.
- Keep the scheduler event as `event_type: "schedule"` and
  `content_type: "trigger"`.
- Add a router rule whose match is exactly the schedule event:
  `{"event_type":"schedule","event_key":"<event_key>"}`.
- Use deterministic router actions such as `send_message`, `call_cap`,
  `run_script`.
- Put the final **user-facing message** in the router action, not in the scheduler
  `text`, unless the action intentionally renders `{{event.text}}`.

### Strategy 2: send message to wake up agent
- Use this only when each trigger should spend LLM tokens and behave like a new
  agent task.
- Keep the scheduler event as `event_type: "schedule"` and add a matching router
  rule with action type `run_agent`.
- Put the agent instruction in `run_agent.input.text` or pass through
  `{{event.text}}`.
- Set `target_channel` and `target_chat_id` in the `run_agent` input when the
  eventual agent response should be delivered to a chat.
- Prefer `session_policy: "trigger"` for an independent recurring task thread.
  Use `chat` only when the scheduled run should share the chat session history.
- If the project does not already route `agent_response` events to outbound
  messages, add or verify that router rule separately.

## Recommended workflow
1. Use `scheduler_list` / `scheduler_get` and `list_router_rules` /
   `get_router_rule` to inspect current state and avoid id conflicts.
2. Choose the time kind: `once`, `interval`, or `cron`.
3. Choose the router action strategy: direct deterministic action or agent
   wake-up.
4. Add one scheduler entry with a stable `event_key`.
5. Add or update the matching router rule in `cap_router_mgr`.

## Common failure causes
- Adding only the schedule and forgetting the matching router rule.
- Using `event_type: "message"` for a simple reminder, causing unnecessary LLM
  usage.
- Mismatched `event_key` between the schedule and `match.event_key`.
- Using a 6-field cron expression with seconds; only 5 fields are supported.
- Creating a `once` schedule without a positive `start_at_ms`.
- Omitting `chat_id`, `target_channel`, or `target_chat_id` when the action must
  produce a visible chat message.
- Waking the agent without an `agent_response` delivery rule.

## Examples

### `interval + send_message`

Scheduler entry: trigger every 30 seconds, 3 times in total.

```json
{
  "schedule_json": "{\"id\":\"drink_reminder_30s\",\"enabled\":true,\"kind\":\"interval\",\"start_at_ms\":0,\"end_at_ms\":0,\"interval_ms\":30000,\"cron_expr\":\"\",\"event_type\":\"schedule\",\"event_key\":\"drink_reminder\",\"source_channel\":\"time\",\"chat_id\":\"a_certain_QQ_chat_ID\",\"content_type\":\"trigger\",\"session_policy\":\"trigger\",\"text\":\"drink reminder tick\",\"payload_json\":\"{\\\"message\\\":\\\"time to drink water\\\"}\",\"max_runs\":3}"
}
```

Matching router rule: send a fixed QQ message.

```json
{
  "rule_json": "{\"id\":\"drink_reminder_to_qq\",\"description\":\"Send drink reminder to QQ\",\"enabled\":true,\"consume_on_match\":true,\"match\":{\"event_type\":\"schedule\",\"event_key\":\"drink_reminder\"},\"actions\":[{\"type\":\"send_message\",\"input\":{\"channel\":\"qq\",\"chat_id\":\"a_certain_QQ_chat_ID\",\"message\":\"It's time to drink water!\"}}]}"
}
```

### `cron + run_agent`

Scheduler entry: trigger every day at 08:00 and publish a schedule event to make agent check the weather and tell the user what should be prepared for going out.

```json
{
  "schedule_json": "{\"id\":\"daily_agent_check\",\"enabled\":true,\"kind\":\"cron\",\"start_at_ms\":0,\"end_at_ms\":0,\"interval_ms\":0,\"cron_expr\":\"0 8 * * *\",\"event_type\":\"schedule\",\"event_key\":\"daily_agent_check\",\"source_channel\":\"time\",\"chat_id\":\"a_certain_QQ_chat_ID\",\"content_type\":\"trigger\",\"session_policy\":\"trigger\",\"text\":\"Please check the weather today and tell me what I should prepare for going out.\",\"payload_json\":\"{}\",\"max_runs\":0}"
}
```

Matching router rule: wake the agent and let it reason before responding.

```json
{
  "rule_json": "{\"id\":\"daily_agent_check_agent\",\"description\":\"Wake agent for daily scheduled check\",\"enabled\":true,\"consume_on_match\":true,\"match\":{\"event_type\":\"schedule\",\"event_key\":\"daily_agent_check\"},\"actions\":[{\"type\":\"run_agent\",\"input\":{\"target_channel\":\"qq\",\"target_chat_id\":\"a_certain_QQ_chat_ID\",\"session_policy\":\"chat\"}}]}"
}
```

### Pause a schedule

```json
{
  "id": "sedentary_reminder"
}
```
