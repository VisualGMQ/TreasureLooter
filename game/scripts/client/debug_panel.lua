local ComponentDisplay = require("client.component_display")

---@class DebugPanel
local _M = {}

local debug_event_value = 0

local selected_entity = TL_Common.null_entity

---@param ctx ClientContext
---@param px number
---@param py number
---@param half number
---@param r number
---@param g number
---@param b number
---@param a number
local function context_debug_fill_rect_world(ctx, px, py, half, r, g, b, a)
    local dd = ctx:GetDebugDraw()
    if dd then
        dd:FillRect(TL_Common.Vec2(px, py), TL_Common.Vec2(half, half), TL_Common.Color(r, g, b, a), TL_Common.DebugDraw.kOneFrame)
    end
end

---@param entity LogicEntity
---@param display_name string
---@param relationship_mgr RelationshipManager
local function draw_entity_tree(entity, display_name, relationship_mgr)
    local relationship = relationship_mgr:Get(entity)
    if not relationship then
        return
    end

    ImGui.PushID(entity)

    local sel_mark = (selected_entity == entity) and " *" or ""
    local label = string.format("%s [%d]%s###hierarchy_%d", display_name, entity, sel_mark, entity)

    local flags = ImGui.TreeNodeFlags_OpenOnDoubleClick | ImGui.TreeNodeFlags_OpenOnArrow
    if relationship:GetChildrenCount() == 0 then
        flags = flags | ImGui.TreeNodeFlags_Leaf
    end
    if selected_entity == entity then
        flags = flags | ImGui.TreeNodeFlags_Selected
    end

    local open = ImGui.TreeNodeEx(label, flags)
    if ImGui.IsItemClicked() then
        selected_entity = entity
    end
    if open then
        for i = 0, relationship:GetChildrenCount() - 1 do
            local child = relationship:Get(i)
            draw_entity_tree(child, "LogicEntity", relationship_mgr)
        end
        ImGui.TreePop()
    end

    ImGui.PopID()
end

local COLLAPSE_FLAGS = ImGui.TreeNodeFlags_CollapsingHeader | ImGui.TreeNodeFlags_DefaultOpen

---@param ctx ClientContext
local function draw_hierarchy_section(ctx)
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

---@param ctx ClientContext
local function draw_inspector_section(ctx)
    if ImGui.TreeNodeEx("Inspector###debug_panel_inspector", COLLAPSE_FLAGS) then
        if selected_entity == TL_Common.null_entity then
            ImGui.Text("Select an entity in Hierarchy.")
        else
            ComponentDisplay.DisplayInspector(ctx, selected_entity)
        end
    end
end

---@param ctx ClientContext
local function draw_selected_entity_marker(ctx)
    if selected_entity == TL_Common.null_entity then
        return
    end
    local e = selected_entity

    local px, py = nil, nil
    local transform_mgr = ctx:GetTransformManager()
    if transform_mgr:Has(e) then
        local w = ComponentDisplay.TransformWorldPosition(transform_mgr:Get(e))
        px, py = w.x, w.y
    else
        local cct_mgr = ctx:GetCCTManager()
        if cct_mgr:Has(e) then
            local w = cct_mgr:Get(e):GetPosition()
            px, py = w.x, w.y
        end
    end

    if px ~= nil and py ~= nil then
        context_debug_fill_rect_world(ctx, px, py, 2, 0.15, 0.45, 1, 0.95)
    end
end

function _M.ShowDebugPanel()
    local ctx = TL_Client.GetContext()
    local event_debugger = ctx:GetEventDebugger()
    if not ImGui.Begin("Debug panel") then
        ImGui.End()
        return
    end

    local time = ctx:GetTime()
    ImGui.Text("FPS: " .. tostring(time:GetFPS()) .. "  (unlimit: " .. tostring(time:GetUnlimitFPS()) .. ")")
    ImGui.Separator()

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
