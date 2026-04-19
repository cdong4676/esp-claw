# Lua Script Patterns

Use this skill together with `cap_lua_edit` whenever you are about to write
or rewrite a Lua script. These are the canonical templates for common
scenarios in this firmware. Start from the template that matches the user's
intent, then swap in the real module calls.

All templates follow the rules in `cap_lua_edit`:
- `require` only documented modules.
- **IM from a button callback:** use `ep.publish_message("text")` (string). Do not use a table without `source_cap` — see `lua_module_event_publisher`.
- Never busy-wait: every loop calls `delay.delay_ms(ms)`.
- Open hardware in `run()`, release it in `cleanup()`, invoke both through
  `xpcall(run, debug.traceback)` + `cleanup()` + rethrow.
- Clamp floats with `math.floor(...)` before passing to hardware APIs.
- Stick to ASCII text on the display.

The patterns reference modules such as `display`, `button`, `led_strip`,
`audio`, `camera`, `lcd_touch`. Not every firmware build ships every
module. Before emitting a script, check the active `lua_module_*` skill
list and use only modules that are present. When a required module is not
available, change the design rather than inventing an API.

## Reference demos on-device (read these first)

When the user asks for something that looks similar to an existing demo,
`read_file` the matching demo **before** writing a new script and reuse
its `cleanup()` + `xpcall(..., debug.traceback)` skeleton.

**Important:** names below match `lua_list_scripts`. For the `read_file` tool, prefix them with **`scripts/`** (e.g. `scripts/button_demo.lua`, `scripts/temp/wechat_button_notify_skeleton.lua`) so the path resolves under `/fatfs/scripts/...`. Using the bare name makes `read_file` look under `/fatfs/` and returns not found.

| User intent | Canonical demo to read first |
|---|---|
| Button press / click handling | `button_demo.lua` |
| Button + IM notify (skeleton) | `temp/wechat_button_notify_skeleton.lua` |
| Button + audio combo | `button_play_test_wav.lua` |
| LED strip animation / indicator | `led_strip_demo.lua` |
| Audio playback / recording | `audio_demo.lua`, `audio_play_test_wav.lua` |
| Display drawing / UI | `display_demo.lua`, `clock_dial_demo.lua` |
| Camera still / preview | `camera_capture_demo.lua`, `camera_preview_demo.lua` |
| Touchscreen interaction | `lcd_touch_paint.lua` |
| Periodic polling loop | `button_demo.lua` (always use `delay.delay_ms` in the loop) |

When adapting a demo, preserve three things:
1. The `cleanup()` function and every `pcall(...)` inside it.
2. The outer `xpcall(run, debug.traceback)` + `cleanup()` + rethrow pair.
3. Any `args.*` surface the demo already exposes, so tool-generated scripts
   can be rerun with different parameters without another edit.

The shipped demos **`button_demo.lua`**, **`led_strip_demo.lua`**, and **`button_play_test_wav.lua`** follow this shape (no stray `board_manager` except the audio combo demo for codec lookup). Do not copy older scripts that open hardware at top level without `cleanup`.

## Pattern 1: One-shot utility (no hardware)

Use for quick system queries, text output, file checks.

```lua
local system = require("system")

local function run()
  local info = system.info()
  print(string.format("uptime_s=%s sram_free=%s",
        tostring(info.uptime_s or 0),
        tostring(info.sram_free or 0)))
end

local ok, err = xpcall(run, debug.traceback)
if not ok then
  error(err)
end
```

## Pattern 2: Finite animation on the display

Use for short effects that finish on their own, like a splash screen.
Note: `run` is the only place that opens hardware; `cleanup` is idempotent
and safe to call on both success and failure. The `display` module is a
global API (no handle); the one owned resource is the arbiter token taken
by `display.init` and released by `display.deinit`.

```lua
local bm      = require("board_manager")
local display = require("display")
local delay   = require("delay")

local FRAME_MS = 40
local FRAMES   = 60

local display_inited = false

local function cleanup()
  if display_inited then
    pcall(display.end_frame)
    pcall(display.deinit)
    display_inited = false
  end
end

local function run()
  local panel, io_h, w, h, panel_if = bm.get_display_lcd_params("display_lcd")
  display.init(panel, io_h, w, h, panel_if)
  display_inited = true

  for i = 1, FRAMES do
    display.begin_frame({ clear = true, r = 0, g = 0, b = 0 })
    local x = math.floor((w - 20) * i / FRAMES)
    display.fill_rect(x, math.floor(h / 2) - 10, 20, 20, 255, 255, 255)
    display.present()
    display.end_frame()
    delay.delay_ms(FRAME_MS)
  end
end

local ok, err = xpcall(run, debug.traceback)
cleanup()
if not ok then
  error(err)
end
```

## Pattern 3: Polling loop with an exit condition

Use for "watch until the user presses a button" style scripts. Button
events in this firmware are delivered through callbacks that fire inside
`button.dispatch()`. The loop body always calls `delay.delay_ms(...)`.

```lua
local button = require("button")
local delay  = require("delay")

local a        = type(args) == "table" and args or {}
local pin      = type(a.pin) == "number" and a.pin or 0
local max_ms   = type(a.max_ms) == "number" and a.max_ms or 30000

local btn
local should_exit = false

local function cleanup()
  if btn then
    pcall(button.off, btn)
    pcall(button.close, btn)
    btn = nil
  end
end

local function run()
  btn = button.new(pin, 0)
  button.on(btn, "single_click", function()
    should_exit = true
    -- If replying to the same IM chat: ep.publish_message("clicked")  (string form; see lua_module_event_publisher)
  end)

  local started = 0
  while not should_exit do
    button.dispatch()
    started = started + 10
    if started > max_ms then
      print("timeout, exiting")
      break
    end
    delay.delay_ms(10)
  end
  print("exit")
end

local ok, err = xpcall(run, debug.traceback)
cleanup()
if not ok then
  error(err)
end
```

## Pattern 4: Multi-resource script (audio + display)

Use when the script needs two or more hardware resources at once. Track
each resource in a flag or local handle and release them all from one
`cleanup()`. Note that `audio` handles are freed with `audio.close(h)`,
not `h:close()`.

```lua
local bm      = require("board_manager")
local display = require("display")
local audio   = require("audio")
local delay   = require("delay")
local storage = require("storage")

local display_inited = false
local rec_h
local out_h

local function cleanup()
  if rec_h then pcall(audio.close, rec_h); rec_h = nil end
  if out_h then pcall(audio.close, out_h); out_h = nil end
  if display_inited then
    pcall(display.end_frame)
    pcall(display.deinit)
    display_inited = false
  end
end

local function run()
  local panel, io_h, w, h, panel_if = bm.get_display_lcd_params("display_lcd")
  display.init(panel, io_h, w, h, panel_if)
  display_inited = true

  local in_codec, rate_in, ch_in, bits_in =
      bm.get_audio_codec_input_params("audio_adc")
  rec_h = audio.new_input(in_codec, rate_in, ch_in, bits_in)

  display.begin_frame({ clear = true, r = 0, g = 0, b = 0 })
  display.draw_text(12, 12, "Recording...", {
    r = 255, g = 255, b = 255, font_size = 24,
  })
  display.present()
  display.end_frame()

  audio.record_wav(rec_h,
    storage.join_path(storage.get_root_dir(), "clip.wav"), 2000)
  delay.delay_ms(10)
end

local ok, err = xpcall(run, debug.traceback)
cleanup()
if not ok then
  error(err)
end
```

## Pattern 5: Async worker started through `lua_run_script_async`

Use when the user wants a long-running behaviour (breathing LED, status
watcher, background chime) that they expect to stop later through the async
job API. Even though the loop runs until the job is cancelled or times out,
every iteration still calls `delay.delay_ms(...)`.

```lua
local led_strip = require("led_strip")
local delay     = require("delay")

local a   = type(args) == "table" and args or {}
local pin = type(a.pin) == "number" and a.pin or 38
local num = type(a.num) == "number" and a.num or 1
local ms  = type(a.ms)  == "number" and a.ms  or 40

local strip

local function cleanup()
  if strip then
    pcall(strip.clear, strip)
    pcall(strip.close, strip)
    strip = nil
  end
end

local function run()
  strip = led_strip.new(pin, num)
  local i = 0
  while true do
    local v = math.floor((math.sin(i / 10) + 1) * 127)
    for idx = 0, num - 1 do
      strip:set_pixel(idx, v, v, v)
    end
    strip:refresh()
    i = i + 1
    delay.delay_ms(ms)
  end
end

local ok, err = xpcall(run, debug.traceback)
cleanup()
if not ok then
  error(err)
end
```

## Anti-patterns (do not emit)

These examples are **wrong**. They are listed here so you can recognise and
refuse them, not so you can reuse them.

```lua
-- BAD: busy-wait delay, will trip the watchdog.
for i = 1, 1000000 do end

-- BAD: blocking loop without delay, starves other tasks.
while true do
  button.dispatch()
end

-- BAD: no cleanup on error path, any error between init and deinit
-- leaves the display locked and unusable for later scripts.
display.init(panel, io_h, w, h, panel_if)
display.draw_text(0, 0, "hi", { r = 255, g = 255, b = 255 })
display.present()
display.deinit()

-- BAD: float passed to hardware API, crashes inside C layer.
display.draw_rect(w / 3, h / 3, 10, 10, 255, 0, 0)

-- BAD: using an API that does not exist. The firmware only ships
-- the modules listed in the activated lua_module_* skills; there is
-- no `require("socket")`, `require("lfs")`, or `require("io")`.
local sock = require("socket")
```
