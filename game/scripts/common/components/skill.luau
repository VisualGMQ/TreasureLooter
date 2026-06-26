local Component = require("@common/components/component")

local _M = {}
_M.__index = _M
setmetatable(_M, Component)

-- Matches AnimationPlayer::InfLoop: an animation that loops forever.
local INF_LOOP = -1

type SkillCollision = {
	_shape_begin: number,
	_shape_end: number,
	_collision_mask: number,
	_event_type: number,
	_trig_every_frame: boolean,
	_begin_time: number,
	_end_time: number,
}

type SkillPhaseData = {
	_delay: number,
	_animation: AnimationHandle,
	_collision: SkillCollision?,
	_loop: number,
	_is_aoe: boolean,
	_damage: number,
	_duration: number,
}

type SkillComponentData = {
	_trigger: Trigger?,
	_all_shapes: { PhysicsShape },
	_phases: { SkillPhaseData },
	_current_phase: number,
	_elapsed: number,
	_is_casting: boolean,
	_on_hit_callback: (any, any) -> (),
	_hit_entities: { [Entity]: boolean },
	_caster: Entity,
}

export type SkillComponent = typeof(_M) & SkillComponentData & Component.Component

function _M.new(gameobject, skill_definition: SkillDefinitionHandle): SkillComponent
	local self = Component.new(gameobject) :: SkillComponent
	self._phases = {}
	self._current_phase = -1
	self._elapsed = 0
	self._is_casting = false
	self._on_hit_callback = nil
	self._hit_entities = {}
	self._caster = TL_Common.null_entity

	local entity = gameobject:GetEntity()
	local ctx = TL_Common.GetContext()
	self._trigger = ctx:GetTriggerComponentManager():Get(entity)
	if self._trigger then
		self._trigger:Disable()
		self._all_shapes = self._trigger:GetUnderlyingShapes()
	end

	local shape_index = 0

	if skill_definition and skill_definition:IsValid() then
		for _, phase in ipairs(skill_definition.m_phases) do
			local anim_finish = 0
			if phase.m_animation then
				anim_finish = phase.m_animation:GetFinishTime()
			end

			local collision_data: SkillCollision? = nil
			if phase.m_collision and phase.m_collision.m_collision then
				local colldef = phase.m_collision.m_collision
				local shape_count = #colldef.m_physics_shapes

				local mask_value = 0
				if self._trigger then
					local first_shape = self._all_shapes[shape_index + 1]
					if first_shape then
						mask_value = first_shape:GetCollisionMask():GetUnderlying()
						local empty_mask = TL_Common.CollisionGroup()
						for i = shape_index + 1, shape_index + shape_count do
							self._all_shapes[i]:SetCollisionMask(empty_mask)
						end
					end
				end

				-- An endless-loop animation doesn't have a natural finish time;
				-- keep the collision window open forever unless the user
				-- explicitly configured an end_time.
				local end_time = phase.m_collision.m_end_time
				if not end_time then
					end_time = phase.m_loop == INF_LOOP and math.huge or anim_finish
				end

				collision_data = {
					_shape_begin = shape_index,
					_shape_end = shape_index + shape_count,
					_collision_mask = mask_value,
					_event_type = colldef.m_event_type,
					_trig_every_frame = colldef.m_trig_every_frame_when_touch,
					_begin_time = phase.m_collision.m_begin_time or 0,
					_end_time = end_time,
				}
				shape_index = shape_index + shape_count
			end

			-- The moment (relative to the end of the phase delay) at which this
			-- phase finishes: the collision window end when present, otherwise
			-- the animation length. Endless-loop phases never end on a timer;
			-- they must be stopped externally (e.g. on a collision hit).
			local active_end
			if collision_data then
				active_end = collision_data._end_time
			elseif phase.m_loop == INF_LOOP then
				active_end = math.huge
			else
				active_end = anim_finish
			end

			table.insert(self._phases, {
				_delay = phase.m_delay,
				_animation = phase.m_animation,
				_collision = collision_data,
				_loop = phase.m_loop,
				_is_aoe = phase.m_is_aoe,
				_damage = phase.m_damage,
				_duration = phase.m_delay + active_end,
			})
		end
	end

	return setmetatable(self, _M)
end

function _M.SetHitCallback(self: SkillComponent, cb: (any, any) -> ())
	self._on_hit_callback = cb
end

function _M.GetCaster(self: SkillComponent): Entity
	return self._caster
end

function _M.Cast(self: SkillComponent, caster: Entity)
	if self._is_casting then return end
	if #self._phases == 0 then return end

	self._caster = caster
	self._is_casting = true
	self._elapsed = 0
	self._hit_entities = {}

	if self._trigger then
		self._trigger:Enable()
	end
	self:StepToNextPhase()
end

function _M.Stop(self: SkillComponent)
	self._current_phase = -1
	self._is_casting = false
	self._hit_entities = {}
	self._phases = {}
	self:disableAllShapes()
	if self._trigger then
		self._trigger:Disable()
	end
end

function _M.StepToNextPhase(self: SkillComponent, carry_over: number?)
	if self._current_phase >= 0 then
		local old_phase = self._phases[self._current_phase + 1]
		if old_phase and old_phase._collision then
			self:disablePhaseShapes(old_phase._collision)
		end
	end

	self._current_phase += 1
	self._hit_entities = {}
	-- Carry the time that overshot the previous phase into this one, so total
	-- skill timing stays frame-rate independent.
	self._elapsed = carry_over or 0

	if self._current_phase >= #self._phases then
		self:Stop()
		return
	end

	local new_phase = self._phases[self._current_phase + 1]
	if new_phase._collision then
		self:enablePhaseShapes(new_phase._collision)
	end
end

function _M.sweepCollision(self: SkillComponent, phase: SkillPhaseData)
	if not phase._collision then
		return
	end
	if not self._trigger then
		return
	end

	local go = self:GetGameObject()
	self._trigger:MoveTo(go.m_transform)
	self._trigger:Update()

	local shapes = self._trigger:GetTouchingShapes()

	local ctx = TL_Common.GetContext()
	for _, shape in ipairs(shapes) do
		local dst_entity = shape:GetOwner()
		if dst_entity == TL_Common.null_entity then
			continue
		end

		if dst_entity == self:GetCaster() then
			continue
		end

		local target = nil
		local script = ctx:GetScriptManager():Get(dst_entity)
		if script then
			target = script.m_gameobject
		end

		if self._on_hit_callback then
			self._on_hit_callback(go, target)
		end

		-- Apply buffs before dealing damage: taking damage starts the target's
		-- invincibility window, and buffs are skipped while the target is
		-- invincible (see BuffReceiveComponent.ApplyBuff).
		if go.m_buff_apply_component then
			go.m_buff_apply_component:ApplyTo(target)
		end
		if target and target.m_hp_component then
			target.m_hp_component:Hurt(phase._damage)
		end

		if not phase._is_aoe then
			break
		end
	end
end

function _M.enablePhaseShapes(self: SkillComponent, collision: SkillCollision)
	local mask = TL_Common.CollisionGroup()
	mask:SetUnderlying(collision._collision_mask)
	for i = collision._shape_begin + 1, collision._shape_end do
		self._all_shapes[i]:SetCollisionMask(mask)
	end
	self._trigger:SetEventType(collision._event_type)
	self._trigger:EnableTriggerEveryFrameWhenTouch(collision._trig_every_frame)
end

function _M.disablePhaseShapes(self: SkillComponent, collision: SkillCollision)
	local empty = TL_Common.CollisionGroup()
	for i = collision._shape_begin + 1, collision._shape_end do
		self._all_shapes[i]:SetCollisionMask(empty)
	end
end

function _M.disableAllShapes(self: SkillComponent)
	local empty = TL_Common.CollisionGroup()
	for _, shape in ipairs(self._all_shapes) do
		shape:SetCollisionMask(empty)
	end
end

function _M.GetCurPhase(self: SkillComponent): SkillPhase?
	return self._phases[self._current_phase + 1]
end

function _M.Update(self: SkillComponent, elapse_time: TimeType)
	if not self._is_casting then return end

	self._elapsed += elapse_time

	local phase = self:GetCurPhase()
	if not phase then
		self:Stop()
		return
	end

	-- collision window (measured from the end of the phase delay)
	if self._elapsed >= phase._delay then
		local phase_elapsed = self._elapsed - phase._delay
		local collision = phase._collision
		if collision then
			local can_collision = phase_elapsed >= collision._begin_time
				and phase_elapsed <= collision._end_time
			if can_collision then
				self:sweepCollision(phase)
			end
		end
	end

	-- advance to the next phase once this phase's duration elapsed, carrying the
	-- overshoot so the next phase (and its animation) starts already advanced.
	if self._elapsed >= phase._duration then
		local carry_over = self._elapsed - phase._duration
		self:StepToNextPhase(carry_over)
	end
end

return _M
