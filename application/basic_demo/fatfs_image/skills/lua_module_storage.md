# Lua Storage

This skill describes how to correctly use storage when writing Lua scripts.

## How to call
- Import it with `local storage = require("storage")`
- Call `storage.mkdir(path)` to create a directory
- Call `storage.write_file(path, content)` to write a file
- Call `storage.read_file(path)` to read a file

## Example
```lua
local storage = require("storage")

storage.mkdir("/fatfs/data/demo")
storage.write_file("/fatfs/data/demo/test.txt", "hello")
local text = storage.read_file("/fatfs/data/demo/test.txt")
```
