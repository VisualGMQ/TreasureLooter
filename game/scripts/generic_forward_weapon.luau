local GenericForwardWeapon = {}
GenericForwardWeapon.__index = GenericForwardWeapon

type GenericForwardWeaponData = {
	m_entity: number,
    m_transform: userdata?,
    m_anim_player: userdata?,
    m_dir : userdata,
}

export type GenericForwardWeapon = typeof(setmetatable({}::GenericForwardWeaponData, GenericForwardWeapon))

function GenericForwardWeapon.new(entity: number): GenericForwardWeapon
	local self  = {}::GenericForwardWeaponData

	self.m_entity = entity
    self.m_anim_player = nil
    self.m_transform = nil
    self.m_dir = TL.Vec2.ZERO

	return setmetatable(self, GenericForwardWeapon)
end

function GenericForwardWeapon.OnInit(self: GenericForwardWeapon)
    local ctx = TL.GetContext()
    self.m_anim_player = ctx:GetAnimationPlayerManager():Get(self.m_entity)
    self.m_transform = ctx:GetTransformManager():Get(self.m_entity)
    self.m_dir = TL.Vec2.X_UNIT
end

function GenericForwardWeapon.ChangeDir(self: GenericForwardWeapon, dir: userdata)
    self.m_dir = dir
    self.m_transform.m_rotation = TL.Degrees(TL.GetAngle(self.m_dir, TL.Vec2.X_UNIT))
end

function GenericForwardWeapon.Attack(self: GenericForwardWeapon)
    if self.m_anim_player ~= nil and not self.m_anim_player:IsPlaying() then
        self.m_anim_player:Stop()
        self.m_anim_player:Play()
    end

    if self.m_transform ~= nil then
        local angle = TL.GetAngle(self.m_dir, TL.Vec2.X_UNIT)
        self.m_transform.m_rotation = TL.Degrees(angle);
    end
end

function GenericForwardWeapon.OnUpdate(self: GenericForwardWeapon, elapse_time: number)
end

function GenericForwardWeapon.OnQuit(self: GenericForwardWeapon)
end

function GenericForwardWeapon.OnRender(self: GenericForwardWeapon)
end

return GenericForwardWeapon