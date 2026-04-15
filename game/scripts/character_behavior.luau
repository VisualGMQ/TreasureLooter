local TL = require("@game/tl_types")
local ScriptBehavior = require("@game/script_behavior")
local WeaponBehavior = require("@game/weapon_behavior")

local CharacterBehavior = {}
CharacterBehavior.__index = CharacterBehavior
setmetatable(CharacterBehavior, { __index = ScriptBehavior })

type CharacterBehaviorData = {
    m_weapon : WeaponBehavior.WeaponBehavior,

    -- from definition
    m_hp: number,
    m_move_left_anim : TL.AnimationHandle?,
    m_move_right_anim : TL.AnimationHandle?,
    m_move_up_anim : TL.AnimationHandle?,
    m_move_down_anim : TL.AnimationHandle?,
    m_move_speed : number,
    m_sprite_sheet : TL.ImageHandle,

    -- from runtime
    m_team_id: TL.TeamID,
}

export type CharacterBehavior = typeof(setmetatable({} :: CharacterBehaviorData, CharacterBehavior)) & ScriptBehavior.ScriptBehavior

CharacterBehavior.Direction = {
    Left = 0,
    Right = 1,
    Down = 2,
    Up = 3,
}

export type Direction = typeof(CharacterBehavior.Direction)

function CharacterBehavior.new(entity: number): CharacterBehavior
    local self = ScriptBehavior.new(entity)
    self.m_weapon = nil
    self.m_team_id = TL.TeamID.Team_invalid
    return setmetatable(self, CharacterBehavior)
end

function CharacterBehavior._initByDefinition(self: CharacterBehavior, definition: TL.CharacterDefinitionHandle, weapon: WeaponBehavior.WeaponBehavior)
    self.m_weapon = weapon

    self.m_hp = definition.m_hp
    self.m_move_left_anim = definition.m_move_left_animation
    self.m_move_right_anim = definition.m_move_right_animation
    self.m_move_up_anim = definition.m_move_up_animation
    self.m_move_down_anim = definition.m_move_down_animation
    self.m_move_speed = definition.m_speed
    self.m_sprite_sheet = definition.m_sprite_sheet
end

function CharacterBehavior.SetTeamID(self: CharacterBehavior, team_id: TL.TeamID)
    self.m_team_id = team_id
end

function CharacterBehavior.GetTeamID(self: CharacterBehavior): TL.TeamID
    return self.m_team_id
end

function CharacterBehavior.GetWeaponEntity(self: CharacterBehavior): TL.Entity
    return self._weapon_entity
end

function CharacterBehavior.GetWeaponControlNode(self: CharacterBehavior): TL.Entity
    return self._weapon_control_node
end

return CharacterBehavior
