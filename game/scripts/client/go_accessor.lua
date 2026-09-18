--- Fast accessors for the mapping between a logic entity and the render-only
--- present entity (present entity = logic entity + PresentEntityOffset).
---@class ClientGameObjectAccessor
local _M = {}

---@param entity LogicEntity
---@return Transform?
function _M.GetPresentTransform(entity)
    local ctx = TL_Client.GetContext()
    local present_entity = ctx:GetPresentEntity(entity)
    local manager = ctx:GetPresentTransformManager()
    if not manager then
        return nil
    end
    return manager:Get(present_entity)
end

return _M
