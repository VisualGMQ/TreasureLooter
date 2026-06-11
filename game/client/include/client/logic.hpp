#pragma once
#include "animation_player.hpp"
#include "client/sprite.hpp"
#include "common/cct.hpp"
#include "common/logic.hpp"
#include "common/net/udp.hpp"
#include "common/timer.hpp"
#include "proto/proto.pb.h"
#include "schema/gameplay_config.hpp"
/* NOTE: don't include enet.h in header file!!! It includes <winsock.h> and
 * pollute your project!!!
 * #include "enet/enet.h"
 */

typedef struct _ENetHost ENetHost;
typedef struct _ENetPeer ENetPeer;

class TalkBox {
public:
    void AddMsg(const std::string& msg);
    void Render();
    const char* GetInputText() const;

    bool NeedSend() const;
    void MarkSent();

private:
    std::vector<std::string> m_msgs;
    char m_input_text[2048] = {0};
    bool m_is_require_send = false;
    bool m_scroll_to_bottom = false;
};

class IPlank {
public:
    virtual ~IPlank() = default;

    Vec2 GetPosition() const;
    void Move(const Vec2& offset);
    void MoveTo(const Vec2& dst);
    float GetSpeed() const;

private:
    Vec2 m_position;
    float m_speed = 300.0;
};

class CtrlPlank : public IPlank {
public:
    CtrlPlank(const Vec2& half_rect_size, const Color& color, uint32_t id);

    void Render();
    const Vec2& GetHalfRectSize() const;
    uint32_t GetID() const;

private:
    uint32_t m_id;
    Vec2 m_half_rect_size{50, 150};
    Color m_color{Color::Green};
};

class SyncPlank: public IPlank {
public:
    SyncPlank(const Vec2& half_rect_size, const Color& color, uint32_t id);

    void Render();
    uint32_t GetID() const;

private:
    uint32_t m_id;
    Vec2 m_half_rect_size{50, 150};
    Color m_color{Color::Green};
};

// NOTE: temporary class for learning networking, we write network code here
// rather than put into luau script
class ClientLogic : public ILogic {
public:
    void OnInit() override;
    void OnUpdate(TimeType) override;
    void OnRender();
    void OnQuit() override;

private:
    TalkBox m_talk_box;
    std::unique_ptr<CtrlPlank> m_ctrl_plank;
    std::unique_ptr<SyncPlank> m_sync_plank;

    void onDisconnectMsgReceive(const NetMsg<proto::Disconnect>&);
    void onTalkMsgReceive(const NetMsg<proto::TalkMsg>&);
    void onEnterMsgReceive(const NetMsg<proto::Enter>&);
    void onPeerDisconnectReceive(const NetMsg<proto::Leave>&);
    void onMoveReceive(const NetMsg<proto::Move>&);

    template <typename T, typename Class>
    void registerNetMsgDelegate(void (Class::*f)(const T&)) {
        ILogic::registerNetMsgDelegate<typename T::message_type>(
            [this, f](EventListenerID, const T& msg) {
                std::invoke(f, this, msg);
            });
    }

    void talkBoxUpdate();
};