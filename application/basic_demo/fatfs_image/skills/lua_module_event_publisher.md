# Lua Event Publisher

This skill describes how to correctly use event_publisher when writing Lua scripts.

## How to call
- Import it with `local event_publisher = require("event_publisher")`
- Call `event_publisher.publish_message({...})` to publish a message event
- Call `event_publisher.publish_trigger({...})` to publish a trigger event
- Call `event_publisher.publish({...})` to publish a full custom event

## Example
```lua
local event_publisher = require("event_publisher")

event_publisher.publish_message({
  source_cap = "lua_script",
  channel = "custom",
  chat_id = "demo",
  text = "hello"
})
```
