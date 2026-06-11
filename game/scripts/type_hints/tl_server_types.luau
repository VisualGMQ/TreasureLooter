--!nolua
export type ServerContext = CommonContext & {
	NetListen: (self: ServerContext, address: NetAddress, peer_count: number) -> (),
}

export type GetContext = () -> ServerContext

return TL_Server
