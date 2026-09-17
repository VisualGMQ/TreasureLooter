local ScriptBehavior = require("common.script_behavior")
local ServerWorld = require("server.world")

-- The damage dealt by a spike every time a player touches it. The hp component
-- ignores it while invincible, so standing on the spike hurts at the character
-- definition's invincible rate.
local k_spike_damage = 1

---@class ServerSpikeBehavior : ScriptBehavior
---@field private _trigger Trigger|nil
--- Whether the spike is still part of a live scene. Cleared by `OnQuit` so a
--- queued touch callback cannot use the removed trigger.
---@field private _active boolean
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ScriptBehavior })

---@param entity LogicEntity
---@return ServerSpikeBehavior
function _M.new(entity)
    local self = setmetatable(ScriptBehavior.new(entity), _M)
    ---@cast self ServerSpikeBehavior
    self._trigger = nil
    self._active = false
    return self
end

function _M:OnInit()
    local ctx = TL_Common.GetContext()
    local trigger = ctx:GetTriggerComponentManager():Get(self:GetEntity())
    if not trigger then
        ctx:Log("spike behavior: entity ", self:GetEntity(), " has no trigger component")
        return
    end

    self._active = true
    self._trigger = trigger
    -- The prefab sets trig_every_frame_when_touch, so the listener is called
    -- as long as the player overlaps the spike.
    trigger:SetTouchListener(function(event)
        self:onTouch(event)
    end)
end

function _M:OnQuit()
    self._active = false
    self._trigger = nil
end

--- Only the server hurts players: it is the authority and replicates the new
--- hp to every client.
---@private
---@param event TriggerTouchEvent
function _M:onTouch(event)
    -- The spike (or the whole scene) may be gone while a touch callback is
    -- still queued.
    if not self._active then
        return
    end

    -- Spikes only hurt while the match is live: during the lobby, the pre-game
    -- freeze and the round teardown no player takes damage (and no message is
    -- sent to peers that are being disconnected).
    local world = ServerWorld.GetInst()
    ---@cast world ServerWorld
    if not world:CanAcceptPlayerInput() then
        return
    end

    local ctx = TL_Common.GetContext()

    local result = event:GetOverlapResult()
    local script = ctx:GetScriptManager():Get(result.m_dst_entity)
    if not script then
        return
    end

    local gameobject = script.m_gameobject
    if not gameobject or not gameobject.m_hp_component then
        return
    end

    -- Only replicated players (net_id ~= 0) are hurt: monsters and other
    -- scene entities are not part of the match.
    local net_id = gameobject:GetNetID()
    if net_id == 0 then
        return
    end

    local hp_component = gameobject.m_hp_component
    -- A player that is already dead (or already left the round) must not be
    -- hurt again: the death was already replicated.
    if hp_component:IsDead() then
        return
    end

    local before_hp = hp_component:GetHp()
    hp_component:Hurt(k_spike_damage)
    local after_hp = hp_component:GetHp()
    if after_hp == before_hp then
        -- The hp component is invincible right now, nothing to replicate.
        return
    end

    ctx:Log("spike hurt player net_id ", net_id, " hp ", after_hp)

    local hurt = TL_Proto.Hurt()
    hurt:set_m_net_id(net_id)
    hurt:set_m_hp(after_hp)

    local net_msg = TL_Proto.NetMsg()
    net_msg:set_m_hurt(hurt)

    world:broadcastToPlayers(net_msg)

    if hp_component:IsDead() then
        world:MarkDead(net_id)
    end
end

return _M
