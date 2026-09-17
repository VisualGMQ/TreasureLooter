local ClientWorld = require("client.world")

-- The input widget has no placeholder concept, its initial text is the hint,
-- so it is cleared as soon as the player presses it to type an address.
local k_ip_placeholder = "请输入服务器IP"

---@class GlobalNetTest
---@field private _connect_button LogicEntity
---@field private _ip_input LogicEntity
---@field private _pending_connect boolean
---@field private _listeners EventListenerID[]
local GlobalNetTest = {}
GlobalNetTest.__index = GlobalNetTest

---@param entity LogicEntity?
---@return GlobalNetTest
function GlobalNetTest.new(entity)
    local self = setmetatable({}, GlobalNetTest)
    self._connect_button = TL_Common.null_entity
    self._ip_input = TL_Common.null_entity
    self._pending_connect = false
    self._listeners = {}
    return self
end

-- `World.GetInst()` asserts when nothing initialized it yet, so existence is
-- checked through `FindInst()` before creating the shared client world.
local function ensureClientWorld()
    if ClientWorld.FindInst() then
        return
    end

    local world = ClientWorld.new()
    ClientWorld.SetInst(world)
    world:RegisterNetEventHandler()
end

---@param id EventListenerID
function GlobalNetTest:trackListener(id)
    table.insert(self._listeners, id)
end

--- The connect button was clicked: request the connection and hide the
--- connection widgets while the client tries to reach the server. The entity
--- still receives clicks while hidden, so the request is only honoured when
--- the button is actually drawn.
function GlobalNetTest:onConnectClicked()
    local widget = TL_Client.GetContext():GetUIManager():Get(self._connect_button)
    if not widget or not widget.m_enable_draw then
        return
    end

    -- A previous attempt may have left the "server full" notice on screen.
    local world = ClientWorld.FindInst()
    if world then
        ---@cast world ClientWorld
        world:SetUIEnableDraw("ServerFullText", false)
    end

    self._pending_connect = true
    self:setConnectUIEnableDraw(false)
end

--- The connection could not be established (timeout/refused) or the server
--- rejected/dropped this client: show the connection widgets again so the
--- player can retry without restarting the client.
---@param reason string
function GlobalNetTest:onConnectFailed(reason)
    local ctx = TL_Client.GetContext()
    ctx:Log("global net test: connect failed (", reason, ")")

    self._pending_connect = false
    self:setConnectUIEnableDraw(true)
end

--- Show or hide the IP input and the connection button. They overlap the lobby
--- counter and the "server full" notice at the bottom of the screen, so they
--- are hidden as soon as the player asks to connect. `OnInit` restores them:
--- going back to the title reloads the welcome scene, which runs this script
--- again.
---@param enable boolean
function GlobalNetTest:setConnectUIEnableDraw(enable)
    local ui_manager = TL_Client.GetContext():GetUIManager()
    for _, entity in ipairs({ self._connect_button, self._ip_input }) do
        if entity ~= TL_Common.null_entity then
            local widget = ui_manager:Get(entity)
            if widget then
                widget.m_enable_draw = enable
            end
        end
    end
end

function GlobalNetTest:removeListeners()
    local event_system = TL_Client.GetContext():GetEventSystem()
    for _, id in ipairs(self._listeners) do
        event_system:Remove(id)
    end
    self._listeners = {}
end

function GlobalNetTest:OnInit()
    ensureClientWorld()

    local ctx = TL_Client.GetContext()
    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        ctx:Log("global net test: no current scene")
        return
    end

    local name_manager = ctx:GetEntityNameManager()
    local ui_root = scene:GetUIRootEntity()
    self._connect_button = name_manager:FindChildByName(ui_root, "ConnectButton")
    self._ip_input = name_manager:FindChildByName(ui_root, "IPTextInput")
    -- The welcome scene is reloaded when the round ends: show the widgets
    -- hidden by the previous connection again.
    self:setConnectUIEnableDraw(true)

    local event_system = ctx:GetEventSystem()
    self:trackListener(event_system:AddUIMouseClickedEvent(function(id, event)
        if event.m_entity == self._connect_button then
            self:onConnectClicked()
        end
    end))
    self:trackListener(event_system:AddNetMsg_ConnectEvent(function(id, peer, payload)
        self:onConnected()
    end))
    -- Keep these while the client waits in the lobby: the server can reject
    -- this client (full) or drop the connection, and the player must be able to
    -- try again right away. They are dropped by `OnQuit` when the game scene is
    -- entered (or when the round sends the player back to the title).
    self:trackListener(event_system:AddNetMsg_ServerFullEvent(function(id, peer, payload)
        self:onConnectFailed("server full")
    end))
    self:trackListener(event_system:AddNetMsg_DisconnectEvent(function(id, peer, payload)
        self:onConnectFailed("disconnected")
    end))
    self:trackListener(event_system:AddUIMouseDownEvent(function(id, event)
        if event.m_entity ~= self._ip_input then
            return
        end
        local widget = ctx:GetUIManager():Get(self._ip_input)
        local text_input = widget and widget:GetTextInput()
        if text_input and text_input:GetText() == k_ip_placeholder then
            text_input:SetText("")
        end
    end))
end

---@param elapse_time TimeType
function GlobalNetTest:OnUpdate(elapse_time)
    if not self._pending_connect then
        return
    end
    self._pending_connect = false

    local ctx = TL_Client.GetContext()
    local net_config = require("common.net")
    local ip = net_config.ip
    local widget = ctx:GetUIManager():Get(self._ip_input)
    local text_input = widget and widget:GetTextInput()
    if text_input then
        local text = text_input:GetText():match("^%s*(.-)%s*$")
        if text ~= "" and text ~= k_ip_placeholder then
            ip = text
        end
    end

    -- Connecting blocks until ENet reports a result, so it is deferred to the
    -- update loop instead of running inside the UI event dispatch.
    ctx:ConnectToServer(TL_Common.NetAddress(ip, net_config.port))
    if not ctx:GetNetPeer():IsValid() then
        self:onConnectFailed(tostring(ip) .. ":" .. tostring(net_config.port))
    end
end

function GlobalNetTest:onConnected()
    -- The connection is up but stays on this scene until the lobby is full
    -- (ClientWorld switches on PlayerJoined). The listeners are intentionally
    -- kept: if the server drops this client before the match starts, the
    -- connection widgets have to come back so the player can retry.
    TL_Client.GetContext():Log("global net test: connected, waiting for the lobby")
end

function GlobalNetTest:OnQuit()
    self:removeListeners()
end

function GlobalNetTest:OnRender()
end

return GlobalNetTest
