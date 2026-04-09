# Lua Script Editing

Use this skill when the user wants to create or modify a Lua script.

## Module Rule
- Before writing Lua code, first activate all available `lua_module_xxx` skills.
- Read those skills to learn which Lua modules are available and how to call them.
- Treat the activated `lua_module_xxx` skills as the source of truth for usable Lua modules in this firmware.

## Import Rule
- Only use Lua modules that are described by the activated `lua_module_xxx` skills.
- Do not use `require(...)` for any module outside that set.
- Do not assume standard Lua libraries or third-party Lua modules are available unless they are explicitly described by an activated `lua_module_xxx` skill.
- If a needed capability is not covered by the activated `lua_module_xxx` skills, do not invent an API. Change the script design to use only the available modules.

## Writing Rule
- Prefer small, direct scripts that only depend on the available Lua modules.
- When using a module, follow the exact import name and function names documented by its `lua_module_xxx` skill.
- If the user asks for behavior that depends on GPIO, delay, storage, LED strip, or event publishing, activate the matching module skills first and then write the script.

## Workflow
1. Activate all `lua_module_xxx` skills.
2. Check which modules and functions are available.
3. Write or update the Lua script using only those modules.
4. Save the script through the Lua authoring capability flow.
