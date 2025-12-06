local TL = require("@game/tl_types")
local ImGui = require("@game/imgui_types")

local _M = {}

-- DragInt 状态需跨帧保留
local debug_event_value: number = 0

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

	ImGui.End()
end

return _M
