# Lua Delay

This skill describes how to correctly use delay when writing Lua scripts.

## How to call
- Import it with `local delay = require("delay")`
- Call `delay.delay_ms(ms)` to sleep for a number of milliseconds
- `ms` should be an integer greater than or equal to `0`

## Example
```lua
local delay = require("delay")
delay.delay_ms(500)
```
