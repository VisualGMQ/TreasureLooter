local TL = require("@game/tl_types")
local ImGui = require("@game/imgui_types")
local ComponentDisplay = require("@game/component_display")

local _M = {}

local debug_event_value: number = 0

local selected_entity: TL.Entity = TL.null_entity

local function context_debug_fill_rect_world(
	ctx: TL.GameContext,
	px: number,
	py: number,
	half: number,
	r: number,
	g: number,
	b: number,
	a: number
)
	local dd = ctx:GetDebugDraw()
	if dd then
		dd:FillRect(TL.Vec2(px, py), TL.Vec2(half, half), TL.Color(r, g, b, a), TL.DebugDraw.kOneFrame)
	end
end

local function draw_entity_tree(entity: TL.Entity, display_name: string, relationship_mgr: TL.RelationshipManager)
	ImGui.PushID(entity)

	local rel = relationship_mgr:Get(entity)
	local children: { TL.Entity } = {}
	if rel and rel.m_children then
		children = rel.m_children
	end

	local sel_mark = (selected_entity == entity) and " *" or ""
	local label = string.format("%s [%d]%s###hierarchy_%d", display_name, entity, sel_mark, entity)

	local flags = bit32.bor(ImGui.TreeNodeFlags_OpenOnDoubleClick, ImGui.TreeNodeFlags_OpenOnArrow)
	if #children == 0 then
		flags = bit32.bor(flags, ImGui.TreeNodeFlags_Leaf)
	end
	if selected_entity == entity then
		flags = bit32.bor(flags, ImGui.TreeNodeFlags_Selected)
	end

	local open = ImGui.TreeNodeEx(label, flags)
	if ImGui.IsItemClicked() then
		selected_entity = entity
	end
	if open then
		for _, child in ipairs(children) do
			draw_entity_tree(child, "Entity", relationship_mgr)
		end
		ImGui.TreePop()
	end

	ImGui.PopID()
end

local COLLAPSE_FLAGS = bit32.bor(
	ImGui.TreeNodeFlags_CollapsingHeader,
	ImGui.TreeNodeFlags_DefaultOpen
)

local function draw_hierarchy_section(ctx: TL.GameContext)
	if ImGui.TreeNodeEx("Hierarchy###debug_panel_hierarchy", COLLAPSE_FLAGS) then
		local scene = ctx:GetSceneManager():GetCurrentScene()
		if scene == nil then
			ImGui.Text("(no active level)")
		else
			local root = scene:GetRootEntity()
			local relationship_mgr = ctx:GetRelationshipManager()
			draw_entity_tree(root, "Root", relationship_mgr)
		end
	end
end

local function draw_inspector_section(ctx: TL.GameContext)
	if ImGui.TreeNodeEx("Inspector###debug_panel_inspector", COLLAPSE_FLAGS) then
		if selected_entity == TL.null_entity then
			ImGui.Text("Select an entity in Hierarchy.")
		else
			ComponentDisplay.DisplayInspector(ctx, selected_entity)
		end
	end
end

local function draw_selected_entity_marker(ctx: TL.GameContext)
	if selected_entity == TL.null_entity then
		return
	end
	local e = selected_entity

	local px: number?, py: number? = nil, nil
	local transform_mgr = ctx:GetTransformManager()
	if transform_mgr:Has(e) then
		local w = ComponentDisplay.TransformWorldPosition(transform_mgr:Get(e) :: TL.Transform)
		px, py = w.x, w.y
	else
		local cct_mgr = ctx:GetCCTManager()
		if cct_mgr:Has(e) then
			local w = (cct_mgr:Get(e) :: TL.CharacterController):GetPosition()
			px, py = w.x, w.y
		end
	end

	if px ~= nil and py ~= nil then
		context_debug_fill_rect_world(ctx, px, py, 2, 0.15, 0.45, 1, 0.95)
	end
end

function _M.ShowDebugPanel()
	local ctx: TL.GameContext = TL.GetContext()
	local event_debugger: TL.EventDebugger = ctx:GetEventDebugger()
	if not ImGui.Begin("Debug panel") then
		ImGui.End()
		return
	end

	local physics = ctx:GetPhysicsScene()
	local debug_draw = physics:IsEnableDebugDraw()
	local _, changed = ImGui.Checkbox("Physics scene debug draw", debug_draw)
	if changed then
		physics:ToggleDebugDraw()
	end

	ImGui.SeparatorText("Event debugger")

	ImGui.BeginChild("##event_debug_child", 0, 140, ImGui.ChildFlags_Borders)
	do
		local new_val, _ = ImGui.DragInt("value", debug_event_value, 1, -1000000, 1000000)
		debug_event_value = new_val

		ImGui.SameLine()
		if ImGui.Button("Send DebugEvent") then
			event_debugger:SendDebugEvent(math.floor(debug_event_value))
		end

		ImGui.Spacing()
		local count = event_debugger:GetTriggeredCount()
		ImGui.Text("DebugEvent triggered count: " .. tostring(count))
	end
	ImGui.EndChild()

	draw_hierarchy_section(ctx)
	draw_inspector_section(ctx)

	draw_selected_entity_marker(ctx)

	ImGui.End()
end

return _M
