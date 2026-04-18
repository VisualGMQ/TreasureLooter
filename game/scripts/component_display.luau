local TL = require("@game/tl_types")
local ImGui = require("@game/imgui_types")

local _M = {}

local READ_F_SPEED = 0
local READ_F_LO = -1e9
local READ_F_HI = 1e9
local READ_I_SPEED = 0
local READ_I_LO = -2147483648
local READ_I_HI = 2147483647

local function readonly_drag_float(label: string, v: number?)
	local n = v
	if n == nil or n ~= n then
		n = 0
	end
	ImGui.BeginDisabled(true)
	ImGui.DragFloat(label, n, READ_F_SPEED, READ_F_LO, READ_F_HI)
	ImGui.EndDisabled()
end

local function readonly_drag_int(label: string, v: number?)
	local n = v
	if n == nil or n ~= n then
		n = 0
	end
	ImGui.BeginDisabled(true)
	ImGui.DragInt(label, math.floor(n + 0.5), READ_I_SPEED, READ_I_LO, READ_I_HI)
	ImGui.EndDisabled()
end

local function readonly_checkbox(label: string, v: boolean?)
	ImGui.BeginDisabled(true)
	ImGui.Checkbox(label, v == true)
	ImGui.EndDisabled()
end

-- Names must match `CollisionGroupType` in schema / TL bindings.
local COLLISION_GROUP_TYPE_NAMES: { string } =
	{ "CCT", "Obstacle", "Coin", "WeaponAttack", "Interactable" }

local function collision_group_active_names(g: TL.CollisionGroup): string
	local cgt = (TL :: any).CollisionGroupType
	if type(cgt) ~= "table" then
		return ""
	end
	local parts: { string } = {}
	for _, name in ipairs(COLLISION_GROUP_TYPE_NAMES) do
		local ty = (cgt :: any)[name]
		if ty ~= nil then
			local ok, has = pcall(function()
				return g:Has(ty)
			end)
			if ok and has then
				table.insert(parts, name)
			end
		end
	end
	return table.concat(parts, ", ")
end

function _M.DisplayPhysicsShapeReadonly(id_suffix: string, shape: TL.PhysicsShape)
	ImGui.PushID(id_suffix)
	readonly_drag_int("GetOwner (entity)", shape:GetOwner())
	readonly_drag_int("GetType", shape:GetType())
	readonly_drag_int("GetStorageType", shape:GetStorageType())
	local p = shape:GetPosition()
	readonly_drag_float("position.x", p.x)
	readonly_drag_float("position.y", p.y)
	local layer = shape:GetCollisionLayer()
	readonly_drag_int("collision_layer (underlying)", layer:GetUnderlying())
	ImGui.Text("collision_layer groups: " .. collision_group_active_names(layer))
	local mask = shape:GetCollisionMask()
	readonly_drag_int("collision_mask (underlying)", mask:GetUnderlying())
	ImGui.Text("collision_mask groups: " .. collision_group_active_names(mask))
	ImGui.PopID()
end

function _M.TransformWorldPosition(t: TL.Transform): TL.Vec2
	local ok, w = pcall(function()
		return (t :: any):GetGlobalPosition()
	end)
	if ok and w ~= nil then
		return w :: TL.Vec2
	end
	return t.m_position
end

local function bind_point_world_position(pt: any): TL.Vec2
	local ok, w = pcall(function()
		return pt:GetGlobalPosition()
	end)
	if ok and w ~= nil then
		return w :: TL.Vec2
	end
	return pt.m_position
end

local function script_manager_is_enable(script_mgr: TL.ScriptComponentManager, e: TL.Entity): boolean?
	local ok, v = pcall(function()
		return (script_mgr :: any):IsEnable(e)
	end)
	if ok then
		return v
	end
	return nil
end

local function context_get_draw_order_manager(ctx: TL.GameContext): any
	local ok, m = pcall(function()
		return (ctx :: any):GetDrawOrderManager()
	end)
	if ok then
		return m
	end
	return nil
end

function _M.DisplayVec2(name: string, v: TL.Vec2)
	ImGui.PushID(name)
	ImGui.Text(name)
	ImGui.SameLine()
	readonly_drag_float("x", v.x)
	ImGui.SameLine()
	readonly_drag_float("y", v.y)
	ImGui.PopID()
end

local function display_color_rgba(title: string, c: TL.Color)
	ImGui.PushID(title)
	ImGui.Text(title)
	readonly_drag_float("r", c.r)
	readonly_drag_float("g", c.g)
	readonly_drag_float("b", c.b)
	readonly_drag_float("a", c.a)
	ImGui.PopID()
end

function _M.DisplayTransform(t: TL.Transform)
	ImGui.PushID("Transform")
	_M.DisplayVec2("m_position (local)", t.m_position)
	local rot_deg = 0
	local ok_r, rv = pcall(function()
		return (t.m_rotation :: any):Value()
	end)
	if ok_r and type(rv) == "number" then
		rot_deg = rv
	end
	readonly_drag_float("m_rotation (deg)", rot_deg)
	_M.DisplayVec2("m_size", t.m_size)
	_M.DisplayVec2("m_scale", t.m_scale)
	local w = _M.TransformWorldPosition(t)
	_M.DisplayVec2("world (approx if no GetGlobalPosition bind)", w)
	ImGui.PopID()
end

function _M.DisplayRelationship(rel: TL.Relationship)
	ImGui.PushID("Relationship")
	if rel.m_children and #rel.m_children > 0 then
		local parts = {}
		for i, c in ipairs(rel.m_children) do
			parts[i] = tostring(c)
		end
		ImGui.Text("children: " .. table.concat(parts, ", "))
	else
		ImGui.Text("children: (none)")
	end
	ImGui.PopID()
end

function _M.DisplaySprite(s: TL.Sprite)
	ImGui.PushID("Sprite")
	local img = s.m_image
	if img ~= nil and img:IsValid() then
		ImGui.Text("m_image: " .. img:GetAssetPath())
	else
		ImGui.Text("m_image: (none / invalid)")
	end
	ImGui.Text("m_region.m_topleft")
	readonly_drag_float("x##reg_tl", s.m_region.m_topleft.x)
	readonly_drag_float("y##reg_tl", s.m_region.m_topleft.y)
	ImGui.Text("m_region.m_size")
	readonly_drag_float("x##reg_sz", s.m_region.m_size.x)
	readonly_drag_float("y##reg_sz", s.m_region.m_size.y)
	_M.DisplayVec2("m_size", s.m_size)
	_M.DisplayVec2("m_anchor", s.m_anchor)
	display_color_rgba("m_color", s.m_color)
	readonly_drag_int("m_flip", s.m_flip)
	ImGui.PopID()
end

function _M.DisplayDrawOrder(order: TL.DrawOrder)
	ImGui.PushID("DrawOrder")
	readonly_drag_int("m_z_order", order.m_z_order)
	readonly_checkbox("m_enable_y_sorting", order.m_enable_y_sorting)
	readonly_drag_float("GetGlobalOrder", order:GetGlobalOrder())
	ImGui.PopID()
end

function _M.DisplayAnimationPlayer(ap: TL.AnimationPlayer)
	ImGui.PushID("AnimationPlayer")
	readonly_checkbox("IsPlaying", ap:IsPlaying())
	readonly_checkbox("IsAutoPlayEnabled", ap:IsAutoPlayEnabled())
	readonly_drag_int("GetLoopCount", ap:GetLoopCount())
	readonly_drag_float("GetCurTime", ap:GetCurTime())
	readonly_drag_float("GetMaxTime", ap:GetMaxTime())
	readonly_drag_float("GetRate", ap:GetRate())
	readonly_checkbox("HasAnimation", ap:HasAnimation())
	ImGui.PopID()
end

local function inspect_script_value(label: string, v: any, depth: number)
	ImGui.PushID(tostring(depth) .. "\0" .. label)
	if depth > 10 then
		ImGui.Text(label .. ": …")
		ImGui.PopID()
		return
	end
	local vt = type(v)
	if vt == "table" then
		if ImGui.TreeNode(label) then
			for k, sub in pairs(v :: any) do
				inspect_script_value(tostring(k), sub, depth + 1)
			end
			ImGui.TreePop()
		end
	elseif vt == "number" then
		local n = v :: number
		if math.abs(n - math.floor(n + 0.5)) < 1e-6 then
			readonly_drag_int(label, n)
		else
			readonly_drag_float(label, n)
		end
	elseif vt == "userdata" then
		ImGui.Text(label .. ": [userdata]")
	else
		ImGui.Text(label .. " = " .. tostring(v))
	end
	ImGui.PopID()
end

function _M.DisplayScript(script_mgr: TL.ScriptComponentManager, e: TL.Entity)
	ImGui.PushID("Script")
	local script_en = script_manager_is_enable(script_mgr, e)
	if script_en ~= nil then
		readonly_checkbox("IsEnable", script_en)
	else
		ImGui.Text("IsEnable: (method not in this build)")
	end
	local tbl = script_mgr:Get(e)
	if tbl ~= nil then
		if ImGui.TreeNode("script_table") then
			for k, sub in pairs(tbl :: any) do
				inspect_script_value(tostring(k), sub, 0)
			end
			ImGui.TreePop()
		end
	else
		ImGui.Text("script_table: nil")
	end
	ImGui.PopID()
end

function _M.DisplayUIWidget(ui: TL.UIWidget)
	ImGui.PushID("UIWidget")
	readonly_checkbox("m_use_clip", ui.m_use_clip)
	readonly_checkbox("m_disabled", ui.m_disabled)
	readonly_checkbox("m_selected", ui.m_selected)
	readonly_checkbox("m_can_be_selected", ui.m_can_be_selected)
	_M.DisplayVec2("m_margin", ui.m_margin)
	_M.DisplayVec2("m_padding", ui.m_padding)
	ImGui.PopID()
end

function _M.DisplayTilemapComponent(tc: TL.TilemapComponent)
	ImGui.PushID("Tilemap")
	readonly_checkbox("has GetLayer ptr", tc:GetLayer() ~= nil)
	local col = tc:GetTilemapCollision()
	if col then
		ImGui.Text("collision m_topleft")
		readonly_drag_float("x##tc_col", col.m_topleft.x)
		readonly_drag_float("y##tc_col", col.m_topleft.y)
	else
		ImGui.Text("collision: nil")
	end
	ImGui.PopID()
end

function _M.DisplayTrigger(tr: TL.Trigger)
	ImGui.PushID("Trigger")
	ImGui.Text("GetEventType: " .. tostring(tr:GetEventType()))
	readonly_checkbox("IsTriggerEveryFrameWhenTouch", tr:IsTriggerEveryFrameWhenTouch())
	local ok, list = pcall(function()
		return tr:GetPhysicsData()
	end)
	if not ok then
		ImGui.Text("GetPhysicsData: [error]")
	elseif list == nil then
		ImGui.Text("GetPhysicsData: nil")
	else
		local n = #(list :: any)
		if n == 0 then
			ImGui.Text("GetPhysicsData: (empty)")
		else
			for i = 1, n do
				local pd = (list :: any)[i] :: TL.TriggerPhysicsData
				if ImGui.TreeNode("PhysicsData #" .. tostring(i)) then
					_M.DisplayVec2("m_local_position", pd.m_local_position)
					local sh = pd.m_shape
					if sh then
						if ImGui.TreeNode("m_shape") then
							_M.DisplayPhysicsShapeReadonly("tr_ph_" .. tostring(i), sh)
							ImGui.TreePop()
						end
					else
						ImGui.Text("m_shape: nil")
					end
					ImGui.TreePop()
				end
			end
		end
	end
	ImGui.PopID()
end

function _M.DisplayBindPoints(bind_mgr: TL.BindPointsComponentManager, e: TL.Entity)
	ImGui.PushID("BindPoints")
	local ok, err = pcall(function()
		local bp = bind_mgr:Get(e)
		if bp and bp.m_bind_points then
			if ImGui.TreeNode("m_bind_points") then
				for name, pt in pairs(bp.m_bind_points :: any) do
					if ImGui.TreeNode(tostring(name)) then
						ImGui.Text("m_name: " .. tostring(pt.m_name))
						readonly_drag_float("m_position.x", pt.m_position.x)
						readonly_drag_float("m_position.y", pt.m_position.y)
						local gw = bind_point_world_position(pt)
						readonly_drag_float("world.x (approx)", gw.x)
						readonly_drag_float("world.y (approx)", gw.y)
						ImGui.TreePop()
					end
				end
				ImGui.TreePop()
			end
		else
			ImGui.Text("(no bind point data)")
		end
	end)
	if not ok then
		ImGui.Text("BindPoints: [could not iterate] " .. tostring(err))
	end
	ImGui.PopID()
end

function _M.DisplayCharacterController(cct: TL.CharacterController)
	ImGui.PushID("CCT")
	readonly_drag_float("GetPosition.x", cct:GetPosition().x)
	readonly_drag_float("GetPosition.y", cct:GetPosition().y)
	readonly_drag_float("GetMinDisp", cct:GetMinDisp())
	local shape = cct:GetPhysicsShape()
	if shape then
		if ImGui.TreeNode("PhysicsShape (body)") then
			_M.DisplayPhysicsShapeReadonly("cct_shape", shape)
			ImGui.TreePop()
		end
	else
		ImGui.Text("GetPhysicsShape: nil")
	end
	ImGui.PopID()
end

--- 根据 ctx / entity 绘制各 Component（由 debug Inspector 调用）
function _M.DisplayInspector(ctx: TL.GameContext, e: TL.Entity)
	ImGui.Text(string.format("Entity: %d", e))

	local transform_mgr = ctx:GetTransformManager()
	if transform_mgr:Has(e) then
		ImGui.SeparatorText("Transform")
		_M.DisplayTransform(transform_mgr:Get(e) :: TL.Transform)
	end

	local relationship_mgr = ctx:GetRelationshipManager()
	if relationship_mgr:Has(e) then
		ImGui.SeparatorText("Relationship")
		_M.DisplayRelationship(relationship_mgr:Get(e) :: TL.Relationship)
	end

	local sprite_mgr = ctx:GetSpriteManager()
	if sprite_mgr:Has(e) then
		ImGui.SeparatorText("Sprite")
		_M.DisplaySprite(sprite_mgr:Get(e) :: TL.Sprite)
	end

	local draw_order_mgr = context_get_draw_order_manager(ctx)
	if draw_order_mgr ~= nil and draw_order_mgr:Has(e) then
		ImGui.SeparatorText("DrawOrder")
		_M.DisplayDrawOrder(draw_order_mgr:Get(e) :: TL.DrawOrder)
	end

	local anim_mgr = ctx:GetAnimationPlayerManager()
	if anim_mgr:Has(e) then
		ImGui.SeparatorText("AnimationPlayer")
		_M.DisplayAnimationPlayer(anim_mgr:Get(e) :: TL.AnimationPlayer)
	end

	local script_mgr = ctx:GetScriptManager()
	if script_mgr:Has(e) then
		ImGui.SeparatorText("Script")
		_M.DisplayScript(script_mgr, e)
	end

	local ui_mgr = ctx:GetUIManager()
	if ui_mgr:Has(e) then
		ImGui.SeparatorText("UIWidget")
		_M.DisplayUIWidget(ui_mgr:Get(e) :: TL.UIWidget)
	end

	local tile_mgr = ctx:GetTilemapComponentManager()
	if tile_mgr:Has(e) then
		ImGui.SeparatorText("TilemapComponent")
		_M.DisplayTilemapComponent(tile_mgr:Get(e) :: TL.TilemapComponent)
	end

	local trig_mgr = ctx:GetTriggerComponentManager()
	if trig_mgr:Has(e) then
		ImGui.SeparatorText("Trigger")
		_M.DisplayTrigger(trig_mgr:Get(e) :: TL.Trigger)
	end

	local bind_mgr = ctx:GetBindPointsComponentManager()
	if bind_mgr:Has(e) then
		ImGui.SeparatorText("BindPoints")
		_M.DisplayBindPoints(bind_mgr, e)
	end

	local cct_mgr = ctx:GetCCTManager()
	if cct_mgr:Has(e) then
		ImGui.SeparatorText("CharacterController")
		_M.DisplayCharacterController(cct_mgr:Get(e) :: TL.CharacterController)
	end
end

return _M
