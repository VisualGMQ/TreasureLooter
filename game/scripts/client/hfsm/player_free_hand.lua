local ClientAttackComponent = require("client.components.attack")
local ClientGameObject = require("client.gameobject")
local ClientWorld = require("client.world")
local ClientCreation = require("client.creation")
local HFSMUtil = require("client.hfsm.hfsm_util")
local States = require("client.hfsm.player_states")

local k_fireball_speed = 150
local k_skill_cooldown = 1
local k_fireball_distance = 25
local k_other_skill_distance = 30

---@class PlayerFreeHandNode
---@field _entity Entity
---@field _skill_cooldown_elapsed number
---@field _skill2_cooldown_elapsed number
---@field _skill3_cooldown_elapsed number
local _M = {}
_M.__index = _M

---@param entity Entity
---@return PlayerFreeHandNode
function _M.new(entity)
    local self = setmetatable({}, _M)
    self._entity = entity
    self._skill_cooldown_elapsed = 0
    self._skill2_cooldown_elapsed = 0
    self._skill3_cooldown_elapsed = 0
    return self
end

--- The direction the player last moved towards, falling back to the default
--- facing when the player has not moved yet.
---@param go ClientGameObject
---@return Vec2
local function get_aim_dir(go)
    local dir = ClientAttackComponent.k_init_forward_dir
    local move_component = go.m_move_component
    if move_component then
        dir = move_component:GetLastMoveDirection()
        if dir:LengthSquared() == 0 then
            dir = ClientAttackComponent.k_init_forward_dir
        end
    end
    return dir
end

--- Spawn a skill entity at `dir * distance` from `go` and parent it to the land.
---@param go ClientGameObject
---@param did DID
---@param dir Vec2
---@param distance number
---@return any script
local function spawn_skill(go, did, dir, distance)
    local ctx = TL_Client.GetContext()
    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        return nil
    end

    local world = ClientWorld.GetInst()
    local spawn_info = TL_Schema.ObjectSpawnDefinition()
    spawn_info.m_did = did
    local position = go.m_transform:GetGlobalPosition() + dir * distance
    local entity = ClientCreation:CreateSkill(scene, spawn_info, position, world.m_object_definitions)

    local land_relationship = ctx:GetRelationshipManager():Get(world.m_land_entity)
    if land_relationship then
        local skill_relationship = ctx:GetRelationshipManager():Get(entity)
        if skill_relationship then
            skill_relationship:RemoveFromParent()
        end
        land_relationship:AddChild(entity)
    end

    return ctx:GetScriptManager():Get(entity)
end

function _M:OnUpdate()
    local ctx = TL_Client.GetContext()
    local elapse_time = HFSMUtil.GetElapseTime()
    local go = HFSMUtil.GetGameObject(self._entity)
    if not go then
        return
    end
    ---@cast go ClientGameObject
    local input_manager = ctx:GetInputManager()

    self._skill_cooldown_elapsed = math.max(0, self._skill_cooldown_elapsed - elapse_time)
    self._skill2_cooldown_elapsed = math.max(0, self._skill2_cooldown_elapsed - elapse_time)
    self._skill3_cooldown_elapsed = math.max(0, self._skill3_cooldown_elapsed - elapse_time)

    local action = input_manager:GetAction("Attack")
    if action:IsPressed(0) and self._skill_cooldown_elapsed <= 0 then
        local dir = get_aim_dir(go)
        local fireball_script = spawn_skill(go, TL_Schema.DID.SkillDID1, dir, k_fireball_distance)
        if fireball_script then
            fireball_script:SetFireDir(dir)
            fireball_script:SetFireSpeed(k_fireball_speed)
            fireball_script:Start(go:GetEntity())
            self._skill_cooldown_elapsed = k_skill_cooldown
        end
    end

    local action2 = input_manager:GetAction("Attack2")
    if action2:IsPressed(0) and self._skill2_cooldown_elapsed <= 0 then
        local dir2 = get_aim_dir(go)
        local rock_script = spawn_skill(go, TL_Schema.DID.SkillDID2, dir2, k_other_skill_distance)
        if rock_script then
            rock_script:Start(go:GetEntity())
            self._skill2_cooldown_elapsed = k_skill_cooldown
        end
    end

    local action3 = input_manager:GetAction("Attack3")
    if action3:IsPressed(0) and self._skill3_cooldown_elapsed <= 0 then
        local dir3 = get_aim_dir(go)
        local ice_script = spawn_skill(go, TL_Schema.DID.SkillDID3, dir3, k_other_skill_distance)
        if ice_script then
            ice_script:Start(go:GetEntity())
            self._skill3_cooldown_elapsed = k_skill_cooldown
        end
    end

    if go.m_interact_component then
        local dig_action = input_manager:GetAction("Dig")
        if dig_action:IsPressed(0) then
            local obj = go.m_interact_component:GetInteractObject()
            if obj then
                ---@cast obj ClientGameObject
                if obj.m_item_component and obj.m_item_component:GetBehavior():Has(TL_Schema.ItemBehavior.Diggable) then
                    if obj.m_spawn_object_component then
                        obj.m_spawn_object_component:Spawn(obj.m_transform:GetGlobalPosition())
                    end
                end
                if obj.m_item_component and obj.m_item_component:GetBehavior():Has(TL_Schema.ItemBehavior.Choppable) then
                    if obj.m_constructable_component and obj.m_constructable_component:IsAtLastPhase() then
                        if obj.m_spawn_object_component then
                            obj.m_spawn_object_component:Spawn(obj.m_transform:GetGlobalPosition())
                            obj.m_constructable_component:ResetToPhase(1)
                        end
                    end
                end
            end
        end

        local interact_action = input_manager:GetAction("Interact")
        if interact_action:IsPressed(0) then
            local obj = go.m_interact_component:GetInteractObject()
            if obj then
                HFSMUtil.ChangeState(self._entity, States.PickingUp)
            end
        end
    end
end

function _M:OnQuit()
end

return _M
