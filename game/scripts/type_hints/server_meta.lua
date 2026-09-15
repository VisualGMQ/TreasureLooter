---@meta
-- LuaLS (LuaCATS) definition file for the `TL_Server` C++ binding namespace.
-- Hand-written equivalent of server/src/script_binding.cpp.
-- Types declared in common_meta.lua / schema_meta.lua are referenced as globals.

---@class ServerContext : CommonContext
---@field NetListen fun(self: ServerContext, addr: NetAddress, peer_count: number)
---@field GetConfig fun(self: ServerContext): ServerConfig

-- The runtime global table created by `beginNamespace("TL_Server")`.
---@class TL_Server
---@field GetContext fun(): ServerContext
TL_Server = {}
