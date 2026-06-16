--!strict
-- luau-lsp definition file for the `TL_Server` C++ binding namespace.
-- Generated equivalent of server/src/script_binding.cpp.
-- Types defined in common.d.luau / schema.d.luau are referenced as globals.

export type ServerContext = CommonContext & {
	NetListen: (self: ServerContext, addr: NetAddress, peer_count: number) -> (),
	GetConfig: (self: ServerContext) -> ServerConfig,
}

-- The runtime global table created by `beginNamespace("TL_Server")`.
declare TL_Server: {
	GetContext: () -> ServerContext,
}
