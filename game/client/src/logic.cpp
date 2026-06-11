#include "client/logic.hpp"

#include "client/context.hpp"
#include "client/input/input.hpp"
#include "client/input/keyboard.hpp"
#include "common/asset_manager.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/scene.hpp"
#include "enet/enet.h"
#include "imgui.h"
#include "proto/all_proto.pb.h"

void TalkBox::AddMsg(const std::string& msg) {
    m_msgs.push_back(msg);
    m_scroll_to_bottom = true;
}

void TalkBox::Render() {
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar;
    ImVec2 size{400, 300};
    if (ImGui::Begin("TalkBox", nullptr, flags)) {
        ImGui::SetWindowSize(size);
        auto window_size = CLIENT_CONTEXT.m_window->GetWindowSize();
        ImGui::SetWindowPos(ImVec2{50, window_size.h - size.y - 50});

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{6, 6});

        ImGui::PushTextWrapPos();
        float input_area_height = ImGui::GetFrameHeightWithSpacing() + 8.0f;
        if (ImGui::BeginChild("text_panel",
                              ImVec2{0, size.y - input_area_height - 40.0f},
                              ImGuiChildFlags_Border)) {
            for (auto& msg : m_msgs) {
                ImGui::TextWrapped("%s", msg.c_str());
            }
            if (m_scroll_to_bottom) {
                ImGui::SetScrollHereY(1.0f);
                m_scroll_to_bottom = false;
            }
            ImGui::EndChild();
        }
        ImGui::PopTextWrapPos();

        float button_width = ImGui::CalcTextSize("Send").x +
                             ImGui::GetStyle().FramePadding.x * 2.0f +
                             ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x -
                                button_width);
        if (ImGui::InputText("##input", m_input_text, sizeof(m_input_text),
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (std::strlen(m_input_text) > 0) {
                m_is_require_send = true;
            }
            ImGui::SetKeyboardFocusHere(-1);
        }
        ImGui::SameLine();
        if (ImGui::Button("Send") ||
            CLIENT_CONTEXT.m_keyboard->Get(Key::Return).IsPressed()) {
            if (std::strlen(m_input_text) > 0) {
                m_is_require_send = true;
            }
        }

        ImGui::PopStyleVar();
        ImGui::End();
    }
}

const char* TalkBox::GetInputText() const {
    return m_input_text;
}

bool TalkBox::NeedSend() const {
    return m_is_require_send;
}

void TalkBox::MarkSent() {
    m_is_require_send = false;
    std::fill(std::begin(m_input_text), std::end(m_input_text), 0);
}

Vec2 IPlank::GetPosition() const {
    return m_position;
}

void IPlank::Move(const Vec2& offset) {
    m_position += offset;
}

void IPlank::MoveTo(const Vec2& dst) {
    m_position = dst;
}

float IPlank::GetSpeed() const {
    return m_speed;
}

CtrlPlank::CtrlPlank(const Vec2& half_rect_size, const Color& color,
                     uint32_t id)
    : m_half_rect_size{half_rect_size}, m_color{color}, m_id{id} {}

void CtrlPlank::Render() {
    CLIENT_CONTEXT.m_renderer->FillRect(Rect{GetPosition(), m_half_rect_size},
                                        m_color);
}

const Vec2& CtrlPlank::GetHalfRectSize() const {
    return m_half_rect_size;
}

uint32_t CtrlPlank::GetID() const {
    return m_id;
}

SyncPlank::SyncPlank(const Vec2& half_rect_size, const Color& color,
                     uint32_t id)
    : m_half_rect_size{half_rect_size}, m_color{color}, m_id{id} {}

void SyncPlank::Render() {
    CLIENT_CONTEXT.m_renderer->FillRect(Rect{GetPosition(), m_half_rect_size},
                                        m_color);
}

uint32_t SyncPlank::GetID() const {
    return m_id;
}

void ClientLogic::OnInit() {
    CLIENT_CONTEXT.ConnectToServer({IP.data(), PORT});

    if (!CLIENT_CONTEXT.m_net_peer.IsValid()) {
        m_talk_box.AddMsg("connect to server failed");
        LOGE("connect to server {}:{} failed", IP, PORT);
        return;
    }

    // register net msg handler
    registerNetMsgDelegate(&ClientLogic::onDisconnectMsgReceive);
    registerNetMsgDelegate(&ClientLogic::onTalkMsgReceive);
    registerNetMsgDelegate(&ClientLogic::onEnterMsgReceive);
    registerNetMsgDelegate(&ClientLogic::onPeerDisconnectReceive);
    registerNetMsgDelegate(&ClientLogic::onMoveReceive);
}

void ClientLogic::OnUpdate(TimeType elapse) {
    talkBoxUpdate();

    if (m_ctrl_plank) {
        auto& move_y_axise = CLIENT_CONTEXT.m_input_manager->GetAxis("MoveY");

        if (move_y_axise.Value() != 0) {
            m_ctrl_plank->Move(
                {0, move_y_axise.Value() * static_cast<float>(elapse) *
                        m_ctrl_plank->GetSpeed()});

            proto::NetMsg net_msg;
            auto* move = net_msg.mutable_m_move();
            move->set_m_id(m_ctrl_plank->GetID());
            auto* pos = move->mutable_m_position();
            pos->set_m_x(m_ctrl_plank->GetPosition().x);
            pos->set_m_y(m_ctrl_plank->GetPosition().y);

            CLIENT_CONTEXT.m_net_host->Send(&CLIENT_CONTEXT.m_net_peer, net_msg, 0);
        }
    }
}

void ClientLogic::OnRender() {
    m_talk_box.Render();

    if (m_ctrl_plank) {
        m_ctrl_plank->Render();
    }
    if (m_sync_plank) {
        m_sync_plank->Render();
    }
}

void ClientLogic::OnQuit() {
    m_ctrl_plank.reset();
    m_sync_plank.reset();
}

void ClientLogic::onDisconnectMsgReceive(const NetMsg<proto::Disconnect>&) {
    m_talk_box.AddMsg("server disconnected");

    CLIENT_CONTEXT.m_net_peer.Reset();
}

void ClientLogic::onTalkMsgReceive(const NetMsg<proto::TalkMsg>& msg) {
    m_talk_box.AddMsg(msg->m_msg());
}

void ClientLogic::onEnterMsgReceive(const NetMsg<proto::Enter>& msg) {
    Vec2 position{msg->m_position().m_x(), msg->m_position().m_y()};

    if (!m_ctrl_plank) {
        m_ctrl_plank =
            std::make_unique<CtrlPlank>(RectHalfSize, Color::Blue, msg->m_id());

        m_ctrl_plank->MoveTo(position);
    } else if (msg->m_id() != m_ctrl_plank->GetID() && !m_sync_plank) {
        m_sync_plank = std::make_unique<SyncPlank>(RectHalfSize, Color::Green,
                                                   msg->m_id());

        m_sync_plank->MoveTo(position);
    }
}

void ClientLogic::onPeerDisconnectReceive(const NetMsg<proto::Leave>& msg) {
    TL_RETURN_IF_NULL(m_sync_plank);

    LOGI("onPeerDisconnectReceive: leave id={}, sync id={}", msg->m_id(),
         m_sync_plank->GetID());

    if (msg->m_id() == m_sync_plank->GetID()) {
        m_sync_plank.reset();
    }
}

void ClientLogic::onMoveReceive(const NetMsg<proto::Move>& msg) {
    TL_RETURN_IF_FALSE(m_sync_plank && msg->m_id() == m_sync_plank->GetID());

    Vec2 position{msg->m_position().m_x(), msg->m_position().m_y()};
    m_sync_plank->MoveTo(position);
}

void ClientLogic::talkBoxUpdate() {
    TL_RETURN_IF_FALSE(m_talk_box.NeedSend());

    if (!CLIENT_CONTEXT.m_net_host || !CLIENT_CONTEXT.m_net_host->IsValid()) {
        m_talk_box.AddMsg("disconnect");
        return;
    }

    std::string input_text = m_talk_box.GetInputText();
    bool success = true;

    proto::NetMsg net_msg;
    net_msg.mutable_m_talk_msg()->set_m_msg(input_text);

    std::vector<std::byte> buf(net_msg.ByteSizeLong());
    if (!net_msg.SerializeToArray(buf.data(), buf.size())) {
        LOGE("Serialize NetMsg to buffer failed");
        return;
    }

    CLIENT_CONTEXT.m_net_host->Send(&CLIENT_CONTEXT.m_net_peer, buf.data(),
                                    buf.size(), 0);

    if (!success) {
        m_talk_box.AddMsg("send msg failed");
    }
    m_talk_box.MarkSent();
}