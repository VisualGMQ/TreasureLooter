local TL = require("@game/tl_types")
local ScriptBehavior = require("@game/script_behavior")
local Creation = require("@game/creation")
local DebugPanel = require("@game/debug_panel")
local CharacterBehavior = require("@game/character_behavior")

local GameEntry = {}
GameEntry.__index = GameEntry
setmetatable(GameEntry, { __index = ScriptBehavior })

type GameEntryData = { }

export type GameEntry = typeof(setmetatable({}::GameEntryData, GameEntry))

function GameEntry.new(entity: TL.Entity) : GameEntry
    local self = ScriptBehavior.new(entity)
    return setmetatable(self, GameEntry)
end

function GameEntry.OnInit(self: GameEntry)
    local ctx: TL.GameContext = TL.GetContext()
    local script_mgr = ctx:GetScriptManager()

    -- create player character
    local config = ctx:GetAssetsManager():GetCharacterDefinitionManager():Load("assets/gpa/monsters/waggo/waggo.character_def.xml", false)
    local transform: TL.Transform = TL.Transform()
    transform.m_position.x = 300
    transform.m_position.y = 350
    local character = Creation.CreateCharacter(transform, config)

    if character == nil then
        TL.Log("failed to create player, entity is null")
        return
    end
    local player_script = script_mgr:Get(character.m_entity)
    if player_script then
        (player_script :: CharacterBehavior.CharacterBehavior):SetTeamID(TL.TeamID.team1)
    end

    -- create slime monster
    local slime_config = ctx:GetAssetsManager():GetCharacterDefinitionManager():Load("assets/gpa/monsters/slime2/slime2.character_def.xml", false)
    local slime_transform: TL.Transform = TL.Transform()
    slime_transform.m_position.x = 300
    slime_transform.m_position.y = 100
    local slime_character = Creation.CreateCharacter(slime_transform, slime_config)

    if slime_character == nil then
        TL.Log("failed to create slime character, entity is null")
        return
    end

    local slime_script = script_mgr:Get(slime_character.m_entity)
    if slime_script then
        (slime_script :: CharacterBehavior.CharacterBehavior):SetTeamID(TL.TeamID.neutral);
        (slime_script :: CharacterBehavior.CharacterBehavior):SetHP(3)
    end

    -- put monsters into level
    local scene = ctx:GetSceneManager():GetCurrentScene()
    if scene == nil then
        TL.Log("current level is nil")
        return
    end

    local root_entity = scene:GetRootEntity()
    local relationship_mgr = ctx:GetRelationshipManager()

    local root_relationship = relationship_mgr:Get(root_entity)
    if root_relationship == nil then
        TL.Log("root relationship is nil")
        return
    end

    local arch_entity = root_relationship.m_children[2]

    TL.Log("arch_entity = ", arch_entity)

    local arch_layer_relationship = relationship_mgr:Get(arch_entity)
    if arch_layer_relationship == nil then
        relationship_mgr:RegisterEntity(arch_entity)
        arch_layer_relationship = relationship_mgr:Get(arch_entity)
    end

    local arch_children = arch_layer_relationship.m_children
    arch_children[#arch_children + 1] = character.m_entity
    arch_children[#arch_children + 1] = slime_character.m_entity
    arch_layer_relationship.m_children = arch_children
end

function GameEntry.OnRender(self: GameEntry)
    DebugPanel.ShowDebugPanel()
end

return GameEntry
