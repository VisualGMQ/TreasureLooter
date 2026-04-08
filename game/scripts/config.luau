local TL = require("@game/tl_types")

local _M = {}

export type CharacterConfig = {
    m_entity: TL.Entity,
    m_weapon_entity: TL.Entity,
    m_move_left_anim : TL.AnimationHandle?,
    m_move_right_anim : TL.AnimationHandle?,
    m_move_up_anim : TL.AnimationHandle?,
    m_move_down_anim : TL.AnimationHandle?,
    m_move_speed : number,
}

export type WeaponConfig = {
    m_type : TL.WeaponType,
    m_image : TL.ImageHandle,
    m_attack_speed : number,
    m_cooldown: number,
    -- m_attack_regions: { TL.Circle },
}

return _M