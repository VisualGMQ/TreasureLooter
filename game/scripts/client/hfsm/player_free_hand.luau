local ClientGameObject = require("@client/gameobject")
local ClientAttackComponent = require("@client/components/attack")
local ClientWorld = require("@client/world")
local ClientCreation = require("@client/creation")
local HFSMUtil = require("@client/hfsm/hfsm_util")
local States = require("@client/hfsm/player_states")

local k_fireball_speed = 150

local _M = {}
_M.__index = _M

export type PlayerFreeHandNode = typeof(_M) & {
    _entity: Entity,
    _skill_cooldown_elapsed: number,
    _skill2_cooldown_elapsed: number,
    _skill3_cooldown_elapsed: number,
}

function _M.new(entity: Entity): PlayerFreeHandNode
    local self = {} :: PlayerFreeHandNode
    self._entity = entity
    self._skill_cooldown_elapsed = 0
    self._skill2_cooldown_elapsed = 0
    self._skill3_cooldown_elapsed = 0
    return setmetatable(self, _M)
end

local function get_aim_dir(go: ClientGameObject.ClientGameObject): Vec2
    local dir = go.m_move_component:GetLastMoveDirection()
    if dir:LengthSquared() == 0 then
        dir = ClientAttackComponent.k_init_forward_dir
    end
    return dir
end

function _M.OnUpdate(self: PlayerFreeHandNode)
    local ctx = TL_Client.GetContext()
    local elapse_time = HFSMUtil.GetElapseTime()
    local go = HFSMUtil.GetGameObject(self._entity) :: ClientGameObject.ClientGameObject
    if not go then
        return
    end
    local input_manager = ctx:GetInputManager()

    self._skill_cooldown_elapsed = math.max(0, self._skill_cooldown_elapsed - elapse_time)
    self._skill2_cooldown_elapsed = math.max(0, self._skill2_cooldown_elapsed - elapse_time)
    self._skill3_cooldown_elapsed = math.max(0, self._skill3_cooldown_elapsed - elapse_time)

    local action = input_manager:GetAction("Attack")
    if action:IsPressed(0) and self._skill_cooldown_elapsed <= 0 then
        local dir = get_aim_dir(go)
        local spawn_info = TL_Schema.ObjectSpawnDefinition()
        spawn_info.m_did = TL_Schema.DID.SkillDID1
        local world = ClientWorld.GetInst()
        local fireball_pos = go.m_transform:GetGlobalPosition() + dir * 25
        local fireball_entity = ClientCreation.CreateSkill(ClientCreation, ctx:GetSceneManager():GetCurrentScene(), spawn_info, fireball_pos, world.m_object_definitions)

        local land_relationship = ctx:GetRelationshipManager():Get(world.m_land_entity)
        if land_relationship then
            local fireball_relationship = ctx:GetRelationshipManager():Get(fireball_entity)
            if fireball_relationship then
                fireball_relationship:RemoveFromParent()
            end
            land_relationship:AddChild(fireball_entity)
        end

        local fireball_script = ctx:GetScriptManager():Get(fireball_entity)
        if fireball_script then
            fireball_script:SetFireDir(dir)
            fireball_script:SetFireSpeed(k_fireball_speed)
            fireball_script:Start(go:GetEntity())
            self._skill_cooldown_elapsed = 1
        end
    end

    local action2 = input_manager:GetAction("Attack2")
    if action2:IsPressed(0) and self._skill2_cooldown_elapsed <= 0 then
        local dir2 = get_aim_dir(go)
        local spawn_info2 = TL_Schema.ObjectSpawnDefinition()
        spawn_info2.m_did = TL_Schema.DID.SkillDID2
        local world2 = ClientWorld.GetInst()
        local rock_pos = go.m_transform:GetGlobalPosition() + dir2 * 30
        local rock_entity = ClientCreation.CreateSkill(ClientCreation, ctx:GetSceneManager():GetCurrentScene(), spawn_info2, rock_pos, world2.m_object_definitions)

        local land_relationship2 = ctx:GetRelationshipManager():Get(world2.m_land_entity)
        if land_relationship2 then
            local rock_relationship = ctx:GetRelationshipManager():Get(rock_entity)
            if rock_relationship then
                rock_relationship:RemoveFromParent()
            end
            land_relationship2:AddChild(rock_entity)
        end

        local rock_script = ctx:GetScriptManager():Get(rock_entity)
        if rock_script then
            rock_script:Start(go:GetEntity())
        end

        self._skill2_cooldown_elapsed = 1
    end

    local action3 = input_manager:GetAction("Attack3")
    if action3:IsPressed(0) and self._skill3_cooldown_elapsed <= 0 then
        local dir3 = get_aim_dir(go)
        local spawn_info3 = TL_Schema.ObjectSpawnDefinition()
        spawn_info3.m_did = TL_Schema.DID.SkillDID3
        local world3 = ClientWorld.GetInst()
        local ice_pos = go.m_transform:GetGlobalPosition() + dir3 * 30
        local ice_entity = ClientCreation.CreateSkill(ClientCreation, ctx:GetSceneManager():GetCurrentScene(), spawn_info3, ice_pos, world3.m_object_definitions)

        local land_relationship3 = ctx:GetRelationshipManager():Get(world3.m_land_entity)
        if land_relationship3 then
            local ice_relationship = ctx:GetRelationshipManager():Get(ice_entity)
            if ice_relationship then
                ice_relationship:RemoveFromParent()
            end
            land_relationship3:AddChild(ice_entity)
        end

        local ice_script = ctx:GetScriptManager():Get(ice_entity)
        if ice_script then
            ice_script:Start(go:GetEntity())
        end

        self._skill3_cooldown_elapsed = 1
    end

    if go.m_interact_component then
        local dig_action = input_manager:GetAction("Dig")
        if dig_action:IsPressed(0) then
            local obj = go.m_interact_component:GetInteractObject()
            if obj then
                local obj_go = obj :: ClientGameObject.ClientGameObject
                if obj_go.m_item_component and obj_go.m_item_component:GetBehavior():Has(TL_Schema.ItemBehavior.Diggable) then
                    if obj_go.m_spawn_object_component then
                        local mine_pos = obj_go.m_transform:GetGlobalPosition()
                        obj_go.m_spawn_object_component:Spawn(mine_pos)
                    end
                end
                if obj_go.m_item_component and obj_go.m_item_component:GetBehavior():Has(TL_Schema.ItemBehavior.Choppable) then
                    if obj_go.m_constructable_component and obj_go.m_constructable_component:IsAtLastPhase() then
                        if obj_go.m_spawn_object_component then
                            local tree_pos = obj_go.m_transform:GetGlobalPosition()
                            obj_go.m_spawn_object_component:Spawn(tree_pos)
                            obj_go.m_constructable_component:ResetToPhase(1)
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

function _M.OnQuit(self: PlayerFreeHandNode)
end

return _M
