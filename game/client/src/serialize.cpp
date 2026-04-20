#include "client/serialize.hpp"
#include "schema/serialize/anim_player.hpp"

rapidxml::xml_node<>* Serialize(ClientContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const AnimationPlayer& payload,
                                const std::string& name) {
    AnimationPlayerDefinition create_info;
    create_info.m_auto_play = payload.IsAutoPlayEnabled();
    create_info.m_animation = payload.GetAnimation();
    create_info.m_loop = payload.GetLoopCount();
    create_info.m_rate = payload.GetRate();

    return Serialize(ctx, doc, create_info, name);
}

void Deserialize(ClientContext& ctx, const rapidxml::xml_node<>& node,
                 AnimationPlayer& payload) {
    AnimationPlayerDefinition create_info;
    Deserialize(ctx, node, create_info);

    payload.SetLoop(create_info.m_loop);
    payload.SetRate(create_info.m_rate);
    payload.EnableAutoPlay(create_info.m_auto_play);
    payload.ChangeAnimation(create_info.m_animation);
}
