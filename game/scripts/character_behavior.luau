local TL = require("@game/tl_types")
local Config = require("@game/config")
local ScriptBehavior = require("@game/script_behavior")

local CharacterBehavior = {}
CharacterBehavior.__index = CharacterBehavior
setmetatable(CharacterBehavior, { __index = ScriptBehavior })

type CharacterBehaviorData = {
    __config: Config.CharacterConfig,
    __weapon_entity: TL.Entity,
    m_team_id: TL.TeamID,
    m_hp: number,
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
    self.__config = nil
    self.__weapon_entity = TL.null_entity
    self.m_team_id = TL.TeamID.Team_invalid
    self.m_hp = 1
    return setmetatable(self, CharacterBehavior)
end

function CharacterBehavior.__attach_config(self: CharacterBehavior, config: Config.CharacterConfig, weapon_entity: TL.Entity)
    self.__config = config
    self.__weapon_entity = weapon_entity
end

function CharacterBehavior.SetTeamID(self: CharacterBehavior, team_id: TL.TeamID)
    self.m_team_id = team_id
end

function CharacterBehavior.GetTeamID(self: CharacterBehavior): TL.TeamID
    return self.m_team_id
end

function CharacterBehavior.GetCharacterConfig(self: CharacterBehavior): Config.CharacterConfig
    return self.__config
end

function CharacterBehavior.GetWeaponEntity(self: CharacterBehavior): TL.Entity
    return self.__weapon_entity
end

function CharacterBehavior.GetHP(self: CharacterBehavior): number
    return self.m_hp
end

function CharacterBehavior.SetHP(self: CharacterBehavior, hp: number)
    self.m_hp = hp
end

return CharacterBehavior
