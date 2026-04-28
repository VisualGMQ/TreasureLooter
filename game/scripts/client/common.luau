local _M = {}

_M.Direction = {
    Left = 0,
    Right = 1,
    Down = 2,
    Up = 3,
}

export type Direction = typeof(_M.Direction)

return _M

